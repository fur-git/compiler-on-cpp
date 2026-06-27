#ifndef ASSEMBLY_CODE_CPP
#define ASSEMBLY_CODE_CPP

#include <fstream>
#include <string>
#include <iostream>

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
            << std::flush;
        }
};

#endif
