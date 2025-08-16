#include <memory>
#include <filesystem>

#include <BinaryCache.hpp>
#include <SXSManifest.hpp>

using namespace std;

unique_ptr<ApiSetSchemaBase> BinaryCache::ApiSetmapCache = PHLib::GetInstance()->GetApiSetSchema();

string BinaryCache::GetBinaryHash(const wstring& PePath) {
    // Convert to absolute path and normalize
    try {
        auto absolutePath = filesystem::absolute(PePath);
        wstring normalizedPath = absolutePath.wstring();
        
        // Convert to lowercase for case-insensitive hashing on Windows
        transform(normalizedPath.begin(), normalizedPath.end(), normalizedPath.begin(), ::towlower);
        
        // Simple but better hash function (FNV-1a style)
        size_t hash = 2166136261u; // FNV offset basis
        for (wchar_t c : normalizedPath) {
            hash ^= static_cast<size_t>(c);
            hash *= 16777619u; // FNV prime
        }
        
        return to_string(hash);
    } catch (...) {
        // Fallback for invalid paths
        size_t hash = std::hash<wstring>{}(PePath);
        return to_string(hash);
    }
}

void BinaryCache::UpdateLRU(const string& PeHash) {
    // TODO(unknown): To be implemented
}

PEManager* BinaryCache::GetBinary(const wstring& PePath) {
    // Check if file exists and is accessible
    if (!filesystem::exists(PePath)) {
        return nullptr;
    }
    
    // Check if it's a regular file (not a directory or special file)
    if (!filesystem::is_regular_file(PePath)) {
        return nullptr;
    }

    // Get file hash
    string PeHash = GetBinaryHash(PePath);

    // Check and return if file already loaded in cache
    if (BinaryDatabase.find(PeHash) != BinaryDatabase.end()) {
        return BinaryDatabase[PeHash];
    }

    PEManager* ShadowBinary = nullptr;

    // A sync lock would be mandatory here for thread safety
    // For now, we'll keep it simple for single-threaded usage
    try {
        PEManager* NewShadowBinary = new PEManager(PePath);
        if (NewShadowBinary && NewShadowBinary->Load()) {
            // Add file in the cache
            BinaryDatabase[PeHash] = NewShadowBinary;
            ShadowBinary = NewShadowBinary;
        } else {
            // Clean up if loading failed
            delete NewShadowBinary;
        }
    } catch (...) {
        // Handle any exceptions during PE loading
        return nullptr;
    }

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
