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
#include <xercesc/dom/DOMDocument.hpp>

#include <PEManager.hpp>

class SxsEntry {
public:
    std::wstring Name;
    std::wstring Path;
    std::wstring Version;
    std::wstring Type;
    std::wstring PublicKeyToken;

    SxsEntry(std::wstring _Name, std::wstring _Path, std::wstring _Version = L"", std::wstring _Type = L"", std::wstring _PublicKeyToken = L"");
    SxsEntry(const SxsEntry& OtherSxsEntry);
    SxsEntry(xercesc::DOMDocument* doc, const std::wstring basePath, xercesc::DOMXPathNSResolver* resolver, std::wstring relPath, std::wstring Folder);
};

class SxsEntries : public std::vector<SxsEntry> {
public:
    static SxsEntries FromSxsAssembly(xercesc::DOMDocument* doc, const std::wstring basePath, xercesc::DOMXPathNSResolver* resolver, std::wstring Folder);
};

class SxsManifest {
private:
    static SxsManifest* _instance;
    SxsManifest();

public:
    static SxsManifest* GetInstance();

    // find dll with same name as sxs assembly in target directory
    SxsEntries SxsFindTargetDll(std::wstring AssemblyName, std::wstring Folder);

    SxsEntries ExtractDependenciesFromSxsElement(xercesc::DOMDocument* doc, const std::wstring& basePath, xercesc::DOMXPathNSResolver* resolver, std::wstring Folder, std::wstring ExecutableName = L"", bool Wow64Pe = false);
    SxsEntries ExtractDependenciesFromSxsManifestFile(std::wstring ManifestFile, std::wstring Folder, std::wstring ExecutableName = L"", bool Wow64Pe = false);
    SxsEntries ExtractDependenciesFromSxsManifest(std::string Manifest, std::wstring Folder, std::wstring ExecutableName = L"", bool Wow64Pe = false);
/*
    static xml_document ParseSxsManifest(System.IO.Stream ManifestStream);
*/

    SxsEntries GetSxsEntries(PEManager* Pe);
};
