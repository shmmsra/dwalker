#include <vector>

#include <PEManager.hpp>
#include <ApiSet.hpp>
#include <phnative.h>
#include <ntpsapi.h>

using namespace std;

PeImport::PeImport(
    _In_ const PPH_MAPPED_IMAGE_IMPORT_DLL importDll,
    _In_ size_t Index
) {
    PH_MAPPED_IMAGE_IMPORT_ENTRY importEntry;

    if (NT_SUCCESS(PhGetMappedImageImportEntry((PPH_MAPPED_IMAGE_IMPORT_DLL)importDll, (ULONG)Index, &importEntry))) {
        this->Hint = importEntry.NameHint;
        this->Ordinal = importEntry.Ordinal;
        this->DelayImport = (importDll->Flags) & PH_MAPPED_IMAGE_DELAY_IMPORTS;

        // TODO(unknown): Should check if wstring is required for Name and ModuleName
        this->Name = (importEntry.Name == nullptr) ? "" : std::string(importEntry.Name);
        this->ModuleName = (importDll->Name == nullptr) ? "" : std::string(importDll->Name);

        this->ImportByOrdinal = (importEntry.Name == nullptr);
    }
}

PeImport::PeImport(
    _In_ const PeImport& other
) {
    this->Hint = other.Hint;
    this->Ordinal = other.Ordinal;
    this->DelayImport = other.DelayImport;
    this->Name = other.Name;
    this->ModuleName = other.ModuleName;
    this->ImportByOrdinal = other.ImportByOrdinal;
}

PeImport::~PeImport() {
}

PeImportDll::PeImportDll(
    _In_ const PPH_MAPPED_IMAGE_IMPORTS& PvMappedImports,
    _In_ size_t ImportDllIndex
) : ImportDll(new PH_MAPPED_IMAGE_IMPORT_DLL) {
    if (!NT_SUCCESS(PhGetMappedImageImportDll(PvMappedImports, (ULONG)ImportDllIndex, ImportDll))) {
        Flags = 0;
        Name = std::string("## PeImportDll error: Invalid DllName ##");  // TODO(unknown): Should check if wstring is required
        NumberOfEntries = 0;
        return;
    }

    Flags = ImportDll->Flags;
    Name = (ImportDll->Name == nullptr) ? "" : string(ImportDll->Name); // TODO(unknown): Should check if wstring is required
    NumberOfEntries = ImportDll->NumberOfEntries;

    for (size_t IndexImport = 0; IndexImport < (size_t)NumberOfEntries; IndexImport++) {
        ImportList.push_back(PeImport(ImportDll, IndexImport));
    }
}

PeImportDll::~PeImportDll() {
    delete ImportDll;
}

PeImportDll::PeImportDll(
    _In_ const PeImportDll& other
) : ImportDll(new PH_MAPPED_IMAGE_IMPORT_DLL) {
    memcpy(ImportDll, other.ImportDll, sizeof(PH_MAPPED_IMAGE_IMPORT_DLL));

    Flags = other.Flags;
    Name = other.Name;
    NumberOfEntries = other.NumberOfEntries;

    for (size_t IndexImport = 0; IndexImport < (size_t)NumberOfEntries; IndexImport++) {
        ImportList.push_back(PeImport(other.ImportList[(int)IndexImport]));
    }
}

bool PeImportDll::IsDelayLoad() {
    return this->Flags & PH_MAPPED_IMAGE_DELAY_IMPORTS;
}


PEManager::PEManager(const wstring& filepath) : loadSuccessful(false), m_ExportsInit(false), m_ImportsInit(false) {
    this->m_Impl = new PE();
    this->filepath = filepath;
}

PEManager::~PEManager() {
}

bool PEManager::Load() {
    loadSuccessful = this->m_Impl->LoadPE((LPWSTR)filepath.c_str());
    if (!loadSuccessful) {
        return false;
    }

    // Parse PE
    loadSuccessful &= InitProperties();
    if (!loadSuccessful) {
        m_Impl->UnloadPE();
        return false;
    }

    return loadSuccessful;
}

void PEManager::Unload() {
    if (loadSuccessful) {
        m_Impl->UnloadPE();
    }
}

