#pragma once

#include <string>
#include <set>
#include <memory>

#include <ph.h>
#include <phnet.h>
#include <PEManager.h>

using namespace std;

class PHLib {
private:
    static PHLib* _instance;
    bool bInitializedPhLib;
    set<wstring> KnownDll64List;
    set<wstring> KnownDll32List;

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
    set<wstring> GetKnownDlls(_In_ bool Wow64Dlls);

    // Return the Api Set schema:
    // NB: Api set resolution rely on hash buckets who 
    // can contains more entries than this schema.
    unique_ptr<ApiSetSchemaBase> GetApiSetSchema();
};
