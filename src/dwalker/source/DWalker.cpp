#include <DWalker.hpp>
#include <PEManager.hpp>
#include <BinaryCache.hpp>
#include <filesystem>
#include <Logger.hpp>

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
    LOG_DEBUG_FUNC(L"DumpDependencyChain", L"called with: " + filePath);
    processedFiles.clear();
    LOG_DEBUG_FUNC(L"DumpDependencyChain", L"About to print analysis header...");
    
    try {
        LOG_DEBUG_FUNC(L"DumpDependencyChain", L"Getting filename from path...");
        wstring filename = filesystem::path(filePath).filename();
        LOG_DEBUG_FUNC_VAL(L"DumpDependencyChain", L"Filename extracted", filename);
        
        LOG_DEBUG_FUNC(L"DumpDependencyChain", L"Writing first output line...");
        *outputStream << L"[ANALYZE] Analyzing dependencies for: " << filename << endl;
        LOG_DEBUG_FUNC(L"DumpDependencyChain", L"First output line written successfully.");
        
        LOG_DEBUG_FUNC(L"DumpDependencyChain", L"Writing second output line...");
        *outputStream << L"[PATH] Full path: " << filePath << endl << endl;
        LOG_DEBUG_FUNC(L"DumpDependencyChain", L"Second output line written successfully.");
    } catch (const std::exception& e) {
        wstring errorMsg = L"Exception caught: ";
        errorMsg += wstring(e.what(), e.what() + strlen(e.what()));
        LOG_ERROR(errorMsg);
        return false;
    } catch (...) {
        LOG_ERROR(L"Unknown exception caught!");
        return false;
    }
    
    LOG_DEBUG_FUNC(L"DumpDependencyChain", L"About to call DumpDependencyChainInternal...");
    
    bool result = DumpDependencyChainInternal(filePath, 0);
    LOG_DEBUG_FUNC_VAL(L"DumpDependencyChain", L"DumpDependencyChainInternal returned", (result ? L"true" : L"false"));
    return result;
}

bool DWalker::DumpDependencyChainInternal(const wstring& filePath, int depth) {
    LOG_DEBUG_FUNC_VAL(L"DumpDependencyChainInternal", L"called with depth " + to_wstring(depth), filePath);
    
    // Prevent infinite recursion and respect max depth
    if (depth > maxDepth) {
        PrintIndent();
        *outputStream << L"[WARNING] Maximum depth reached (" << maxDepth << L")" << endl;
        return true;
    }
    
    // Mark this file as processed
    processedFiles.insert(filePath);
    
    // Load the current user provided binary and save it in the cache as well
    LOG_DEBUG_FUNC(L"DumpDependencyChainInternal", L"About to call BinaryCache::GetBinary...");
    PEManager* peManager = binaryCache->GetBinary(filePath);
    LOG_DEBUG_FUNC_VAL(L"DumpDependencyChainInternal", L"BinaryCache::GetBinary returned", (peManager ? L"valid pointer" : L"nullptr"));
    if (!peManager) {
        PrintIndent();
        *outputStream << L"[ERROR] Failed to load: " << filesystem::path(filePath).filename() << endl;
        return false;
    }
    LOG_DEBUG_FUNC(L"DumpDependencyChainInternal", L"PE file loaded successfully.");

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
        
        // Check if this module is already processed
        bool isAlreadyProcessed = false;
        if (result.second) {
            isAlreadyProcessed = (processedFiles.find(result.second->filepath) != processedFiles.end());
        }
        
        if (isAlreadyProcessed) {
            // Skip this module entirely - don't print anything
            continue;
        }
        
        // Print module info for new modules
        PrintModuleInfo(result.second, result.first);
        
        if (result.first == ModuleSearchStrategy::NOT_FOUND) {
            // Continue with other dependencies even if one is not found
            continue;
        }
        
        if (result.second) {
            // Recursively analyze dependencies only for new modules
            if (!DumpDependencyChainInternal(result.second->filepath, depth + 1)) {
                // Continue even if recursive analysis fails
                continue;
            }
        }
    }

    return true;
}
