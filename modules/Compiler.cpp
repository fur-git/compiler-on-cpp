#ifndef COMPILER_CPP
#define COMPILER_CPP

#include "AssemblyCode.cpp"

#include <cstdint>
#include <vector>
#include <sstream>
#include <format>
#include <cstdlib>
#include <map>
#include <set>

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
            ELSE,
            READ,
            INVALID,
            INFO,
            WARNING,
            ERROR,
            DEBUG,
            PRINTSTRING,
            NOTHING,
            FDONE,
            FUNCTION,
            EXECUTE,
            COMPILETIMEINFO,
            COMPILETIMEWARNING,
            COMPILETIMEERROR,
            COMPILETIMEDEBUG,
            COMPILETIMEIF,
            COMPILETIMEDONE,
            MACRO,
            MARK,
            GOTO,
        };
        struct ConditionalMetadata {
            uint16_t labelId;
            bool hasElse;
        };
        std::ifstream _sourceCodeFile;
        AssemblyCode _assemblyCode;
        std::string _filename;
        bool _is64Bits;
        uint16_t _labelCounter;
        uint16_t _variableCounter;
        uint16_t _functionCounter;
        uint16_t _conditionalCounter;
        bool _isInAFunction;
        bool _isANumber;
        bool _isSkippingCode;
        bool _needsBuffer;
        bool _needsGetStringLength;
        bool _needsItoa;
        bool _needsAtoi;
        bool _needsNewline;
        std::vector<ConditionalMetadata> _conditionalMetadataStack;
        std::vector<uint16_t> _functionStack;
        std::vector<uint16_t> _compileTimeIfStack;
        std::map<std::string, std::string> _definedMacroVariables;
        std::set<std::string> _definedFunctions;
        std::set<std::string> _definedVariables;
        std::set<std::string> _definedMarks;
        InstructionType getInstructionType(const std::string& instruction) {
            if (instruction.find("#macro") != std::string::npos) return InstructionType::MACRO;
            if (instruction.find("#compileTimeInfo") != std::string::npos) return InstructionType::COMPILETIMEINFO;
            if (instruction.find("#compileTimeWarning") != std::string::npos) return InstructionType::COMPILETIMEWARNING;
            if (instruction.find("#compileTimeError") != std::string::npos) return InstructionType::COMPILETIMEERROR;
            if (instruction.find("#compileTimeDebug") != std::string::npos) return InstructionType::COMPILETIMEDEBUG;
            if (instruction.find("#if") != std::string::npos) return InstructionType::COMPILETIMEIF;
            if (instruction.find("#done") != std::string::npos) return InstructionType::COMPILETIMEDONE;
            if (instruction.find("function") != std::string::npos) return InstructionType::FUNCTION;
            if (instruction.find("fdone") != std::string::npos) return InstructionType::FDONE;
            if (instruction.find("execute") != std::string::npos) return InstructionType::EXECUTE;
            if (instruction.find("printString") != std::string::npos) return InstructionType::PRINTSTRING;
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
            if (instruction.find("else") != std::string::npos) return InstructionType::ELSE;
            if (instruction.find("if") != std::string::npos) return InstructionType::IF;
            if (instruction.find("read") != std::string::npos) return InstructionType::READ;
            if (instruction.find("info") != std::string::npos) return InstructionType::INFO;
            if (instruction.find("warning") != std::string::npos) return InstructionType::WARNING;
            if (instruction.find("error") != std::string::npos) return InstructionType::ERROR;
            if (instruction.find("debug") != std::string::npos) return InstructionType::DEBUG;
            if (instruction.find("nothing") != std::string::npos) return InstructionType::NOTHING;
            if (instruction.find("mark") != std::string::npos) return InstructionType::MARK;
            if (instruction.find("go to") != std::string::npos) return InstructionType::GOTO;
            return InstructionType::INVALID;
        }
        bool doesTheVariableExist(const std::string& variableName) {
            return _definedVariables.contains(variableName);
        }
        bool doesTheFunctionExist(const std::string& functionName) {
            return _definedFunctions.contains(functionName);
        }
        bool doesTheMacroExist(const std::string& macroName) {
            return _definedMacroVariables.contains(macroName);
        }
        bool isANumber(const std::string& string) {
            for (const char& c : string) {
                if (c < '0' || c > '9') {
                    return false;
                }
            }
            return true;
        }
    public:
        bool _errorFlag;
        static void reportError(const std::string& message) { std::cerr << "ERROR: " << message << std::endl; }
        static void reportWarning(const std::string& message) { std::cerr << "WARNING: " << message << std::endl; }
        static void reportInfo(const std::string& message) { std::cout << "INFO: " << message << std::endl; }
        static void reportDebug(const std::string& message) { std::cout << "DEBUG: " << message << std::endl; }
        Compiler(const std::string& filename, bool is64Bits) : _assemblyCode(filename) {
            _filename = filename;
            _sourceCodeFile.open(filename);
            _errorFlag = false;
            _is64Bits = is64Bits;
            _labelCounter = 0;
            _variableCounter = 0;
            _functionCounter = 0;
            _conditionalCounter = 0;
            _isInAFunction = false;
            _isSkippingCode = false;
            _needsBuffer = false;
            _needsGetStringLength = false;
            _needsItoa = false;
            _needsAtoi = false;
            _needsNewline = false;
            _definedFunctions.clear();
            _definedVariables.clear();
            _definedMacroVariables.clear();
            _compileTimeIfStack.clear();
            _definedMarks.clear();
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
            reportInfo("Compiling now...");
            uint32_t lineNumber = 0;
            std::string line;
            while (std::getline(_sourceCodeFile, line)) {
                lineNumber++;
                if (_errorFlag) { return; }
                if (line.empty()) { continue; }
                InstructionType instructionType = getInstructionType(line);
                if (_isSkippingCode && instructionType != InstructionType::COMPILETIMEDONE) { continue; }
                std::vector<std::string> tokens;
                std::stringstream ss(line);
                std::string token;
                std::string variableName;
                while (ss >> token) { tokens.push_back(token); }
                try {
                    std::string assemblyInstruction;
                    std::string endMessage;
                    switch (instructionType) {
                        case InstructionType::PRINT:
                            _needsBuffer = true;
                            _needsItoa = true;
                            _needsGetStringLength = true;
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
    call RESERVED_itoa_BY_LANGUAGE
    leaq buffer(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    syscall
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rax
    call RESERVED_itoa_BY_LANGUAGE
    leaq buffer(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    syscall
)", tokens[1]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
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
                            if (doesTheFunctionExist(tokens[1])) {
                                reportError("Function with the same name as the variable already exists");
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
                            _definedVariables.insert(tokens[1]);
                            break;
                        case InstructionType::SET:
                            if (tokens.size() != 5 || tokens[2] != "to" || tokens[3] != "be") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheMacroExist(tokens[4])) {
                                tokens[4] = _definedMacroVariables[tokens[4]];
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq ${}, {}(%rip)
)", tokens[4], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl ${}, {}(%rip)
)", tokens[4], tokens[1]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::ADD:
                            if (tokens.size() != 4 || tokens[2] != "into") {
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
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    addq %rax, {}(%rip)
)", tokens[1], tokens[3]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    addl %eax, {}(%rip)
)", tokens[1], tokens[3]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::SUBTRACT:
                            if (tokens.size() != 4 || tokens[2] != "from") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[3])) {
                                reportError("Variable " + tokens[3] + " does not exist");
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
)", tokens[1], tokens[3]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    subl %eax, {}(%rip)
)", tokens[1], tokens[3]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::MULTIPLY:
                            if (tokens.size() != 4 || tokens[2] != "by") {
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
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    movq {}(%rip), %rbx
    imulq %rax, %rbx
    movq %rbx, {}(%rip)
)", tokens[3], tokens[1], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    movl {}(%rip), %ebx
    imull %eax, %ebx
    movl %ebx, {}(%rip)
)", tokens[3], tokens[1], tokens[1]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::DIVIDE:
                            if (tokens.size() != 4 || tokens[2] != "by") {
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
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cqto
    movq {}(%rip), %rbx
    idivq %rbx
    movq %rax, {}(%rip)
)", tokens[1], tokens[3], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cltd
    movl {}(%rip), %ebx
    idivl %ebx
    movl %eax, {}(%rip)
)", tokens[1], tokens[3], tokens[1]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::NEWLINE:
                            _needsNewline = true;
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
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::EXIT:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1]) && !doesTheMacroExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rdi
    jmp RESERVED_exit_BY_LANGUAGE
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rdi
    jmp RESERVED_exit_BY_LANGUAGE
)", tokens[1]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::IF:
                            if (tokens.size() != 7 ||
                            tokens[2] != "equals" ||
                            tokens[3] != "to" ||
                            tokens[5] != "then" ||
                            tokens[6] != "do") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[4])) {
                                reportError("Variable " + tokens[3] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            _conditionalMetadataStack.push_back({_labelCounter, false});
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cmpq {}(%rip), %rax
    jne .Lelse_{}
)", tokens[1], tokens[4], _labelCounter);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cmpl {}(%rip), %eax
    jne .Lelse_{}
)", tokens[1], tokens[4], _labelCounter);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _labelCounter++;
                            break;
                        case InstructionType::ELSE:
                            if (tokens.size() != 2 || tokens[1] != "do") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_conditionalMetadataStack.empty()) {
                                reportError("'else' without a matching 'if' at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_conditionalMetadataStack.back().hasElse) {
                                reportError("'if' block already has an 'else' at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    jmp .Lendif_{}

.Lelse_{}:
)", _conditionalMetadataStack.back().labelId, _conditionalMetadataStack.back().labelId);
                            _conditionalMetadataStack.back().hasElse = true;
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::DONE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_conditionalMetadataStack.empty()) {
                                reportError("'done' without a matching 'if' at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_conditionalMetadataStack.back().hasElse) {
                                assemblyInstruction = std::format(R"(
.Lendif_{}:
)", _conditionalMetadataStack.back().labelId);
                            } else {
                                assemblyInstruction = std::format(R"(
.Lelse_{}:
)", _conditionalMetadataStack.back().labelId);
                            }
                            _conditionalMetadataStack.pop_back();
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::READ:
                            _needsBuffer = true;
                            _needsAtoi = true;
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
    call RESERVED_atoi_BY_LANGUAGE
    movq %rax, {}(%rip)
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movq $0, %rax
    movq $0, %rdi
    leaq buffer(%rip), %rsi
    movq $33, %rdx
    syscall
    call RESERVED_atoi_BY_LANGUAGE
    movl %eax, {}(%rip)
)", tokens[1]);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::INFO:
                            _needsGetStringLength = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    info_{}: .asciz "INFO: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "info" && i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else if (i == tokens.size() - 1) {
                                    assemblyInstruction = std::format("{}\\n", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                            }
                            assemblyInstruction = "\"\n";
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            assemblyInstruction = std::format(R"(
    leaq info_{}(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq info_{}(%rip), %rsi
    syscall
)", _variableCounter, _variableCounter);
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _variableCounter++;
                            break;
                        case InstructionType::WARNING:
                            _needsGetStringLength = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    warning_{}: .asciz "WARNING: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "warning" && i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else if (i == tokens.size() - 1) {
                                    assemblyInstruction = std::format("{}\\n", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                            }
                            assemblyInstruction = "\"\n";
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            assemblyInstruction = std::format(R"(
    leaq warning_{}(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $2, %rdi
    leaq warning_{}(%rip), %rsi
    syscall
)", _variableCounter, _variableCounter);
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _variableCounter++;
                            break;
                        case InstructionType::ERROR:
                            _needsGetStringLength = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    error_{}: .asciz "ERROR: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "error" && i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else if (i == tokens.size() - 1) {
                                    assemblyInstruction = std::format("{}\\n", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                            }
                            assemblyInstruction = "\"\n";
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            assemblyInstruction = std::format(R"(
    leaq error_{}(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $2, %rdi
    leaq error_{}(%rip), %rsi
    syscall
)", _variableCounter, _variableCounter);
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _variableCounter++;
                            break;
                        case InstructionType::DEBUG:
                            _needsGetStringLength = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    debug_{}: .asciz "DEBUG: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "debug" && i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else if (i == tokens.size() - 1) {
                                    assemblyInstruction = std::format("{}\\n", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                            }
                            assemblyInstruction = "\"\n";
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            assemblyInstruction = std::format(R"(
    leaq debug_{}(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq debug_{}(%rip), %rsi
    syscall
)", _variableCounter, _variableCounter);
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _variableCounter++;
                            break;
                        case InstructionType::PRINTSTRING:
                            _needsGetStringLength = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    string_{}: .asciz ")", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "printString" && i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else if (i == tokens.size() - 1) {
                                    assemblyInstruction = std::format("{}", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                            }
                            assemblyInstruction = "\"\n";
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            assemblyInstruction = std::format(R"(
    leaq string_{}(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq string_{}(%rip), %rsi
    syscall
)", _variableCounter, _variableCounter);
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _variableCounter++;
                            break;
                        case InstructionType::NOTHING:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    nop
)");
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::FUNCTION:
                            if (tokens.size() != 3 || tokens[2] != "does") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheFunctionExist(tokens[1])) {
                                reportError("Function " + tokens[1] + " already exists");
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheVariableExist(tokens[1])) {
                                reportError("Variable with the same name as the function already exists");
                                _errorFlag = true;
                                continue;
                            }
                            if (_isInAFunction) {
                                reportError("Nested functions are not allowed");
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
{}:
)", tokens[1]);
                            _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            _functionCounter++;
                            _isInAFunction = true;
                            _functionStack.push_back(_functionCounter);
                            _definedFunctions.insert(tokens[1]);
                            break;
                        case InstructionType::EXECUTE:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheFunctionExist(tokens[1])) {
                                reportError("Function " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    call {}
)", tokens[1]);
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::FDONE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!_isInAFunction) {
                                reportError("'fdone' outside of a function at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    ret
)");
                            _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            _isInAFunction = false;
                            _functionStack.pop_back();
                            break;
                        case InstructionType::COMPILETIMEINFO:
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            endMessage = "CompileTime Info: ";
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "#compileTimeInfo" && i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else if (i == tokens.size() - 1) {
                                    endMessage += std::format("{}", tokens[i]);
                                }
                            }
                            reportInfo(endMessage);
                            break;
                        case InstructionType::COMPILETIMEWARNING:
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            endMessage = "CompileTime Warning: ";
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "#compileTimeWarning" && i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else if (i == tokens.size() - 1) {
                                    endMessage += std::format("{}", tokens[i]);
                                }
                            }
                            reportWarning(endMessage);
                            break;
                        case InstructionType::COMPILETIMEERROR:
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            endMessage = "CompileTime Error: ";
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "#compileTimeError" && i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else if (i == tokens.size() - 1) {
                                    endMessage += std::format("{}", tokens[i]);
                                }
                            }
                            reportError(endMessage);
                            break;
                        case InstructionType::COMPILETIMEDEBUG:
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            endMessage = "CompileTime Debug: ";
                            for (uint16_t i = 0; i < tokens.size(); i++) {
                                if (tokens[i] != "#compileTimeDebug" && i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else if (i == tokens.size() - 1) {
                                    endMessage += std::format("{}", tokens[i]);
                                }
                            }
                            reportDebug(endMessage);
                            break;
                        case InstructionType::MACRO:
                            if (tokens.size() != 4 || tokens[2] != "is") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_definedMacroVariables.contains(tokens[1])) {
                                reportError("Macro with the same name is already defined");
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheVariableExist(tokens[3])) {
                                reportError("Variable with the same name as the macro is already defined");
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheFunctionExist(tokens[3])) {
                                reportError("Function with the same name as the macro is already defined");
                                _errorFlag = true;
                                continue;
                            }
                            if (!isANumber(tokens[3])) {
                                reportError("Macro value is not a number");
                                _errorFlag = true;
                                continue;
                            }
                            _definedMacroVariables[tokens[1]] = tokens[3];
                            break;
                        case InstructionType::COMPILETIMEIF:
                            if (tokens.size() != 7 ||
                                tokens[2] != "equals" ||
                                tokens[3] != "to" ||
                                tokens[5] != "then" ||
                                tokens[6] != "do") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheMacroExist(tokens[1])) {
                                reportError("Macro " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheMacroExist(tokens[4])) {
                                reportError("Macro " + tokens[4] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_definedMacroVariables[tokens[1]] != _definedMacroVariables[tokens[4]]) { _isSkippingCode = true; }
                            _compileTimeIfStack.push_back(_conditionalCounter);
                            _conditionalCounter++;
                            break;
                        case InstructionType::COMPILETIMEDONE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_compileTimeIfStack.empty()) {
                                reportError("'done' without a matching 'if' at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            _isSkippingCode = false;
                            _compileTimeIfStack.pop_back();
                            break;
                        case InstructionType::MARK:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_isInAFunction) {
                                reportError("'mark' is not allowed inside a function at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_definedMarks.contains(tokens[1])) {
                                reportError("Mark with the same name is already defined");
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
{}:
)", tokens[1]);
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            _definedMarks.insert(tokens[1]);
                            break;
                        case InstructionType::GOTO:
                            if (tokens.size() != 3 || tokens[0] != "go" || tokens[1] != "to") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_isInAFunction) {
                                reportError("'go to' is not allowed inside a function at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!_definedMarks.contains(tokens[2])) {
                                reportError("Mark " + tokens[2] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    jmp {}
)", tokens[2]);
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
            if (_needsBuffer) {
                std::string bufferAssemblyCode = "buffer: .zero 33\n";
                _assemblyCode.addInstructionToBss(bufferAssemblyCode);
            }
            if (_needsGetStringLength) {
                std::string getStringLengthAssemblyCode = R"(
RESERVED_get_string_BY_length_LANGUAGE:
    xorq %rcx, %rcx
RESERVED_get_string_BY_length_LANGUAGE_loop:
    cmpb $0, (%rax)
    je RESERVED_get_string_BY_length_LANGUAGE_done
    incq %rcx
    incq %rax
    jmp RESERVED_get_string_BY_length_LANGUAGE_loop
RESERVED_get_string_BY_length_LANGUAGE_done:
    ret

)";
                _assemblyCode.addInstructionToFunctions(getStringLengthAssemblyCode);
            }
            if (_needsItoa) {
                std::string itoaAssemblyCode = R"(
RESERVED_itoa_BY_LANGUAGE:
    testq %rax, %rax
    jne RESERVED_itoa_BY_LANGUAGE_not_zero
    movb $'0', buffer(%rip)
    movb $0, buffer+1(%rip)
    ret
RESERVED_itoa_BY_LANGUAGE_not_zero:
    movq %rax, %r8
    xorq %r10, %r10
    testq %r8, %r8
    jns RESERVED_itoa_BY_LANGUAGE_count
    negq %r8
    movq $1, %r10
RESERVED_itoa_BY_LANGUAGE_count:
    movq %r8, %rax
    xorq %r9, %r9
RESERVED_itoa_BY_LANGUAGE_count_loop:
    incq %r9
    xorq %rdx, %rdx
    movq $10, %rdi
    divq %rdi
    testq %rax, %rax
    jnz RESERVED_itoa_BY_LANGUAGE_count_loop
    leaq buffer(%rip), %rcx
    cmpq $0, %r10
    je RESERVED_itoa_BY_LANGUAGE_sign_done
    movb $'-', (%rcx)
    incq %rcx
RESERVED_itoa_BY_LANGUAGE_sign_done:
    addq %r9, %rcx
    movb $0, (%rcx)
    movq %r8, %rax
RESERVED_itoa_BY_LANGUAGE_emit_loop:
    decq %rcx
    xorq %rdx, %rdx
    movq $10, %rdi
    divq %rdi
    addb $'0', %dl
    movb %dl, (%rcx)
    testq %rax, %rax
    jnz RESERVED_itoa_BY_LANGUAGE_emit_loop
    ret

)";
                _assemblyCode.addInstructionToFunctions(itoaAssemblyCode);
            }
            if (_needsAtoi) {
                std::string atoiAssemblyCode = R"(
RESERVED_atoi_BY_LANGUAGE:
    leaq buffer(%rip), %rsi
    xorq %rax, %rax
    xorq %r10, %r10
    movb (%rsi), %cl
    cmpb $'-', %cl
    jne RESERVED_atoi_BY_LANGUAGE_loop
    movq $1, %r10
    incq %rsi
RESERVED_atoi_BY_LANGUAGE_loop:
    movzbq (%rsi), %rcx
    cmpb $'0', %cl
    jb RESERVED_atoi_BY_LANGUAGE_done
    cmpb $'9', %cl
    ja RESERVED_atoi_BY_LANGUAGE_done
    subb $'0', %cl
    imulq $10, %rax
    addq %rcx, %rax
    incq %rsi
    jmp RESERVED_atoi_BY_LANGUAGE_loop
RESERVED_atoi_BY_LANGUAGE_done:
    cmpq $0, %r10
    je RESERVED_atoi_BY_LANGUAGE_positive
    negq %rax
RESERVED_atoi_BY_LANGUAGE_positive:
    ret

)";
                _assemblyCode.addInstructionToFunctions(atoiAssemblyCode);
            }
            if (_needsNewline) {
                std::string newlineAssemblyCode = R"(
    newline: .ascii "\n"
)";
                _assemblyCode.addInstructionToData(newlineAssemblyCode);
            }
            std::string exitFunctionAssemblyCode = R"(
RESERVED_exit_BY_LANGUAGE:
    movq $60, %rax
    syscall

)";
            _assemblyCode.addInstructionToFunctions(exitFunctionAssemblyCode);
            if (!_conditionalMetadataStack.empty()) {
                reportError("Unterminated 'if' block (missing 'done')");
                _errorFlag = true;
                return;
            }
            if (!_functionStack.empty()) {
                reportError("Unterminated function (missing 'fdone')");
                _errorFlag = true;
                return;
            }
            if (!_compileTimeIfStack.empty()) {
                reportError("Unterminated 'if' block (missing 'done')");
                _errorFlag = true;
                return;
            }
            if (_errorFlag) { return; }
            std::string finalInstruction = R"(
    xorq %rdi, %rdi
    jmp RESERVED_exit_BY_LANGUAGE
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
                return;
            }
        }
};

#endif
