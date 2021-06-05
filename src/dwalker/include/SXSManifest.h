#pragma once

#define WIN32_LEAN_AND_MEAN 1
#ifndef __MSXML_LIBRARY_DEFINED__
#define __MSXML_LIBRARY_DEFINED__
#endif

#include <string>
#include <vector>
#include <filesystem>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <PEManager.h>

using namespace std;
using namespace xercesc;

class SxsEntry {
public:
    wstring Name;
    wstring Path;
    wstring Version;
    wstring Type;
    wstring PublicKeyToken;

    SxsEntry(wstring _Name, wstring _Path, wstring _Version = L"", wstring _Type = L"", wstring _PublicKeyToken = L"");
    SxsEntry(const SxsEntry& OtherSxsEntry);
    SxsEntry(DOMDocument* doc, const wstring basePath, DOMXPathNSResolver* resolver, wstring relPath, wstring Folder);
};

class SxsEntries : public vector<SxsEntry> {
public:
    static SxsEntries FromSxsAssembly(DOMDocument* doc, const wstring basePath, DOMXPathNSResolver* resolver, wstring Folder);
};

class SxsManifest {
private:
    static SxsManifest* _instance;
    SxsManifest();

public:
    static SxsManifest* GetInstance();

/*
    // find dll with same name as sxs assembly in target directory
    static SxsEntries SxsFindTargetDll(wstring AssemblyName, wstring Folder);
*/
//    SxsEntries ExtractDependenciesFromSxsElement(DOMDocument* doc, const wstring basePath, DOMXPathNSResolver* resolver, wstring Folder, wstring ExecutableName = L"", bool Wow64Pe = false);
    SxsEntries ExtractDependenciesFromSxsManifestFile(wstring ManifestFile, wstring Folder, wstring ExecutableName = L"", bool Wow64Pe = false);

/*
    static SxsEntries ExtractDependenciesFromSxsManifest(wstring ManifestStream, wstring Folder, wstring ExecutableName = L"", bool Wow64Pe = false);
    static xml_document ParseSxsManifest(System.IO.Stream ManifestStream);
*/

    SxsEntries GetSxsEntries(PEManager* Pe);
};
