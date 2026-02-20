#include <DWalker.hpp>
#include <PEManager.hpp>
#include <BinaryCache.hpp>

using namespace std;

DWalker::DWalker() : binaryCache(NULL) {
    this->binaryCache = new BinaryCache();
}

DWalker::~DWalker() {
    if (binaryCache) {
        delete binaryCache;
        binaryCache = NULL;
    }
}

/*
 * Recursively extract dependencies of the given file at filePath
 */
bool DWalker::DumpDependencyChain(const wstring& filePath) {
    // Load the current user provided binary and save it in the cache as well
    PEManager* peManager = binaryCache->GetBinary(filePath);
    if (!peManager) {
        return false;
    }

    const bool is32Bit = peManager->IsWow64Dll();

    // Get the list of dependencies for the binary
    // we would then loop over these dependencies recursively to further resolve their dependencies
    vector<PeImportDll> imports = peManager->GetImports();

    for (auto& x : imports) {
        std::pair<ModuleSearchStrategy, PEManager*> t = binaryCache->ResolveModule(peManager, wstring(x.Name.begin(), x.Name.end()));
        // Recursively load all the dependencies
        if (t.first == ModuleSearchStrategy::NOT_FOUND || !t.second || !DumpDependencyChain(t.second->filepath)) {
            return false;
        }
    }

    return true;
}

string EscapeJsonString(const string& input) {
    string output = "";
    for (char c : input) {
        if (c == '"') output += "\\\"";
        else if (c == '\\') output += "\\\\";
        else if (c == '\b') output += "\\b";
        else if (c == '\f') output += "\\f";
        else if (c == '\n') output += "\\n";
        else if (c == '\r') output += "\\r";
        else if (c == '\t') output += "\\t";
        else output += c;
    }
    return output;
}

string DWalker::DumpDependencyChainJsonRecursive(const wstring& filePath, std::set<wstring>& visited) {
    string name(filePath.begin(), filePath.end()); // Simple conversion for ASCII paths
    string json = "{";
    json += "\"name\": \"" + EscapeJsonString(name) + "\"";
    
    if (visited.find(filePath) != visited.end()) {
        json += "}";
        return json;
    }
    visited.insert(filePath);

    PEManager* peManager = binaryCache->GetBinary(filePath);
    if (!peManager) {
        json += "}";
        return json;
    }

    vector<PeImportDll> imports = peManager->GetImports();
    if (!imports.empty()) {
        json += ", \"dependencies\": [";
        bool first = true;
        for (auto& x : imports) {
            std::pair<ModuleSearchStrategy, PEManager*> t = binaryCache->ResolveModule(peManager, wstring(x.Name.begin(), x.Name.end()));
            if (t.second) {
                if (!first) json += ", ";
                json += DumpDependencyChainJsonRecursive(t.second->filepath, visited);
                first = false;
            } else {
                if (!first) json += ", ";
                json += "{\"name\": \"" + EscapeJsonString(x.Name) + "\", \"error\": \"not found\"}";
                first = false;
            }
        }
        json += "]";
    }
    json += "}";
    return json;
}

string DWalker::DumpDependencyChainJson(const wstring& filePath) {
    std::set<wstring> visited;
    return DumpDependencyChainJsonRecursive(filePath, visited);
}

string DWalker::DumpDependencyChainTextRecursive(const wstring& filePath, std::set<wstring>& visited, int depth) {
    string name(filePath.begin(), filePath.end());
    string indent(depth * 2, ' ');
    string text = indent + name + "\n";
    
    if (visited.find(filePath) != visited.end()) {
        return text;
    }
    visited.insert(filePath);

    PEManager* peManager = binaryCache->GetBinary(filePath);
    if (!peManager) {
        return text;
    }

    vector<PeImportDll> imports = peManager->GetImports();
    for (auto& x : imports) {
        std::pair<ModuleSearchStrategy, PEManager*> t = binaryCache->ResolveModule(peManager, wstring(x.Name.begin(), x.Name.end()));
        if (t.second) {
            text += DumpDependencyChainTextRecursive(t.second->filepath, visited, depth + 1);
        } else {
            text += indent + "  " + x.Name + " (NOT FOUND)\n";
        }
    }
    return text;
}

string DWalker::DumpDependencyChainText(const wstring& filePath) {
    std::set<wstring> visited;
    return DumpDependencyChainTextRecursive(filePath, visited, 0);
}
