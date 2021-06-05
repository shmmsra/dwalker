#pragma once

#define _HAS_STD_BYTE   0

#include <string>
#include <vector>

#include <PHLib.h>
#include <PE.h>
#include <PEManager.h>
#include <SXSManifest.h>

enum ModuleSearchStrategy {
    ROOT = -1,

    SxS = 0,
    ApiSetSchema = 1,
    WellKnownDlls = 2,
    ApplicationDirectory = 3,
    System32Folder = 4,
    WindowsFolder = 5,
    WorkingDirectory = 6,
    Environment = 7,
    AppInitDLL = 8,
    Fullpath = 9,
    ClrAssembly = 10,

    UserDefined = 0xfe,
    NOT_FOUND = 0xff
};

/// <summary>
/// Dll path resolver emulator for the NT Loader.
/// </summary>
class FindPE {
public:
    static bool IsFilepathInvalid(wstring Filepath);
    static wstring FindPEFromPath(wstring ModuleName, vector<wstring> CandidateFolders, bool Wow64Dll = false);

    /*
    static Tuple<ModuleSearchStrategy, string> FindPeFromDefault(PE RootPe, string ModuleName) {
        string WorkingDirectory = Path.GetDirectoryName(RootPe.Filepath);
        List<string> CustomSearchFolders = new List<string>();
        SxsEntries SxsCache = SxsManifest.GetSxsEntries(RootPe);

        return FindPeFromDefault(
            RootPe,
            ModuleName,
            SxsCache,
            CustomSearchFolders,
            WorkingDirectory
        );
    }
    */

    /*
     * Default search order :
     * https://msdn.microsoft.com/en-us/library/windows/desktop/ms682586(v=vs.85).aspx
     *
     * -1. Sxs manifests
     *  0. KnownDlls list
     *  1. Loaded PE folder
     *  2. C:\Windows\(System32 | SysWow64 )
     *  3. 16-bit system directory   <-- ignored
     *  4. C:\Windows
     *  5. %pwd%
     *  6. AppDatas
     */
    static pair<ModuleSearchStrategy, wstring> FindPeFromDefault(
        PEManager* RootPe,
        wstring ModuleName,
        SxsEntries SxsCache,
        vector<wstring> CustomSearchFolders);
};
