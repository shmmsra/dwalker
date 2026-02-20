#pragma once

#include <PEManager.hpp>
#include <BinaryCache.hpp>
#include <string>
#include <set>

class DWalker {
private:
    BinaryCache* binaryCache;

public:
    DWalker();
    ~DWalker();

    bool DumpDependencyChain(const std::wstring& filePath);
    std::string DumpDependencyChainJson(const std::wstring& filePath);
    std::string DumpDependencyChainText(const std::wstring& filePath);

private:
    std::string DumpDependencyChainJsonRecursive(const std::wstring& filePath, std::set<std::wstring>& visited);
    std::string DumpDependencyChainTextRecursive(const std::wstring& filePath, std::set<std::wstring>& visited, int depth);
};
