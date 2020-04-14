#pragma once

#include <string>
#include <PE.h>

using namespace std;

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
    // List<PeImportDll ^>^ GetImports();

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
    // List<PeImportDll ^>^ m_Imports;
    // List<PeExport ^>^  m_Exports;
    bool m_ExportsInit;
    bool m_ImportsInit;
};
