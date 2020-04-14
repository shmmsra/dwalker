#include <string>

#include <BinaryCache.h>

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

    // TODO(unknown): Check and return if file already loaded in cache

    // Get file hash
    string PeHash = GetBinaryHash(PePath);

    // A sync lock is mandatory here in order not to load twice the
    // same binary from two differents workers
    // TODO(unknown): Have a file lock
    {
        PEManager* NewShadowBinary = new PEManager(PePath);
        NewShadowBinary->Load();

        // TODO(unknown): Add file in the cache
    }

    // TODO(unknown): Read file from the cache as it should now be present
    PEManager* ShadowBinary = BinaryDatabase[PeHash];

    // TODO(unknown): Convert any paths to an absolute one

    return ShadowBinary;
}
