#include <BinaryCache.h>

#include <filesystem>

string BinaryCache::GetBinaryHash(const wstring& PePath) {
    // TODO(unknown): Fix the hash calculation logic
    unsigned int hash = 0;
    for (unsigned short i = 0; i < PePath.length(); i++) {
        hash += PePath[i];
    }
    return std::to_string(hash);
}

void BinaryCache::UpdateLRU(const string& PeHash) {
    // TODO(unknown): To be implemented
}

PEManager* BinaryCache::GetBinary(const wstring& PePath) {
    // TODO(unknown): Check if file exists

    // Get file hash
    string PeHash = GetBinaryHash(PePath);

    // Check and return if file already loaded in cache
    if (BinaryDatabase.find(PeHash) != BinaryDatabase.end()) {
        return BinaryDatabase[PeHash];
    }

    PEManager* ShadowBinary = NULL;

    // A sync lock is mandatory here in order not to load twice the
    // same binary from two differents workers
    // TODO(unknown): Have a file lock
    {
        PEManager* NewShadowBinary = new PEManager(PePath);
        if (NewShadowBinary->Load()) {
            // Add file in the cache
            BinaryDatabase[PeHash] = NewShadowBinary;
            ShadowBinary = NewShadowBinary;
        }
    }

    // TODO(unknown): Convert any paths to an absolute one

    return ShadowBinary;
}

/*
public static Tuple<ModuleSearchStrategy, PE> ResolveModule(string ModuleName) {
    PE RootPe = LoadPe(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "ntdll.dll"));
    string WorkingDirectory = Path.GetDirectoryName(RootPe.Filepath);
    List<string> CustomSearchFolders = new List<string>();
    SxsEntries SxsCache = SxsManifest.GetSxsEntries(RootPe);

    return ResolveModule(RootPe, ModuleName, SxsCache, CustomSearchFolders, WorkingDirectory);
}

public static Tuple<ModuleSearchStrategy, PE> ResolveModule(PE RootPe, string ModuleName) {
    string WorkingDirectory = Path.GetDirectoryName(RootPe.Filepath);
    List<string> CustomSearchFolders = new List<string>();
    SxsEntries SxsCache = SxsManifest.GetSxsEntries(RootPe);

    return ResolveModule(RootPe, ModuleName, SxsCache, CustomSearchFolders, WorkingDirectory);
}
*/

pair<ModuleSearchStrategy, PEManager*> BinaryCache::ResolveModule(
    PEManager* RootPe,
    wstring ModuleName,
    SxsEntries SxsCache,
    vector<wstring> CustomSearchFolders) {
    pair<ModuleSearchStrategy, wstring> ResolvedFilepath;

    // if no extension is used, assume a .dll
    if (!filesystem::path(ModuleName).has_extension()) {
        ModuleName += L".dll";
    }

    /*
    string ApiSetName = LookupApiSetLibrary(ModuleName);
    if (!string.IsNullOrEmpty(ApiSetName)) {
        ModuleName = ApiSetName;
    }
    */

    ResolvedFilepath = FindPE::FindPeFromDefault(RootPe, ModuleName, SxsCache, CustomSearchFolders);

    /*
    // ApiSet override the underneath search location if found or not
    ModuleSearchStrategy ModuleLocation = ResolvedFilepath.Item1;
    if (!string.IsNullOrEmpty(ApiSetName))
        ModuleLocation = ModuleSearchStrategy::ApiSetSchema;
    */

    PEManager* ResolvedModule = NULL;
    if (!ResolvedFilepath.second.empty())
        ResolvedModule = GetBinary(ResolvedFilepath.second);


    return pair<ModuleSearchStrategy, PEManager*>(ResolvedFilepath.first, ResolvedModule);
}
