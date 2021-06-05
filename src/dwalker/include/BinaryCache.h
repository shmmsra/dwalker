#pragma once

#include <string>
#include <map>
#include <vector>

#include <PEManager.h>
#include <FindPE.h>

using namespace std;

class BinaryCache {
private:
    map<string, PEManager*> BinaryDatabase;
    static unique_ptr<ApiSetSchemaBase> ApiSetmapCache;

    string GetBinaryHash(const wstring& PePath);
    void UpdateLRU(const string& PeHash);
    wstring LookupApiSetLibrary(wstring ImportDllName);

public:
    PEManager* GetBinary(const wstring& PePath);
    pair<ModuleSearchStrategy, PEManager*> BinaryCache::ResolveModule(
        PEManager* RootPe,
        wstring ModuleName);
    pair<ModuleSearchStrategy, PEManager*> ResolveModule(
        PEManager* RootPe,
        wstring ModuleName,
        SxsEntries SxsCache,
        vector<wstring> CustomSearchFolders,
        wstring WorkingDirectory);
};
