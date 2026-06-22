#ifndef CLASS_DEFINITIONS_CPP
#define CLASS_DEFINITIONS_CPP

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <format>
#include <cstdlib>
#include <map>

class AssemblyCode {
    private:
        std::ofstream _assemblyCodeFile;
        struct AssemblyTextSection {
                std::string _assemblyTextSection;
                AssemblyTextSection(const std::string& beginningOfSection) { _assemblyTextSection = beginningOfSection; }
                void addInstruction(const std::string& instruction) { _assemblyTextSection += instruction; }
                std::string getAssemblyTextSection(void) { return _assemblyTextSection; }
        };
        struct AssemblyDataSection {
            std::string _assemblyDataSection;
            AssemblyDataSection(const std::string& beginningOfSection) { _assemblyDataSection = beginningOfSection; }
            void addInstruction(const std::string& instruction) { _assemblyDataSection += instruction; }
            std::string getAssemblyDataSection(void) { return _assemblyDataSection; }
        };
        struct AssemblyBssSection {
            std::string _assemblyBssSection;
            AssemblyBssSection(const std::string& beginningOfSection) { _assemblyBssSection = beginningOfSection; }
            void addInstruction(const std::string& instruction) { _assemblyBssSection += instruction; }
            std::string getAssemblyBssSection(void) { return _assemblyBssSection; }
        };
        struct AssemblyFunctions {
            std::string _assemblyFunctions;
            AssemblyFunctions(const std::string& beginningOfSection) { _assemblyFunctions = beginningOfSection; }
            void addInstruction(const std::string& instruction) { _assemblyFunctions += instruction; }
            std::string getAssemblyFunctions(void) { return _assemblyFunctions; }
        };
        AssemblyTextSection _assemblyTextSection{".section .text\n.global _start\n\n_start:"};
        AssemblyDataSection _assemblyDataSection{".section .data\n"};
        AssemblyBssSection _assemblyBssSection{"\n.section .bss\n\n"};
        AssemblyFunctions _assemblyFunctions{"\n.section .text\n"};
    public:
        AssemblyCode(const std::string& filename) {
            _assemblyCodeFile.open(filename.substr(0, filename.find_last_of('.')) + ".S");
        }
        ~AssemblyCode(void) {
            _assemblyCodeFile.close();
        }
        void addInstructionToText(const std::string& instruction) { _assemblyTextSection.addInstruction(instruction); }
        void addInstructionToData(const std::string& instruction) { _assemblyDataSection.addInstruction(instruction); }
        void addInstructionToBss(const std::string& instruction) { _assemblyBssSection.addInstruction(instruction); }
        void addInstructionToFunctions(const std::string& instruction) { _assemblyFunctions.addInstruction(instruction); }
        std::string getAssemblyTextSection(void) { return _assemblyTextSection.getAssemblyTextSection(); }
        std::string getAssemblyDataSection(void) { return _assemblyDataSection.getAssemblyDataSection(); }
        std::string getAssemblyBssSection(void) { return _assemblyBssSection.getAssemblyBssSection(); }
        std::string getAssemblyFunctions(void) { return _assemblyFunctions.getAssemblyFunctions(); }
        void writeToFile(void) {
            _assemblyCodeFile
            << _assemblyDataSection.getAssemblyDataSection()
            << _assemblyBssSection.getAssemblyBssSection()
            << _assemblyFunctions.getAssemblyFunctions()
            << _assemblyTextSection.getAssemblyTextSection()
            << std::endl; 
        }
};

