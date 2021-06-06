#pragma once

#include <PEManager.hpp>
#include <BinaryCache.hpp>

class DWalker {
private:
    BinaryCache* binaryCache;

public:
    DWalker();
    ~DWalker();

    bool DumpDependencyChain(const std::wstring& filePath);
};
