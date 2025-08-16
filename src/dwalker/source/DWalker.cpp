#include <DWalker.hpp>
#include <PEManager.hpp>
#include <BinaryCache.hpp>
#include <filesystem>

using namespace std;

DWalker::DWalker(std::wostream& output) : 
    binaryCache(nullptr), 
    indentLevel(0), 
    outputStream(&output),
    maxDepth(10),
    verbose(false) {
    this->binaryCache = new BinaryCache();
}

DWalker::~DWalker() {
    if (binaryCache) {
        delete binaryCache;
        binaryCache = nullptr;
    }
}

void DWalker::PrintIndent() {
    for (int i = 0; i < indentLevel; i++) {
        *outputStream << L"  ";
    }
}

const wchar_t* GetModuleSearchStrategyName(ModuleSearchStrategy strategy) {
    switch (strategy) {
        case ModuleSearchStrategy::ROOT: return L"ROOT";
        case ModuleSearchStrategy::SxS: return L"SxS";
        case ModuleSearchStrategy::ApiSetSchema: return L"ApiSet";
        case ModuleSearchStrategy::WellKnownDlls: return L"KnownDLL";
        case ModuleSearchStrategy::ApplicationDirectory: return L"AppDir";
        case ModuleSearchStrategy::System32Folder: return L"System32";
        case ModuleSearchStrategy::WindowsFolder: return L"Windows";
        case ModuleSearchStrategy::WorkingDirectory: return L"WorkDir";
        case ModuleSearchStrategy::Environment: return L"PATH";
        case ModuleSearchStrategy::Fullpath: return L"FullPath";
        case ModuleSearchStrategy::UserDefined: return L"UserDef";
        case ModuleSearchStrategy::NOT_FOUND: return L"NOT_FOUND";
        default: return L"Unknown";
    }
}

void DWalker::PrintModuleInfo(PEManager* peManager, ModuleSearchStrategy strategy) {
    PrintIndent();
    
    if (!peManager) {
        *outputStream << L"❌ [NOT_FOUND]" << endl;
        return;
    }
    
    wstring fileName = filesystem::path(peManager->filepath).filename();
    wstring strategyName = GetModuleSearchStrategyName(strategy);
    
    *outputStream << L"📦 " << fileName;
    
    if (verbose) {
        *outputStream << L" [" << strategyName << L"]";
        if (peManager->IsWow64Dll()) {
            *outputStream << L" (32-bit)";
        } else {
            *outputStream << L" (64-bit)";
        }
        *outputStream << L" -> " << peManager->filepath;
    }
    
    *outputStream << endl;
}

/*
 * Recursively extract dependencies of the given file at filePath
 */
bool DWalker::DumpDependencyChain(const wstring& filePath) {
    processedFiles.clear();
    *outputStream << L"🔍 Analyzing dependencies for: " << filesystem::path(filePath).filename() << endl;
    *outputStream << L"📁 Full path: " << filePath << endl << endl;
    
    return DumpDependencyChainInternal(filePath, 0);
}

bool DWalker::DumpDependencyChainInternal(const wstring& filePath, int depth) {
    // Prevent infinite recursion and respect max depth
    if (depth > maxDepth) {
        PrintIndent();
        *outputStream << L"⚠️  Maximum depth reached (" << maxDepth << L")" << endl;
        return true;
    }
    
    // Check for circular dependencies
    if (processedFiles.find(filePath) != processedFiles.end()) {
        PrintIndent();
        *outputStream << L"🔄 [CIRCULAR] " << filesystem::path(filePath).filename() << endl;
        return true;
    }
    
    processedFiles.insert(filePath);
    
    // Load the current user provided binary and save it in the cache as well
    PEManager* peManager = binaryCache->GetBinary(filePath);
    if (!peManager) {
        PrintIndent();
        *outputStream << L"❌ Failed to load: " << filesystem::path(filePath).filename() << endl;
        return false;
    }

    // Print current module info (for root, strategy is ROOT)
    ModuleSearchStrategy rootStrategy = (depth == 0) ? ModuleSearchStrategy::ROOT : ModuleSearchStrategy::NOT_FOUND;
    if (depth == 0) {
        PrintModuleInfo(peManager, rootStrategy);
    }

    // Get the list of dependencies for the binary
    vector<PeImportDll> imports = peManager->GetImports();
    
    if (imports.empty()) {
        if (depth == 0) {
            PrintIndent();
            *outputStream << L"  ✅ No dependencies found." << endl;
        }
        return true;
    }

    indentLevel = depth + 1;
    
    for (auto& import : imports) {
        wstring moduleName = wstring(import.Name.begin(), import.Name.end());
        
        std::pair<ModuleSearchStrategy, PEManager*> result = 
            binaryCache->ResolveModule(peManager, moduleName);
        
        PrintModuleInfo(result.second, result.first);
        
        if (result.first == ModuleSearchStrategy::NOT_FOUND) {
            // Continue with other dependencies even if one is not found
            continue;
        }
        
        if (result.second) {
            // Recursively analyze dependencies
            if (!DumpDependencyChainInternal(result.second->filepath, depth + 1)) {
                // Continue even if recursive analysis fails
                continue;
            }
        }
    }

    return true;
}
