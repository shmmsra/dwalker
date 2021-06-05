#pragma once

#include <PEManager.h>
#include <BinaryCache.h>

class DWalker {
private:
    BinaryCache* binaryCache;

public:
    DWalker();
    ~DWalker();

    bool DumpDependencyChain(const std::wstring& filePath);
};
