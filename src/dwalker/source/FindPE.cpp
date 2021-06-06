#include <filesystem>
//#include <shlwapi.h>
#include <ShlObj.h>
#include <Knownfolders.h>

#include <FindPE.hpp>

using namespace std;

bool FindPE::IsFilepathInvalid(wstring Filepath) {
    // TODO(unknown): To be implemented
    return false;
}

wstring FindPE::FindPEFromPath(wstring ModuleName, vector<wstring> CandidateFolders, bool Wow64Dll) {
    for (auto path : CandidateFolders) {
        if (!IsFilepathInvalid(path)) {
            wstring PeFilePath = filesystem::path(path) / ModuleName;
            if (filesystem::exists(PeFilePath)) {
                // TODO(unknown): Need to check if the file exists (and is actually loadable) prior to returning
                return PeFilePath;
            }
        }
    }

    return L"";
}

/*
public static Tuple<ModuleSearchStrategy, string> FindPeFromDefault(PE RootPe, string ModuleName) {
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
 * Default search order:
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
pair<ModuleSearchStrategy, wstring> FindPE::FindPeFromDefault(
    PEManager* RootPe,
    wstring ModuleName,
    SxsEntries SxsCache,
    vector<wstring> CustomSearchFolders
) {
    wstring RootPeFolder(filesystem::path(RootPe->filepath).parent_path());
    if (!filesystem::exists(RootPeFolder)) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::NOT_FOUND,
            L""
        );
    }

    bool Wow64Dll = RootPe->IsWow64Dll();
    wstring FoundPePath;

    filesystem::path WindowsSystemFolderPath;
    PWSTR pszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(Wow64Dll ? FOLDERID_SystemX86 : FOLDERID_System, NULL, NULL, &pszPath))) {
        WindowsSystemFolderPath = pszPath;
    }
    CoTaskMemFree(pszPath);

    filesystem::path WindowsFolderPath;
    pszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Windows, NULL, NULL, &pszPath))) {
        WindowsFolderPath = pszPath;
    }
    CoTaskMemFree(pszPath);


    // -1. Look in Sxs manifest (copious reversing needed)
    // TODO : find dll search order
    /*
    if (SxsCache.Count != 0) {
        SxsEntry Entry = SxsCache.Find(SxsItem = >
            string.Equals(SxsItem.Name, ModuleName, StringComparison.OrdinalIgnoreCase)
        );

        if (Entry != null) {
            return new Tuple<ModuleSearchStrategy, string>(ModuleSearchStrategy.SxS, Entry.Path);
        }
    }
    */


    // 0. Look in well-known dlls list
    // HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\KnownDLLs
    // https://blogs.msdn.microsoft.com/larryosterman/2004/07/19/what-are-known-dlls-anyway/
    auto KnownDlls = PHLib::GetInstance()->GetKnownDlls(Wow64Dll);
    auto KnownDllsIterator = KnownDlls.find(ModuleName);
    if (KnownDllsIterator != KnownDlls.end()) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::WellKnownDlls,
            (WindowsSystemFolderPath / *KnownDllsIterator)
        );
    }


    // 1. Look in application folder
    FoundPePath = FindPEFromPath(ModuleName, { RootPeFolder }, Wow64Dll);
    if (!FoundPePath.empty()) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::ApplicationDirectory,
            FoundPePath
        );
    }


    // {2-3-4}. Look in system folders
    vector<wstring> SystemFolders = {
        WindowsSystemFolderPath,
        WindowsFolderPath
    };
    FoundPePath = FindPEFromPath(ModuleName, SystemFolders, Wow64Dll);
    if (!FoundPePath.empty()) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::WindowsFolder,
            FoundPePath
        );
    }


    // 5. Look in current directory
    // Ignored for the time being since we can't know from
    // where the exe is run
    // TODO : Add a user supplied path emulating %cwd%
    FoundPePath = FindPEFromPath(ModuleName, { filesystem::current_path() }, Wow64Dll);
    if (!FoundPePath.empty()) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::WorkingDirectory,
            FoundPePath
        );
    }


    // 6. Look in local app data (check for python for example)
    // TODO(unknown): To be implemented


    /*
    // 7. Find in PATH
    string PATH = Environment.GetEnvironmentVariable("PATH");
    List<String> PATHFolders = new List<string>(PATH.Split(';'));
    // Filter out empty paths, since it resolve to the current working directory
    // fix https://github.com/lucasg/Dependencies/issues/51
    PATHFolders = PATHFolders.Where(path = > path.Length != 0).ToList();
    FoundPePath = FindPEFromPath(ModuleName, PATHFolders, Wow64Dll);
    if (!FoundPePath.empty()) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::Environment,
            FoundPePath
        );
    }
    */


    // 8. Check if it's an absolute import
    if (!ModuleName.empty() && filesystem::path(ModuleName).has_root_path() && filesystem::exists(ModuleName)) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::Fullpath,
            ModuleName
        );
    }


    // 0xff. Allow the user to supply custom search folders, to take into account
    // specific cases.
    FoundPePath = FindPEFromPath(ModuleName, CustomSearchFolders, Wow64Dll);
    if (!FoundPePath.empty()) {
        return pair<ModuleSearchStrategy, wstring>(
            ModuleSearchStrategy::UserDefined,
            FoundPePath
        );
    }


    return pair<ModuleSearchStrategy, wstring>(
        ModuleSearchStrategy::NOT_FOUND,
        L""
    );
}
