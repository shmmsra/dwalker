#pragma once

#include <string>
#include <map>

#include <PEManager.h>

using namespace std;

class BinaryCache {
private:
    map<string, PEManager*> BinaryDatabase;

    string GetBinaryHash(const wstring& PePath);
    void UpdateLRU(const string& PeHash);

public:
    PEManager* GetBinary(const wstring& PePath);
};
