#ifndef COMPILER_CPP
#define COMPILER_CPP

#include "AssemblyCode.cpp"

#include <cstdint>
#include <vector>
#include <sstream>
#include <format>
#include <cstdlib>
#include <algorithm>
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
            IFEQUAL,
            IFGREATER,
            IFLESS,
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
            NEWARRAY,
            GETELEMENT,
            SETELEMENT,
            EDITMACRO,
            TERMNINATECOMPILATION,
            MACROFUNCTION,
            MACROFDONE,
            MACROEXECUTE,
        };
        struct ConditionalMetadata {
            uint16_t labelId;
            bool hasElse;
            bool isEmpty;
        };
        struct FunctionMetadata {
            uint16_t id;
            bool isEmpty;
        };
        struct ArrayMetadata {
            std::string name;
            uint16_t size;
        };
        std::ifstream _sourceCodeFile;
        AssemblyCode _assemblyCode;
        std::string _filename;
        std::string _currentMacroFunctionName;
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
        bool _needsRead;
        bool _needsPrint;
        bool _isInsideAMacroFunction;
        bool _isExecutingAMacroFunction;
        std::vector<ConditionalMetadata> _conditionalMetadataStack;
        std::vector<FunctionMetadata> _functionStack;
        std::vector<uint16_t> _compileTimeIfStack;
        std::vector<std::string> _macroFunctionCodeToExecute;
        std::vector<ArrayMetadata> _definedArrays;
        std::map<std::string, std::string> _definedMacroVariables;
        std::map<std::string, std::vector<std::string>> _definedMacroFunctions;
        std::set<std::string> _definedFunctions;
        std::set<std::string> _definedVariables;
        std::set<std::string> _definedMarks;
        InstructionType getInstructionType(const std::vector<std::string>& tokensVector) {
            if (tokensVector.empty()) return InstructionType::INVALID;
            if (tokensVector[0] == "new" &&
                tokensVector.size() >= 2 &&
                tokensVector[1] == "array") {
                return InstructionType::NEWARRAY;
            }
            if (tokensVector[0] == "get" &&
                tokensVector.size() >= 2 &&
                tokensVector[1] == "element") {
                return InstructionType::GETELEMENT;
            }
            if (tokensVector[0] == "set" &&
                tokensVector.size() >= 2 &&
                tokensVector[1] == "element") {
                return InstructionType::SETELEMENT;
            }
            if (tokensVector[0] == "go" &&
                tokensVector.size() >= 2 &&
                tokensVector[1] == "to") {
                return InstructionType::GOTO;
            }
            if (tokensVector[0] == "if" &&
                tokensVector.size() >= 4 &&
                tokensVector[3] == "greater") {
                return InstructionType::IFGREATER;
            }
            if (tokensVector[0] == "if" &&
                tokensVector.size() >= 4 &&
                tokensVector[3] == "less") {
                return InstructionType::IFLESS;
            }
            if (tokensVector[0] == "#macro") return InstructionType::MACRO;
            if (tokensVector[0] == "#compileTimeInfo") return InstructionType::COMPILETIMEINFO;
            if (tokensVector[0] == "#compileTimeWarning") return InstructionType::COMPILETIMEWARNING;
            if (tokensVector[0] == "#compileTimeError") return InstructionType::COMPILETIMEERROR;
            if (tokensVector[0] == "#compileTimeDebug") return InstructionType::COMPILETIMEDEBUG;
            if (tokensVector[0] == "#editMacro") return InstructionType::EDITMACRO;
            if (tokensVector[0] == "#terminateCompilation") return InstructionType::TERMNINATECOMPILATION;
            if (tokensVector[0] == "#function") return InstructionType::MACROFUNCTION;
            if (tokensVector[0] == "#fdone") return InstructionType::MACROFDONE;
            if (tokensVector[0] == "#execute") return InstructionType::MACROEXECUTE;
            if (tokensVector[0] == "info") return InstructionType::INFO;
            if (tokensVector[0] == "warning") return InstructionType::WARNING;
            if (tokensVector[0] == "error") return InstructionType::ERROR;
            if (tokensVector[0] == "debug") return InstructionType::DEBUG;
            if (tokensVector[0] == "#if") return InstructionType::COMPILETIMEIF;
            if (tokensVector[0] == "#done") return InstructionType::COMPILETIMEDONE;
            if (tokensVector[0] == "function") return InstructionType::FUNCTION;
            if (tokensVector[0] == "fdone") return InstructionType::FDONE;
            if (tokensVector[0] == "execute") return InstructionType::EXECUTE;
            if (tokensVector[0] == "printString") return InstructionType::PRINTSTRING;
            if (tokensVector[0] == "print") return InstructionType::PRINT;
            if (tokensVector[0] == "newline") return InstructionType::NEWLINE;
            if (tokensVector[0] == "new") return InstructionType::NEW;
            if (tokensVector[0] == "set") return InstructionType::SET;
            if (tokensVector[0] == "add") return InstructionType::ADD;
            if (tokensVector[0] == "subtract") return InstructionType::SUBTRACT;
            if (tokensVector[0] == "multiply") return InstructionType::MULTIPLY;
            if (tokensVector[0] == "divide") return InstructionType::DIVIDE;
            if (tokensVector[0] == "exit") return InstructionType::EXIT;
            if (tokensVector[0] == "done") return InstructionType::DONE;
            if (tokensVector[0] == "else") return InstructionType::ELSE;
            if (tokensVector[0] == "if") return InstructionType::IFEQUAL;
            if (tokensVector[0] == "read") return InstructionType::READ;
            if (tokensVector[0] == "nothing") return InstructionType::NOTHING;
            if (tokensVector[0] == "mark") return InstructionType::MARK;
            return InstructionType::INVALID;
        }
        bool doesTheArrayExist(const std::string& arrayName) {
            for (const ArrayMetadata& arrayMetadata : _definedArrays) {
                if (arrayMetadata.name == arrayName) {
                    return true;
                }
            }
            return false;
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
        bool doesTheMacroFunctionExist(const std::string& macroFunctionName) {
            return _definedMacroFunctions.contains(macroFunctionName);
        }
        bool doesTheMarkExist(const std::string& markName) {
            return _definedMarks.contains(markName);
        }
        bool isANumber(const std::string& string) {
            for (const char& c : string) {
                if (c < '0' || c > '9') {
                    return false;
                }
            }
            return true;
        }
        uint16_t getArrayElementCount(const ArrayMetadata& arrayMetadata) {
            if (_is64Bits) { return arrayMetadata.size / 8; }
            else { return arrayMetadata.size / 4; }
        }
        ArrayMetadata getArrayMetadata(const std::string& arrayName) {
            for (const ArrayMetadata& arrayMetadata : _definedArrays) {
                if (arrayMetadata.name == arrayName) {
                    return arrayMetadata;
                }
            }
            return { "", 0 };
        }
        bool doesTheAnythingAlreadyExist(const std::string& name) {
            return doesTheVariableExist(name) ||
                doesTheFunctionExist(name) ||
                doesTheMacroExist(name) ||
                doesTheMacroFunctionExist(name) ||
                doesTheArrayExist(name) ||
                doesTheMarkExist(name);
        }
    public:
        bool _errorFlag;
        bool _terminateCompilationFlag;
        static void reportError(const std::string& message) { std::cerr << "ERROR: " << message << std::endl; }
        static void reportWarning(const std::string& message) { std::cerr << "WARNING: " << message << std::endl; }
        static void reportInfo(const std::string& message) { std::cout << "INFO: " << message << std::endl; }
        static void reportDebug(const std::string& message) { std::cout << "DEBUG: " << message << std::endl; }
        Compiler(const std::string& filename, bool is64Bits) : _assemblyCode(filename) {
            _filename = filename;
            _sourceCodeFile.open(filename);
            _errorFlag = false;
            _terminateCompilationFlag = false;
            _is64Bits = is64Bits;
            _labelCounter = 0;
            _variableCounter = 0;
            _functionCounter = 0;
            _conditionalCounter = 0;
            _isInAFunction = false;
            _isExecutingAMacroFunction = false;
            _isSkippingCode = false;
            _needsBuffer = false;
            _needsGetStringLength = false;
            _needsItoa = false;
            _needsAtoi = false;
            _needsNewline = false;
            _needsRead = false;
            _needsPrint = false;
            _isInsideAMacroFunction = false;
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
            uint16_t arraySize = 0;
            uint16_t elementCount = 0;
            std::string line;
            while (_isExecutingAMacroFunction || !_sourceCodeFile.eof()) {
                if (_isExecutingAMacroFunction) {
                    if (_macroFunctionCodeToExecute.empty()) {
                        _isExecutingAMacroFunction = false;
                        continue;
                    }
                    line = _macroFunctionCodeToExecute.front();
                    _macroFunctionCodeToExecute.erase(_macroFunctionCodeToExecute.begin());
                } else {
                    std::getline(_sourceCodeFile, line);
                }
                if (_errorFlag || _terminateCompilationFlag) { return; }
                lineNumber++;
                if (line.empty()) { continue; }
                std::vector<std::string> tokens;
                std::stringstream ss(line);
                std::string token;
                std::string variableName;
                while (ss >> token) { tokens.push_back(token); }
                auto commentPosition = std::find(tokens.begin(), tokens.end(), "//");
                if (commentPosition != tokens.end()) {
                    tokens.erase(commentPosition, tokens.end());
                }
                if (tokens.empty()) { continue; }
                InstructionType instructionType = getInstructionType(tokens);
                if (_isSkippingCode && instructionType != InstructionType::COMPILETIMEDONE) { continue; }
                if (_isInsideAMacroFunction && instructionType != InstructionType::MACROFDONE) {
                    _definedMacroFunctions[_currentMacroFunctionName].push_back(line);
                    continue;
                }
                if (!_conditionalMetadataStack.empty() &&
                    instructionType != InstructionType::DONE &&
                    instructionType != InstructionType::ELSE) {
                    _conditionalMetadataStack.back().isEmpty = false;
                }
                if (!_functionStack.empty() && instructionType != InstructionType::FDONE) {
                    _functionStack.back().isEmpty = false;
                }
                try {
                    std::string assemblyInstruction;
                    std::string endMessage;
                    switch (instructionType) {
                        case InstructionType::PRINT:
                            _needsBuffer = true;
                            _needsItoa = true;
                            _needsGetStringLength = true;
                            _needsPrint = true;
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
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rax
    call RESERVED_itoa_BY_LANGUAGE
    leaq buffer(%rip), %rax
    call RESERVED_get_string_BY_length_LANGUAGE
    movq %rcx, %rdx
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            if (doesTheAnythingAlreadyExist(tokens[1])) {
                                reportError("Something with the same name as " + tokens[1] + " already exists");
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
                            if (tokens.size() != 4 || tokens[2] != "to") {
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
                            _needsPrint = true;
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = R"(
    movq $1, %rdi
    movq $1, %rdx
    leaq newline(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            if (!doesTheVariableExist(tokens[1])) {
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
                        case InstructionType::IFEQUAL:
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
                                reportError("Variable " + tokens[4] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            _conditionalMetadataStack.push_back({_labelCounter, false, true});
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
                        case InstructionType::IFGREATER:
                            if (tokens.size() != 8 ||
                            tokens[2] != "is" ||
                            tokens[3] != "greater" ||
                            tokens[4] != "than" ||
                            tokens[6] != "then" ||
                            tokens[7] != "do") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[5])) {
                                reportError("Variable " + tokens[5] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            _conditionalMetadataStack.push_back({_labelCounter, false, true});
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cmpq {}(%rip), %rax
    jle .Lelse_{}
)", tokens[1], tokens[5], _labelCounter);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cmpl {}(%rip), %eax
    jle .Lelse_{}
)", tokens[1], tokens[5], _labelCounter);
                            }
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            _labelCounter++;
                            break;
                        case InstructionType::IFLESS:
                            if (tokens.size() != 8 ||
                            tokens[2] != "is" ||
                            tokens[3] != "less" ||
                            tokens[4] != "than" ||
                            tokens[6] != "then" ||
                            tokens[7] != "do") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[5])) {
                                reportError("Variable " + tokens[5] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            _conditionalMetadataStack.push_back({_labelCounter, false, true});
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cmpq {}(%rip), %rax
    jge .Lelse_{}
)", tokens[1], tokens[5], _labelCounter);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cmpl {}(%rip), %eax
    jge .Lelse_{}
)", tokens[1], tokens[5], _labelCounter);
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
                            if (_conditionalMetadataStack.back().isEmpty) {
                                reportError("'if' block cannot be empty (use 'nothing') at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            _conditionalMetadataStack.back().isEmpty = true;
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
                            if (_conditionalMetadataStack.back().isEmpty) {
                                reportError("'if' block cannot be empty (use 'nothing') at line " + std::to_string(lineNumber));
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
                            _needsRead = true;
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
    leaq buffer(%rip), %rsi
    movq $33, %rdx
    call RESERVED_read_BY_LANGUAGE
    call RESERVED_atoi_BY_LANGUAGE
    movq %rax, {}(%rip)
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    leaq buffer(%rip), %rsi
    movq $33, %rdx
    call RESERVED_read_BY_LANGUAGE
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
                            _needsPrint = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    info_{}: .asciz "INFO: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else {
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
    movq $1, %rdi
    leaq info_{}(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            _needsPrint = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    warning_{}: .asciz "WARNING: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else {
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
    movq $2, %rdi
    leaq warning_{}(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            _needsPrint = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    error_{}: .asciz "ERROR: )", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else {
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
    movq $2, %rdi
    leaq error_{}(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else {
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
    movq $1, %rdi
    leaq debug_{}(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            _needsPrint = true;
                            if (tokens.size() == 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = std::format(R"(
    string_{}: .asciz ")", _variableCounter);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    assemblyInstruction = std::format("{} ", tokens[i]);
                                    _assemblyCode.addInstructionToData(assemblyInstruction);
                                }
                                else {
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
    movq $1, %rdi
    leaq string_{}(%rip), %rsi
    call RESERVED_print_BY_LANGUAGE
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
                            if (doesTheAnythingAlreadyExist(tokens[1])) {
                                reportError("Something with the same name as " + tokens[1] + " already exists");
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
                            _functionStack.push_back({_functionCounter, true});
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
                            if (_functionStack.back().isEmpty) {
                                reportError("Function cannot be empty (use 'nothing') at line " + std::to_string(lineNumber));
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
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else {
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
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else {
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
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else {
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
                            for (uint16_t i = 1; i < tokens.size(); i++) {
                                if (i != tokens.size() - 1) {
                                    endMessage += std::format("{} ", tokens[i]);
                                }
                                else {
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
                            if (doesTheAnythingAlreadyExist(tokens[1])) {
                                reportError("Something with the same name as " + tokens[1] + " already exists");
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
                            if (doesTheAnythingAlreadyExist(tokens[1])) {
                                reportError("Something with the same name as " + tokens[1] + " already exists");
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
                        case InstructionType::NEWARRAY:
                            if (tokens.size() != 6 ||
                                tokens[0] != "new" ||
                                tokens[1] != "array" ||
                                tokens[3] != "with" ||
                                tokens[5] != "elements") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheAnythingAlreadyExist(tokens[2])) {
                                reportError("Something with the same name as " + tokens[2] + " already exists");
                                _errorFlag = true;
                                continue;
                            }
                            if (!isANumber(tokens[4])) {
                                reportError("Array size is not a number");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) { arraySize = std::stoi(tokens[4]) * 8; }
                            else { arraySize = std::stoi(tokens[4]) * 4; }
                            assemblyInstruction = std::format(R"(
    {}: .zero {}
)", tokens[2], arraySize);
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            _definedArrays.push_back({ tokens[2], arraySize });
                            break;
                        case InstructionType::GETELEMENT:
                            if (tokens.size() != 10 ||
                                tokens[0] != "get" ||
                                tokens[1] != "element" ||
                                tokens[3] != "from" ||
                                tokens[4] != "array" ||
                                tokens[6] != "and" ||
                                tokens[7] != "put" ||
                                tokens[8] != "into") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheArrayExist(tokens[5])) {
                                reportError("Array " + tokens[5] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[9])) {
                                reportError("Variable " + tokens[9] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[2])) {
                                reportError("Variable " + tokens[2] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            elementCount = getArrayElementCount(getArrayMetadata(tokens[5]));
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rcx
    cmpq $1, %rcx
    jl .Larray_oob_{}
    cmpq ${}, %rcx
    jg .Larray_oob_{}
    decq %rcx
    leaq {}(%rip), %rax
    movq (%rax,%rcx,8), %rdx
    movq %rdx, {}(%rip)
    jmp .Larray_done_{}
.Larray_oob_{}:
    movq $1, %rdi
    jmp RESERVED_exit_BY_LANGUAGE
.Larray_done_{}:
)", tokens[2], _labelCounter, elementCount, _labelCounter, tokens[5], tokens[9], _labelCounter, _labelCounter, _labelCounter);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rcx
    cmpq $1, %rcx
    jl .Larray_oob_{}
    cmpq ${}, %rcx
    jg .Larray_oob_{}
    decq %rcx
    leaq {}(%rip), %rax
    movl (%rax,%rcx,4), %edx
    movl %edx, {}(%rip)
    jmp .Larray_done_{}
.Larray_oob_{}:
    movq $1, %rdi
    jmp RESERVED_exit_BY_LANGUAGE
.Larray_done_{}:
)", tokens[2], _labelCounter, elementCount, _labelCounter, tokens[5], tokens[9], _labelCounter, _labelCounter, _labelCounter);
                            }
                            _labelCounter++;
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::SETELEMENT:
                            if (tokens.size() != 9 ||
                                tokens[0] != "set" ||
                                tokens[1] != "element" ||
                                tokens[3] != "from" ||
                                tokens[4] != "array" ||
                                tokens[6] != "to" ||
                                tokens[7] != "be") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheArrayExist(tokens[5])) {
                                reportError("Array " + tokens[5] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[8])) {
                                reportError("Variable " + tokens[8] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[2])) {
                                reportError("Variable " + tokens[2] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            elementCount = getArrayElementCount(getArrayMetadata(tokens[5]));
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rcx
    cmpq $1, %rcx
    jl .Larray_oob_{}
    cmpq ${}, %rcx
    jg .Larray_oob_{}
    decq %rcx
    leaq {}(%rip), %rax
    movq {}(%rip), %rbx
    movq %rbx, (%rax,%rcx,8)
    jmp .Larray_done_{}
.Larray_oob_{}:
    movq $1, %rdi
    jmp RESERVED_exit_BY_LANGUAGE
.Larray_done_{}:
)", tokens[2], _labelCounter, elementCount, _labelCounter, tokens[5], tokens[8], _labelCounter, _labelCounter, _labelCounter);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rcx
    cmpq $1, %rcx
    jl .Larray_oob_{}
    cmpq ${}, %rcx
    jg .Larray_oob_{}
    decq %rcx
    leaq {}(%rip), %rax
    movl {}(%rip), %ebx
    movl %ebx, (%rax,%rcx,4)
    jmp .Larray_done_{}
.Larray_oob_{}:
    movq $1, %rdi
    jmp RESERVED_exit_BY_LANGUAGE
.Larray_done_{}:
)", tokens[2], _labelCounter, elementCount, _labelCounter, tokens[5], tokens[8], _labelCounter, _labelCounter, _labelCounter);
                            }
                            _labelCounter++;
                            if (_isInAFunction) {
                                _assemblyCode.addInstructionToFunctions(assemblyInstruction);
                            } else {
                                _assemblyCode.addInstructionToText(assemblyInstruction);
                            }
                            break;
                        case InstructionType::EDITMACRO:
                            if (tokens.size() != 4 || tokens[2] != "to") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheMacroExist(tokens[1])) {
                                reportError("Macro " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (!isANumber(tokens[3])) {
                                reportError("Value is not a number");
                                _errorFlag = true;
                                continue;
                            }
                            _definedMacroVariables[tokens[1]] = tokens[3];
                            break;
                        case InstructionType::TERMNINATECOMPILATION:
                            _needsGetStringLength = true;
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            _terminateCompilationFlag = true;
                            break;
                        case InstructionType::MACROFUNCTION:
                            if (tokens.size() != 3 || tokens[2] != "does") {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheAnythingAlreadyExist(tokens[1])) {
                                reportError("Something with the same name as " + tokens[1] + " already exists");
                                _errorFlag = true;
                                continue;
                            }
                            _definedMacroFunctions[tokens[1]] = {};
                            _isInsideAMacroFunction = true;
                            _currentMacroFunctionName = tokens[1];
                            break;
                        case InstructionType::MACROFDONE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!_isInsideAMacroFunction) {
                                reportError("'#fdone' outside of a macro function at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (_definedMacroFunctions[_currentMacroFunctionName].empty()) {
                                reportError("Macro function cannot be empty (use 'nothing') at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            _isInsideAMacroFunction = false;
                            break;
                        case InstructionType::MACROEXECUTE:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheMacroFunctionExist(tokens[1])) {
                                reportError("Macro function " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            _isExecutingAMacroFunction = true;
                            _macroFunctionCodeToExecute = _definedMacroFunctions[tokens[1]];
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
            if (_needsPrint) {
                std::string printAssemblyCode = R"(
RESERVED_print_BY_LANGUAGE:
    movq $1, %rax
    syscall
    ret

)";
                _assemblyCode.addInstructionToFunctions(printAssemblyCode);
            }
            if (_needsRead) {
                std::string readAssemblyCode = R"(
RESERVED_read_BY_LANGUAGE:
    movq $0, %rax
    movq $0, %rdi
    syscall
    ret

)";
                _assemblyCode.addInstructionToFunctions(readAssemblyCode);
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
            if (_isInsideAMacroFunction) {
                reportError("Unterminated macro function (missing '#fdone')");
                _errorFlag = true;
                return;
            }
            if (!_compileTimeIfStack.empty()) {
                reportError("Unterminated 'if' block (missing 'done')");
                _errorFlag = true;
                return;
            }
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
