#pragma once

#include <string>
#include <vector>

#include <PE.h>

using namespace std;

struct PeImport {
    unsigned short Hint;
    unsigned short Ordinal;
    string Name;
    string ModuleName;
    bool ImportByOrdinal;
    bool DelayImport;

    PeImport(const PPH_MAPPED_IMAGE_IMPORT_DLL importDll, size_t Index);
    PeImport(const PeImport& other);
    ~PeImport();
};

struct PeImportDll {
public:
    long Flags;
    string Name;
    long NumberOfEntries;
    vector<PeImport> ImportList;

    // constructors
    PeImportDll(const PPH_MAPPED_IMAGE_IMPORTS& PvMappedImports, size_t ImportDllIndex);
    PeImportDll(const PeImportDll& other);

    // destructors
    ~PeImportDll();

    // getters
    bool IsDelayLoad();

private:
    PPH_MAPPED_IMAGE_IMPORT_DLL ImportDll;
};

struct PeProperties {
    short Machine;
    // DateTime^ Time;
    short Magic;

    long ImageBase;
    int  SizeOfImage;
    long EntryPoint;


    int Checksum;
    bool CorrectChecksum;

    short Subsystem;
    pair<short, short>* SubsystemVersion;

    short Characteristics;
    short DllCharacteristics;

    unsigned long FileSize;
};

class PEManager {
public:
    PEManager(const wstring& filepath);
    ~PEManager();

    // Mapped the PE in memory and init infos
    bool Load();

    // Unmapped the PE from memory
    void Unload();

    // Check if the PE is 32-bit
    bool IsWow64Dll();

    // Return the ApiSetSchema
    // ApiSetSchema^ GetApiSetSchema();

    // Return the list of functions exported by the PE
    // List<PeExport ^>^ GetExports();

    // Return the list of functions imported by the PE, bundled by Dll name
    vector<PeImportDll> GetImports();

    // Retrieve the manifest embedded within the PE
    // Return an empty string if there is none.
    // String^ GetManifest();

    // PE properties parsed from the NT header
    PeProperties* properties;

    // Check if the specified file has been successfully parsed as a PE file.
    bool loadSuccessful;

    // Path to PE file.
    wstring filepath;

protected:
    // Initalize PeProperties struct once the PE has been loaded into memory
    bool InitProperties();

private:
    // C++ part interfacing with phlib
    PE* m_Impl;

    // local cache for imports and exports list
    vector<PeImportDll> m_Imports;
    // List<PeExport ^>^  m_Exports;
    bool m_ExportsInit;
    bool m_ImportsInit;
};
