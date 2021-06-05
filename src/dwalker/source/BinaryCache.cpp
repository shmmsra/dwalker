#include <memory>
#include <filesystem>

#include <BinaryCache.h>
#include <SXSManifest.h>
#include <FindPE.h>

using namespace std;

unique_ptr<ApiSetSchemaBase> BinaryCache::ApiSetmapCache = PHLib::GetInstance()->GetApiSetSchema();

string BinaryCache::GetBinaryHash(const wstring& PePath) {
    // TODO(unknown): Fix the hash calculation logic
    unsigned int hash = 0;
    for (unsigned short i = 0; i < PePath.length(); i++) {
        hash += PePath[i];
    }
    return to_string(hash);
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

/// <summary>
/// Attempt to query the HostDll pointed by the Apiset contract.
/// </summary>
/// <param name="ImportDllName"> DLL name as in the parent import entry. May or may not be an apiset contract </param>
/// <returns> Return the first host dll pointed by the apiset contract if found, otherwise it return an empty string.</returns>
wstring BinaryCache::LookupApiSetLibrary(wstring ImportDllName)
{
    auto lower_name = ImportDllName;
    transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

    // Look for api set target 
    if (lower_name.rfind(L"api-", 0) != 0 && lower_name.rfind(L"ext-", 0) != 0)
        return L"";

    // Strip the .dll extension and search for matching targets
    auto ImportDllWIthoutExtension = filesystem::path(ImportDllName).replace_extension();
    if (BinaryCache::ApiSetmapCache != nullptr) {
        auto Targets = BinaryCache::ApiSetmapCache->Lookup(ImportDllWIthoutExtension);
        if (Targets.size() > 0)
            return Targets[0];
    }

    return L"";
}

/*
pair<ModuleSearchStrategy, PEManager*> BinaryCache::ResolveModule(string ModuleName) {
    PE RootPe = LoadPe(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "ntdll.dll"));
    string WorkingDirectory = Path.GetDirectoryName(RootPe.Filepath);
    List<string> CustomSearchFolders = new List<string>();
    SxsEntries SxsCache = SxsManifest.GetSxsEntries(RootPe);

    return ResolveModule(RootPe, ModuleName, SxsCache, CustomSearchFolders, WorkingDirectory);
}
*/

pair<ModuleSearchStrategy, PEManager*> BinaryCache::ResolveModule(PEManager* RootPe, wstring ModuleName) {
    // TODO: Get the path of ntdll.dll if RootPe is not available and set that as the current directory
    wstring WorkingDirectory = RootPe->filepath;
    vector<wstring> CustomSearchFolders;
    SxsEntries SxsCache = SxsManifest::GetInstance()->GetSxsEntries(RootPe);

    return ResolveModule(RootPe, ModuleName, SxsCache, CustomSearchFolders, WorkingDirectory);
}

pair<ModuleSearchStrategy, PEManager*> BinaryCache::ResolveModule(
    PEManager* RootPe,
    wstring ModuleName,
    SxsEntries SxsCache,
    vector<wstring> CustomSearchFolders,
    wstring WorkingDirectory
) {
    pair<ModuleSearchStrategy, wstring> ResolvedFilepath;

    // if no extension is used, assume a .dll
    if (!filesystem::path(ModuleName).has_extension()) {
        ModuleName += L".dll";
    }

    wstring ApiSetName = LookupApiSetLibrary(ModuleName);
    if (!ApiSetName.empty()) {
        ModuleName = ApiSetName;
    }

    ResolvedFilepath = FindPE::FindPeFromDefault(RootPe, ModuleName, SxsCache, CustomSearchFolders);

    // ApiSet override the underneath search location if found or not
    ModuleSearchStrategy ModuleLocation = ResolvedFilepath.first;
    if (!ApiSetName.empty())
        ModuleLocation = ModuleSearchStrategy::ApiSetSchema;

    PEManager* ResolvedModule = NULL;
    if (!ResolvedFilepath.second.empty())
        ResolvedModule = GetBinary(ResolvedFilepath.second);

    return pair<ModuleSearchStrategy, PEManager*>(ResolvedFilepath.first, ResolvedModule);
}
