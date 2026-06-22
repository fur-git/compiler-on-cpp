#include "modules/ClassDefinitions.cpp"

int main(int argc, char* argv[]) {
    bool clearObjectFiles = false;
    bool clearAssemblyFiles = false;
    bool is64Bits = true;
    std::string sourceFile;
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == "--clearObjectFiles") { clearObjectFiles = true; }
        else if (argument == "--clearAssemblyFiles") { clearAssemblyFiles = true; }
        else if (argument == "--64bits") { is64Bits = true; }
        else if (argument == "--32bits") { is64Bits = false; }
        else if (sourceFile.empty()) { sourceFile = argument; }
        else { std::cerr << "Usage: " << argv[0] << " [flags] <source-file>\n"; return 1; }
    }
    if (sourceFile.empty()) {
        std::cerr << "Usage: " << argv[0] << " [flags] <source-file>\n";
        return 1;
    }
    Compiler compiler(sourceFile, is64Bits);
    compiler.compile();
    if (!compiler._errorFlag) {
        if (clearObjectFiles) {
            Compiler::reportInfo("Clearing object files...");
            system("rm -f *.o");
            Compiler::reportInfo("Object files cleared");
        }
        if (clearAssemblyFiles) {
            Compiler::reportInfo("Clearing assembly files...");
            system("rm -f *.S");
            Compiler::reportInfo("Assembly files cleared");
        }
    } else {
        Compiler::reportError("Compilation failed");
        Compiler::reportInfo("Cleaning up...");
        std::string base = sourceFile.substr(0, sourceFile.find_last_of('.'));
        system(std::format("rm -f {}.S", base).c_str());
        return 1;
    }
    return 0;
}
