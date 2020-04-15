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
