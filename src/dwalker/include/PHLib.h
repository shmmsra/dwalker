#pragma once

#include <string>
#include <set>

#include <ph.h>
#include <phnet.h>

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
    bool InitializePhLib();
    set<wstring> GetKnownDlls(_In_ bool Wow64Dlls);
};
