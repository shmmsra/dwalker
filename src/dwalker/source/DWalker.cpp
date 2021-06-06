#include <DWalker.hpp>
#include <PEManager.hpp>
#include <BinaryCache.hpp>

using namespace std;

DWalker::DWalker() : binaryCache(NULL) {
    this->binaryCache = new BinaryCache();
}

DWalker::~DWalker() {
    if (binaryCache) {
        delete binaryCache;
        binaryCache = NULL;
    }
}

/*
 * Recursively extract dependencies of the given file at filePath
 */
bool DWalker::DumpDependencyChain(const wstring& filePath) {
    // Load the current user provided binary and save it in the cache as well
    PEManager* peManager = binaryCache->GetBinary(filePath);
    if (!peManager) {
        return false;
    }

    const bool is32Bit = peManager->IsWow64Dll();

    // Get the list of dependencies for the binary
    // we would then loop over these dependencies recursively to further resolve their dependencies
    vector<PeImportDll> imports = peManager->GetImports();

    for (auto& x : imports) {
        std::pair<ModuleSearchStrategy, PEManager*> t = binaryCache->ResolveModule(peManager, L"DWalker.exe");
        // Recursively load all the dependencies
        if (!DumpDependencyChain(t.second->filepath)) {
            return false;
        }
        // TODO(unknown): To be removed later
        break;
    }

    return true;
}
