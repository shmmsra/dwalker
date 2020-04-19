#include <DWalker.h>

DWalker::DWalker() : binaryCache(NULL) {
    this->binaryCache = new BinaryCache();
}

DWalker::~DWalker() {
    if (binaryCache) {
        delete binaryCache;
        binaryCache = NULL;
    }
}

bool DWalker::DumpDependencyChain(const wstring& filePath) {
    PEManager* peManager = binaryCache->GetBinary(filePath);
    if (!peManager) {
        return false;
    }

    const bool is32Bit = peManager->IsWow64Dll();
    vector<PeImportDll> imports = peManager->GetImports();

    pair<ModuleSearchStrategy, PEManager*> t = binaryCache->ResolveModule(peManager, L"DWalker.exe", SxsEntries(), {});

    return true;
}
