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

using namespace xercesc;

class SxsEntry {
public:
    std::wstring Name;
    std::wstring Path;
    std::wstring Version;
    std::wstring Type;
    std::wstring PublicKeyToken;

    SxsEntry(std::wstring _Name, std::wstring _Path, std::wstring _Version = L"", std::wstring _Type = L"", std::wstring _PublicKeyToken = L"");
    SxsEntry(const SxsEntry& OtherSxsEntry);
    SxsEntry(DOMDocument* doc, const std::wstring basePath, DOMXPathNSResolver* resolver, std::wstring relPath, std::wstring Folder);
};

class SxsEntries : public std::vector<SxsEntry> {
public:
    static SxsEntries FromSxsAssembly(DOMDocument* doc, const std::wstring basePath, DOMXPathNSResolver* resolver, std::wstring Folder);
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
    SxsEntries ExtractDependenciesFromSxsManifestFile(std::wstring ManifestFile, std::wstring Folder, std::wstring ExecutableName = L"", bool Wow64Pe = false);

/*
    static SxsEntries ExtractDependenciesFromSxsManifest(wstring ManifestStream, wstring Folder, wstring ExecutableName = L"", bool Wow64Pe = false);
    static xml_document ParseSxsManifest(System.IO.Stream ManifestStream);
*/

    SxsEntries GetSxsEntries(PEManager* Pe);
};