bool PEManager::InitProperties() {
    LARGE_INTEGER time;
    SYSTEMTIME systemTime;

    PH_MAPPED_IMAGE PvMappedImage = m_Impl->m_PvMappedImage;

    properties = new PeProperties();
    properties->Machine = PvMappedImage.NtHeaders->FileHeader.Machine;
    properties->Magic = m_Impl->m_PvMappedImage.Magic;
    properties->Checksum = PvMappedImage.NtHeaders->OptionalHeader.CheckSum;
    properties->CorrectChecksum = (properties->Checksum == PhCheckSumMappedImage(&PvMappedImage));

    RtlSecondsSince1970ToTime(PvMappedImage.NtHeaders->FileHeader.TimeDateStamp, &time);
    PhLargeIntegerToLocalSystemTime(&systemTime, &time);
    //properties->Time = gcnew DateTime (systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds, DateTimeKind::Local);

    if (PvMappedImage.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        PIMAGE_OPTIONAL_HEADER32 OptionalHeader = (PIMAGE_OPTIONAL_HEADER32)&PvMappedImage.NtHeaders->OptionalHeader;

        properties->ImageBase = (long)OptionalHeader->ImageBase;
        properties->SizeOfImage = OptionalHeader->SizeOfImage;
        properties->EntryPoint = (long)OptionalHeader->AddressOfEntryPoint;
    } else {
        PIMAGE_OPTIONAL_HEADER64 OptionalHeader = (PIMAGE_OPTIONAL_HEADER64)&PvMappedImage.NtHeaders->OptionalHeader;

        properties->ImageBase = (long)OptionalHeader->ImageBase;
        properties->SizeOfImage = OptionalHeader->SizeOfImage;
        properties->EntryPoint = (long)OptionalHeader->AddressOfEntryPoint;

    }

    properties->Subsystem = PvMappedImage.NtHeaders->OptionalHeader.Subsystem;
    properties->SubsystemVersion = new pair<short, short>(
        PvMappedImage.NtHeaders->OptionalHeader.MajorSubsystemVersion,
        PvMappedImage.NtHeaders->OptionalHeader.MinorSubsystemVersion);
    properties->Characteristics = PvMappedImage.NtHeaders->FileHeader.Characteristics;
    properties->DllCharacteristics = PvMappedImage.NtHeaders->OptionalHeader.DllCharacteristics;

    properties->FileSize = PvMappedImage.Size;
    return true;
}

bool PEManager::IsWow64Dll() {
    return ((properties->Machine & 0xffff) == IMAGE_FILE_MACHINE_I386);
}

unique_ptr<ApiSetSchemaBase> PEManager::GetApiSetSchema()
{
    PH_MAPPED_IMAGE mappedImage = m_Impl->m_PvMappedImage;
    for (auto n = 0u; n < mappedImage.NumberOfSections; ++n)
    {
        IMAGE_SECTION_HEADER const& section = mappedImage.Sections[n];
        if (strncmp(".apiset", reinterpret_cast<char const*>(section.Name), IMAGE_SIZEOF_SHORT_NAME) == 0)
            return ApiSetSchemaImpl::ParseApiSetSchema(reinterpret_cast<PAPI_SET_NAMESPACE_UNION>(PTR_ADD_OFFSET(mappedImage.ViewBase, section.PointerToRawData)));
    }
    return unique_ptr<ApiSetSchemaBase>(new EmptyApiSetSchema());
}

std::vector<PeImportDll> PEManager::GetImports() {
    if (m_ImportsInit)
        return m_Imports;

    m_ImportsInit = true;

    if (!loadSuccessful)
        return m_Imports;

    // Standard Imports
    if (NT_SUCCESS(PhGetMappedImageImports(&m_Impl->m_PvImports, &m_Impl->m_PvMappedImage))) {
        for (size_t IndexDll = 0; IndexDll < m_Impl->m_PvImports.NumberOfDlls; IndexDll++) {
            m_Imports.push_back(PeImportDll(&m_Impl->m_PvImports, IndexDll));
        }
    }

    // Delayed Imports
    if (NT_SUCCESS(PhGetMappedImageDelayImports(&m_Impl->m_PvDelayImports, &m_Impl->m_PvMappedImage))) {
        for (size_t IndexDll = 0; IndexDll < m_Impl->m_PvDelayImports.NumberOfDlls; IndexDll++) {
            m_Imports.push_back(PeImportDll(&m_Impl->m_PvDelayImports, IndexDll));
        }
    }

    return m_Imports;
}

wstring PEManager::GetManifest() {
    if (!loadSuccessful)
        return L"";

    // Extract embedded manifest
    INT  rawManifestLen;
    BYTE* rawManifest;
    if (!m_Impl->GetPeManifest(&rawManifest, &rawManifestLen))
        return L"";

    // TODO(unknown): Need to validate if this manual conversion works fine
    // Converting to wchar* and passing it to a C++ wstring object
    std::wstring manifest;
    for (int i = 0; i < rawManifestLen; i++) {
        manifest += rawManifest[i];
    }

    return manifest;
}
