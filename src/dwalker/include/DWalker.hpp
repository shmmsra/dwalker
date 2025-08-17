#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <PEManager.hpp>
#include <BinaryCache.hpp>
#include <FindPE.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DWalker {
private:
    BinaryCache* binaryCache;
    int indentLevel;
    std::wostream* outputStream;
    int maxDepth;
    bool verbose;
    std::set<std::wstring> processedFiles;
    json jsonOutput; // Store JSON object during construction

    bool DumpDependencyChainInternal(const std::wstring& filePath, int depth);
    void PrintIndent();
    void PrintModuleInfo(class PEManager* peManager, ModuleSearchStrategy strategy);
    
    // JSON output methods
    void OutputJsonHeader(const std::wstring& filename, const std::wstring& fullPath);
    void OutputJsonFooter();
    void OutputModuleJson(class PEManager* peManager, ModuleSearchStrategy strategy, int depth);
    void OutputCircularJson(const std::wstring& filename, int depth);
    bool DumpDependencyChainInternalJson(const std::wstring& filePath, int depth);

public:
    DWalker(std::wostream& output);
    ~DWalker();
    
    void SetVerbose(bool verbose) { this->verbose = verbose; }
    void SetMaxDepth(int depth) { this->maxDepth = depth; }
    
    bool DumpDependencyChain(const std::wstring& filePath);
    
    // New JSON output method
    bool DumpDependencyChainJson(const std::wstring& filePath);
};
