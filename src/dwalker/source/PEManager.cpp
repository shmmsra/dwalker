#include <PEManager.h>

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
