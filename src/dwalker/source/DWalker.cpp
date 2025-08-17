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
        *outputStream << L"[NOT_FOUND] [NOT_FOUND]" << endl;
        return;
    }
    
    wstring fileName = filesystem::path(peManager->filepath).filename();
    wstring strategyName = GetModuleSearchStrategyName(strategy);
    
            *outputStream << L"[MODULE] " << fileName;
    
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
    wcout << L"Debug: DumpDependencyChain called with: " << filePath << endl;
    processedFiles.clear();
    wcout << L"Debug: About to print analysis header..." << endl;
    
    try {
        wcout << L"Debug: Getting filename from path..." << endl;
        wstring filename = filesystem::path(filePath).filename();
        wcout << L"Debug: Filename extracted: " << filename << endl;
        
        wcout << L"Debug: Writing first output line..." << endl;
        *outputStream << L"[ANALYZE] Analyzing dependencies for: " << filename << endl;
        wcout << L"Debug: First output line written successfully." << endl;
        
        wcout << L"Debug: Writing second output line..." << endl;
        *outputStream << L"[PATH] Full path: " << filePath << endl << endl;
        wcout << L"Debug: Second output line written successfully." << endl;
    } catch (const std::exception& e) {
        wcout << L"Debug: Exception caught: " << e.what() << endl;
        return false;
    } catch (...) {
        wcout << L"Debug: Unknown exception caught!" << endl;
        return false;
    }
    
    wcout << L"Debug: About to call DumpDependencyChainInternal..." << endl;
    
    bool result = DumpDependencyChainInternal(filePath, 0);
    wcout << L"Debug: DumpDependencyChainInternal returned: " << (result ? L"true" : L"false") << endl;
    return result;
}

bool DWalker::DumpDependencyChainInternal(const wstring& filePath, int depth) {
    wcout << L"Debug: DumpDependencyChainInternal called with depth " << depth << L" for: " << filePath << endl;
    
    // Prevent infinite recursion and respect max depth
    if (depth > maxDepth) {
        PrintIndent();
        *outputStream << L"[WARNING] Maximum depth reached (" << maxDepth << L")" << endl;
        return true;
    }
    
    // Check for circular dependencies
    if (processedFiles.find(filePath) != processedFiles.end()) {
        PrintIndent();
        *outputStream << L"[CIRCULAR] [CIRCULAR] " << filesystem::path(filePath).filename() << endl;
        return true;
    }
    
    processedFiles.insert(filePath);
    
    // Load the current user provided binary and save it in the cache as well
    wcout << L"Debug: About to call BinaryCache::GetBinary..." << endl;
    PEManager* peManager = binaryCache->GetBinary(filePath);
    wcout << L"Debug: BinaryCache::GetBinary returned: " << (peManager ? L"valid pointer" : L"nullptr") << endl;
    if (!peManager) {
        PrintIndent();
        *outputStream << L"[ERROR] Failed to load: " << filesystem::path(filePath).filename() << endl;
        return false;
    }
    wcout << L"Debug: PE file loaded successfully." << endl;

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
            *outputStream << L"  [OK] No dependencies found." << endl;
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
