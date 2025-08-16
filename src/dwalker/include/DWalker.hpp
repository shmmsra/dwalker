#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <PEManager.hpp>
#include <BinaryCache.hpp>

class DWalker {
private:
    BinaryCache* binaryCache;
    std::set<std::wstring> processedFiles;
    int indentLevel;
    std::wostream* outputStream;
    
    void PrintIndent();
    void PrintModuleInfo(PEManager* peManager, ModuleSearchStrategy strategy);
    bool DumpDependencyChainInternal(const std::wstring& filePath, int depth = 0);

public:
    DWalker(std::wostream& output = std::wcout);
    ~DWalker();

    bool DumpDependencyChain(const std::wstring& filePath);
    void SetMaxDepth(int maxDepth) { this->maxDepth = maxDepth; }
    void SetVerbose(bool verbose) { this->verbose = verbose; }
    
private:
    int maxDepth;
    bool verbose;
};
