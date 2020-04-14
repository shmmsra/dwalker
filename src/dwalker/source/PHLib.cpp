#include <PHLib.h>

PHLib* PHLib::_instance = NULL;

PHLib::PHLib() : bInitializedPhLib(false) {
}

PHLib* PHLib::GetInstance() {
    if (!_instance) {
        _instance = new PHLib();
    }
    return _instance;
}

bool PHLib::InitializePhLib() {
    if (!bInitializedPhLib) {
        bInitializedPhLib = NT_SUCCESS(PhInitializePhLib());
    }

    BuildKnownDllList(false);   // Build 64-bit dlls
    BuildKnownDllList(true);    // Build 32-bit dlls

    return bInitializedPhLib;
}

BOOLEAN NTAPI PHLib::PhEnumDirectoryObjectsCallback(
    _In_ PPH_STRINGREF Name,
    _In_ PPH_STRINGREF TypeName,
    _In_opt_ PVOID Context
) {
    static PH_STRINGREF SectionTypeName = PH_STRINGREF_INIT(L"Section");
    set<wstring>* ReturnList = ((set<wstring>*)Context);

    if (!PhCompareStringRef(&SectionTypeName, TypeName, TRUE)) {
        ReturnList->insert(Name->Buffer);
    }

    return TRUE;
}

bool PHLib::BuildKnownDllList(_In_ bool Wow64Dlls) {
    HANDLE KnownDllDir = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING name;
    NTSTATUS status;

    const PWCHAR KnownDllObjectName = (Wow64Dlls) ? L"\\KnownDlls32" : L"\\KnownDlls";

    name.Length = (USHORT)wcslen(KnownDllObjectName) * sizeof(wchar_t);
    name.MaximumLength = (USHORT)wcslen(KnownDllObjectName) * sizeof(wchar_t);
    name.Buffer = KnownDllObjectName;


    InitializeObjectAttributes(
        &oa,
        &name,
        0,
        NULL,
        NULL
    );

    status = NtOpenDirectoryObject(
        &KnownDllDir,
        DIRECTORY_QUERY,
        &oa
    );

    if (!NT_SUCCESS(status)) {
        return false;
    }

    if (Wow64Dlls) {
        status = PhEnumDirectoryObjects(
            KnownDllDir,
            (PPH_ENUM_DIRECTORY_OBJECTS)PhEnumDirectoryObjectsCallback,
            (PVOID)&KnownDll32List
        );
    } else {
        status = PhEnumDirectoryObjects(
            KnownDllDir,
            (PPH_ENUM_DIRECTORY_OBJECTS)PhEnumDirectoryObjectsCallback,
            (PVOID)&KnownDll64List
        );
    }

    if (!NT_SUCCESS(status)) {
        return false;
    }

    return true;
}

set<wstring> PHLib::GetKnownDlls(_In_ bool Wow64Dlls) {
    if (Wow64Dlls) {
        return KnownDll32List;
    }

    return KnownDll64List;
}
