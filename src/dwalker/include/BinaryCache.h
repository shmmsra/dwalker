#pragma once

#include <string>
#include <map>
#include <vector>

#include <PEManager.h>

class BinaryCache {
private:
    std::map<std::string, PEManager*> BinaryDatabase;
    static std::unique_ptr<ApiSetSchemaBase> ApiSetmapCache;

    std::string GetBinaryHash(const std::wstring& PePath);
    void UpdateLRU(const std::string& PeHash);
    std::wstring LookupApiSetLibrary(std::wstring ImportDllName);

public:
    PEManager* GetBinary(const std::wstring& PePath);
    std::pair<ModuleSearchStrategy, PEManager*> BinaryCache::ResolveModule(
        PEManager* RootPe,
        std::wstring ModuleName);
    std::pair<ModuleSearchStrategy, PEManager*> ResolveModule(
        PEManager* RootPe,
        std::wstring ModuleName,
        SxsEntries SxsCache,
        std::vector<std::wstring> CustomSearchFolders,
        std::wstring WorkingDirectory);
};