class Compiler {
    private:
        enum InstructionType {
            PRINT,
            NEW,
            SET,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            NEWLINE,
            EXIT,
            DONE,
            IF,
            READ,
            INVALID
        };
        std::ifstream _sourceCodeFile;
        AssemblyCode _assemblyCode;
        std::string _filename;
        bool _is64Bits;
        uint32_t _labelCounter;
        std::vector<uint32_t> _ifLabelStack;
        InstructionType getInstructionType(const std::string& instruction) {
            if (instruction.find("print") != std::string::npos) return InstructionType::PRINT;
            if (instruction.find("newline") != std::string::npos) return InstructionType::NEWLINE;
            if (instruction.find("new") != std::string::npos) return InstructionType::NEW;
            if (instruction.find("set") != std::string::npos) return InstructionType::SET;
            if (instruction.find("add") != std::string::npos) return InstructionType::ADD;
            if (instruction.find("subtract") != std::string::npos) return InstructionType::SUBTRACT;
            if (instruction.find("multiply") != std::string::npos) return InstructionType::MULTIPLY;
            if (instruction.find("divide") != std::string::npos) return InstructionType::DIVIDE;
            if (instruction.find("exit") != std::string::npos) return InstructionType::EXIT;
            if (instruction.find("done") != std::string::npos) return InstructionType::DONE;
            if (instruction.find("if") != std::string::npos) return InstructionType::IF;
            if (instruction.find("read") != std::string::npos) return InstructionType::READ;
            return InstructionType::INVALID;
        }
        bool doesTheVariableExist(const std::string& variableName) {
            return _assemblyCode.getAssemblyDataSection().find(variableName) != std::string::npos;
        }
    public:
        bool _errorFlag;
        static void reportError(const std::string& message) { std::cerr << "Error: " << message << std::endl; }
        static void reportWarning(const std::string& message) { std::cerr << "Warning: " << message << std::endl; }
        static void reportInfo(const std::string& message) { std::cout << "Info: " << message << std::endl; }
        Compiler(const std::string& filename, bool is64Bits) : _assemblyCode(filename) {
            _filename = filename;
            _sourceCodeFile.open(filename);
            _errorFlag = false;
            _is64Bits = is64Bits;
            _labelCounter = 0;
            if (!_sourceCodeFile.is_open()) {
                reportError("Failed to open source code file: " + filename);
                return;
            }
            reportInfo("Successfully opened source code file: " + filename);
        }
        ~Compiler(void) {
            _sourceCodeFile.close();
        }
        void compile(void) {
            std::ostringstream sourceBuffer;
            sourceBuffer << _sourceCodeFile.rdbuf();
            std::string source = sourceBuffer.str();
            _sourceCodeFile.clear();
            _sourceCodeFile.seekg(0, std::ios::beg);
            uint32_t lineNumber = 0;
            uint16_t conditionalCounter = 0;
            std::string line;
            if (source.find("read") != std::string::npos || source.find("print") != std::string::npos) {
                const std::string readPrintAssemblyCode = "buffer: .zero 33\n";
                _assemblyCode.addInstructionToBss(readPrintAssemblyCode);
            }
            if (source.find("print") != std::string::npos) {
                const std::string itoaAssemblyCode = R"(
itoa:
    testq %rax, %rax
    jne itoa_not_zero
    movb $'0', buffer(%rip)
    movb $0, buffer+1(%rip)
    ret
itoa_not_zero:
    movq %rax, %r8
    xorq %r10, %r10
    testq %r8, %r8
    jns itoa_count
    negq %r8
    movq $1, %r10
itoa_count:
    movq %r8, %rax
    xorq %r9, %r9
itoa_count_loop:
    incq %r9
    xorq %rdx, %rdx
    movq $10, %rdi
    divq %rdi
    testq %rax, %rax
    jnz itoa_count_loop
    leaq buffer(%rip), %rcx
    cmpq $0, %r10
    je itoa_sign_done
    movb $'-', (%rcx)
    incq %rcx
itoa_sign_done:
    addq %r9, %rcx
    movb $0, (%rcx)
    movq %r8, %rax
itoa_emit_loop:
    decq %rcx
    xorq %rdx, %rdx
    movq $10, %rdi
    divq %rdi
    addb $'0', %dl
    movb %dl, (%rcx)
    testq %rax, %rax
    jnz itoa_emit_loop
    ret

get_string_length:
    xorq %rcx, %rcx
get_string_length_loop:
    cmpb $0, (%rax)
    je get_string_length_done
    incq %rcx
    incq %rax
    jmp get_string_length_loop
get_string_length_done:
    ret

)";
                _assemblyCode.addInstructionToFunctions(itoaAssemblyCode);
            }
            if (source.find("read") != std::string::npos) {
                const std::string atoiAssemblyCode = R"(
atoi:
    leaq buffer(%rip), %rsi
    xorq %rax, %rax
    xorq %r10, %r10
    movb (%rsi), %cl
    cmpb $'-', %cl
    jne atoi_loop
    movq $1, %r10
    incq %rsi
atoi_loop:
    movzbq (%rsi), %rcx
    cmpb $'0', %cl
    jb atoi_done
    cmpb $'9', %cl
    ja atoi_done
    subb $'0', %cl
    imulq $10, %rax
    addq %rcx, %rax
    incq %rsi
    jmp atoi_loop
atoi_done:
    cmpq $0, %r10
    je atoi_positive
    negq %rax
atoi_positive:
    ret

)";
                _assemblyCode.addInstructionToFunctions(atoiAssemblyCode);
            }
            if (source.find("newline") != std::string::npos) {
                const std::string newlineAssemblyCode = R"(
    newline: .ascii "\n"
)";
                _assemblyCode.addInstructionToData(newlineAssemblyCode);
            }
            while (std::getline(_sourceCodeFile, line)) {
                lineNumber++;
                if (_errorFlag) { return; }
                if (line.empty()) { continue; }
                InstructionType instructionType = getInstructionType(line);
                std::vector<std::string> tokens;
                std::stringstream ss(line);
                std::string token;
                std::string variableName;
                while (ss >> token) { tokens.push_back(token); }
                try {
                    std::string assemblyInstruction;
                    switch (instructionType) {
                        case InstructionType::PRINT:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    call itoa
    leaq buffer(%rip), %rax
    call get_string_length
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    syscall
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rax
    call itoa
    leaq buffer(%rip), %rax
    call get_string_length
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    syscall
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::NEW:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " already exists");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    {}: .quad 0
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    {}: .long 0
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            break;
                        case InstructionType::SET:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq ${}, {}(%rip)
)", tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl ${}, {}(%rip)
)", tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::ADD:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    addq %rax, {}(%rip)
)", tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    addl %eax, {}(%rip)
)", tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::SUBTRACT:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    subq %rax, {}(%rip)
)", tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    subl %eax, {}(%rip)
)", tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::MULTIPLY:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    movq {}(%rip), %rbx
    imulq %rax, %rbx
    movq %rbx, {}(%rip)
)", tokens[2], tokens[1], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    movl {}(%rip), %ebx
    imull %eax, %ebx
    movl %ebx, {}(%rip)
)", tokens[2], tokens[1], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::DIVIDE:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cqto
    movq {}(%rip), %rbx
    idivq %rbx
    movq %rax, {}(%rip)
)", tokens[1], tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cltd
    movl {}(%rip), %ebx
    idivl %ebx
    movl %eax, {}(%rip)
)", tokens[1], tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::NEWLINE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = R"(
    movq $1, %rax
    movq $1, %rdi
    leaq newline(%rip), %rsi
    movq $1, %rdx
    syscall
)";
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::EXIT:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq $60, %rax
    movq {}(%rip), %rdi
    syscall
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movq $60, %rax
    movslq {}(%rip), %rdi
    syscall
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::IF:
                            if (tokens.size() != 4 || tokens[2] != "equals") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[3])) {
                                reportError("Variable " + tokens[3] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            _ifLabelStack.push_back(_labelCounter);
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cmpq {}(%rip), %rax
    jne .Lendif_{}
)", tokens[1], tokens[3], _labelCounter);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cmpl {}(%rip), %eax
    jne .Lendif_{}
)", tokens[1], tokens[3], _labelCounter);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            _labelCounter++;
                            break;
                        case InstructionType::DONE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_ifLabelStack.empty()) {
                                reportError("'done' without a matching 'if' at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
.Lendif_{}:
)", _ifLabelStack.back());
                            _ifLabelStack.pop_back();
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::READ:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq $0, %rax
    movq $0, %rdi
    leaq buffer(%rip), %rsi
    movq $33, %rdx
    syscall
    call atoi
    movq %rax, {}(%rip)
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movq $0, %rax
    movq $0, %rdi
    leaq buffer(%rip), %rsi
    movq $33, %rdx
    syscall
    call atoi
    movl %eax, {}(%rip)
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::INVALID:
                            reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                            _errorFlag = true;
                            break;
                    }
                } catch (...) {
                    reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                    _errorFlag = true;
                }
            }
            if (!_ifLabelStack.empty()) {
                reportError("Unterminated 'if' block (missing 'done')");
                _errorFlag = true;
                return;
            }
            if (_errorFlag) { return; }
            std::string finalInstruction = R"(
    movq $60, %rax
    xorq %rdi, %rdi
    syscall
)";
            _assemblyCode.addInstructionToText(finalInstruction);
            _assemblyCode.writeToFile();
            std::string filenameWithoutExtension = _filename.substr(0, _filename.find_last_of('.'));
            if (system(std::format("as {}.S -o {}.o", filenameWithoutExtension, filenameWithoutExtension).c_str()) != 0) {
                reportError("Assembler failed");
                _errorFlag = true;
                return;
            }
            if (system(std::format("ld {}.o -o {}", filenameWithoutExtension, filenameWithoutExtension).c_str()) != 0) {
                reportError("Linker failed");
                _errorFlag = true;
            }
        }
};

#endif
