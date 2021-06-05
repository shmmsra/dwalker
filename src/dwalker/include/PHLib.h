#pragma once

#include <string>
#include <set>
#include <memory>

#include <ph.h>
#include <phnet.h>
#include <PEManager.h>

class PHLib {
private:
    static PHLib* _instance;
    bool bInitializedPhLib;
    std::set<std::wstring> KnownDll64List;
    std::set<std::wstring> KnownDll32List;

    PHLib();
    bool BuildKnownDllList(_In_ bool Wow64Dlls);
    static BOOLEAN NTAPI PhEnumDirectoryObjectsCallback(
        _In_ PPH_STRINGREF Name,
        _In_ PPH_STRINGREF TypeName,
        _In_opt_ PVOID Context
    );

public:
    static PHLib* GetInstance();

    // Imitialize Process Hacker's phlib internal data
    // Must be called before any other API (kinda like OleInitialize).
    bool InitializePhLib();

    // Return the list of knwown dll for this system
    std::set<std::wstring> GetKnownDlls(_In_ bool Wow64Dlls);

    // Return the Api Set schema:
    // NB: Api set resolution rely on hash buckets who 
    // can contains more entries than this schema.
    std::unique_ptr<ApiSetSchemaBase> GetApiSetSchema();
};
