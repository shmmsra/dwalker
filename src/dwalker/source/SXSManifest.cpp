#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <streambuf>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <PEManager.hpp>
#include <SXSManifest.hpp>

using namespace std;
using namespace xercesc;

std::wstring GetXMLNodeAttribute(const DOMXPathResult* node, const string& attributeName) {
    if (node == nullptr)
        return L"";

    DOMNode* nodeValue = node->getNodeValue();
    if (nodeValue == nullptr)
        return L"";

    DOMNamedNodeMap* nodeAttributes = nodeValue->getAttributes();
    if (nodeAttributes == nullptr)
        return L"";

    DOMNode* attribute = nodeAttributes->getNamedItem(XMLString::transcode(attributeName.c_str()));
    if (attribute == nullptr)
        return L"";

    return attribute->getNodeValue();
}

SxsEntry::SxsEntry(wstring _Name, wstring _Path, wstring _Version/* = L""*/, wstring _Type/* = L""*/, wstring _PublicKeyToken/* = L""*/) {
    Name = _Name;
    Path = _Path;
    Version = _Version;
    Type = _Type;
    PublicKeyToken = _PublicKeyToken;
}

SxsEntry::SxsEntry(const SxsEntry& OtherSxsEntry) {
    Name = OtherSxsEntry.Name;
    Path = OtherSxsEntry.Path;
    Version = OtherSxsEntry.Version;
    Type = OtherSxsEntry.Type;
    PublicKeyToken = OtherSxsEntry.PublicKeyToken;
}

SxsEntry::SxsEntry(DOMDocument* doc, const wstring basePath, DOMXPathNSResolver* resolver, wstring relPath, wstring Folder) {
    Name = filesystem::path(relPath).filename();
    Path = filesystem::path(Folder);
    Path += filesystem::path(relPath);

    DOMXPathResult* sxsAssemblyIdentityNode = doc->evaluate(
        basePath.c_str(),
        doc->getDocumentElement(),
        resolver,
        DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
        NULL);
    if (sxsAssemblyIdentityNode != nullptr) {
        XMLSize_t nLength = sxsAssemblyIdentityNode->getSnapshotLength();
        if (nLength == 1) {
            sxsAssemblyIdentityNode->snapshotItem(0);
            Version = GetXMLNodeAttribute(sxsAssemblyIdentityNode, "version");
            Type = GetXMLNodeAttribute(sxsAssemblyIdentityNode, "type");
            PublicKeyToken = GetXMLNodeAttribute(sxsAssemblyIdentityNode, "publicKeyToken");

            // TODO : DLL search order ?
            //if (!File.Exists(Path))
            //{
            //    Path = "???";
            //}
        }
    }
}

SxsEntries SxsEntries::FromSxsAssembly(DOMDocument* doc, const wstring basePath, DOMXPathNSResolver* resolver, wstring Folder) {
    SxsEntries Entries;

    DOMXPathResult* fileNodes = doc->evaluate(
        wstring(basePath + L"/file").c_str(),
        doc->getDocumentElement(),
        resolver,
        DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
        NULL);

    const wstring assemblyIdentityPathStr = basePath + L"/assemblyIdentity";
    DOMXPathResult* assemblyIdentityNodes = doc->evaluate(
        assemblyIdentityPathStr.c_str(),
        doc->getDocumentElement(),
        resolver,
        DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
        NULL);

    XMLSize_t nLength = fileNodes->getSnapshotLength();
    for (XMLSize_t i = 0; i < nLength; i++) {
        fileNodes->snapshotItem(i);
        wstring filename = fileNodes->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("name"))->getNodeValue();
        Entries.push_back(SxsEntry(
            doc,
            assemblyIdentityPathStr,
            resolver,
            filename,
            Folder
        ));
    }

    return Entries;
}

// find dll with same name as sxs assembly in target directory
SxsEntries SxsManifest::SxsFindTargetDll(wstring AssemblyName, wstring Folder) {
    SxsEntries EntriesFromElement;

    wstring TargetFilePath = filesystem::path(Folder);
    TargetFilePath += filesystem::path(AssemblyName);
    if (filesystem::exists(TargetFilePath)) {
        wstring Name = filesystem::path(TargetFilePath).filename();
        wstring Path = TargetFilePath;

        EntriesFromElement.push_back(SxsEntry(Name, Path));
        return EntriesFromElement;
    }

    wstring TargetDllPath = filesystem::path(Folder);
    TargetDllPath += filesystem::path(AssemblyName + L".dll");
    if (filesystem::exists(TargetDllPath)) {
        wstring Name = filesystem::path(TargetDllPath).filename();
        wstring Path = TargetDllPath;

        EntriesFromElement.push_back(SxsEntry(Name, Path));
        return EntriesFromElement;
    }

    return EntriesFromElement;
}

SxsEntries SxsManifest::ExtractDependenciesFromSxsElement(DOMDocument* doc, const wstring& basePath, DOMXPathNSResolver* resolver, wstring Folder, wstring ExecutableName/* = L""*/, bool Wow64Pe/* = false*/) {
    // Private assembly search sequence : https://msdn.microsoft.com/en-us/library/windows/desktop/aa374224(v=vs.85).aspx
    //
    // * In the application's folder. Typically, this is the folder containing the application's executable file.
    // * In a subfolder in the application's folder. The subfolder must have the same name as the assembly.
    // * In a language-specific subfolder in the application's folder.
    //      -> The name of the subfolder is a string of DHTML language codes indicating a language-culture or language.
    // * In a subfolder of a language-specific subfolder in the application's folder.
    //      -> The name of the higher subfolder is a string of DHTML language codes indicating a language-culture or language. The deeper subfolder has the same name as the assembly.
    //
    //
    // 0.   Side-by-side searches the WinSxS folder.
    // 1.   \\<appdir>\<assemblyname>.DLL
    // 2.   \\<appdir>\<assemblyname>.manifest
    // 3.   \\<appdir>\<assemblyname>\<assemblyname>.DLL
    // 4.   \\<appdir>\<assemblyname>\<assemblyname>.manifest

    DOMXPathResult* SxsAssembly = doc->evaluate(
        basePath.c_str(),
        doc->getDocumentElement(),
        resolver,
        DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
        NULL);

    wstring TargetSxsManifestPath;
    wstring SxsManifestName = SxsAssembly->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("name"))->getNodeValue();
    wstring SxsManifestDir = filesystem::path(Folder) / SxsManifestName;


    // 0. find publisher manifest in %WINDIR%/WinSxs/Manifest
    wstring PublicKeyToken = GetXMLNodeAttribute(SxsAssembly, "publicKeyToken");
    if (!PublicKeyToken.empty()) {
        wstring Name = GetXMLNodeAttribute(SxsAssembly, "name");
        transform(Name.begin(), Name.end(), Name.begin(), ::tolower);
        wstring ProcessArch = GetXMLNodeAttribute(SxsAssembly, "processorArchitecture");
        ProcessArch = !ProcessArch.empty() ? ProcessArch : L"*";
        wstring Version = GetXMLNodeAttribute(SxsAssembly, "version");
        wstring Language = GetXMLNodeAttribute(SxsAssembly, "langage");
        Language = !Language.empty() ? Language : L"none";  // TODO : support localized sxs redirection

        wstring lower_processarch = ProcessArch;
        transform(lower_processarch.begin(), lower_processarch.end(), lower_processarch.begin(), ::tolower);
        if (lower_processarch == L"$(build.arch)" || lower_processarch == L"*") {
            ProcessArch = (Wow64Pe) ? L"x86" : L"amd64";
        } else if (lower_processarch == L"amd64" || lower_processarch == L"x86" || lower_processarch == L"wow64" || lower_processarch == L"msil") {
            // nothing to do
        } else {
            ProcessArch = L".*";
        }
        // TODO: Search for the publisher manifest
    }

    // 1. \\<appdir>\<assemblyname>.DLL
    // find dll with same assembly name in same directory
    SxsEntries EntriesFromMatchingDll = SxsFindTargetDll(SxsManifestName, Folder);
    if (EntriesFromMatchingDll.size() > 0) {
        return EntriesFromMatchingDll;
    }


    // 2. \\<appdir>\<assemblyname>.manifest
    // find manifest with same assembly name in same directory
    TargetSxsManifestPath = filesystem::path(Folder);
    TargetSxsManifestPath += filesystem::path(SxsManifestName + L".manifest");
    if (filesystem::exists(TargetSxsManifestPath)) {
        return ExtractDependenciesFromSxsManifestFile(TargetSxsManifestPath, Folder, ExecutableName, Wow64Pe);
    }


    // 3. \\<appdir>\<assemblyname>\<assemblyname>.DLL
    // find matching dll in sub directory
    SxsEntries EntriesFromMatchingDllSub = SxsFindTargetDll(SxsManifestName, SxsManifestDir);
    if (EntriesFromMatchingDllSub.size() > 0) {
        return EntriesFromMatchingDllSub;
    }

    // 4. \<appdir>\<assemblyname>\<assemblyname>.manifest
    // find manifest in sub directory
    TargetSxsManifestPath = filesystem::path(SxsManifestDir);
    TargetSxsManifestPath += filesystem::path(SxsManifestName + L".manifest");
    if (filesystem::is_directory(SxsManifestDir) && filesystem::exists(TargetSxsManifestPath)) {
        return ExtractDependenciesFromSxsManifestFile(TargetSxsManifestPath, SxsManifestDir, ExecutableName, Wow64Pe);
    }

    // TODO : do the same thing for localization
    //
    // 0. Side-by-side searches the WinSxS folder.
    // 1. \\<appdir>\<language-culture>\<assemblyname>.DLL
    // 2. \\<appdir>\<language-culture>\<assemblyname>.manifest
    // 3. \\<appdir>\<language-culture>\<assemblyname>\<assemblyname>.DLL
    // 4. \\<appdir>\<language-culture>\<assemblyname>\<assemblyname>.manifest

    // TODO : also take into account Multilanguage User Interface (MUI) when
    // scanning manifests and WinSxs dll. God this is horrendously complicated.

    // Could not find the dependency
    {
        SxsEntries EntriesFromElement;
        EntriesFromElement.push_back(SxsEntry(SxsManifestName, L"file ???"));
        return EntriesFromElement;
    }

    return SxsEntries();
}

SxsManifest* SxsManifest::_instance = nullptr;

SxsManifest::SxsManifest() {
    try {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException & toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        XMLString::release(&message);
        // TODO(unknown): Need to return error
    }
}

SxsManifest* SxsManifest::GetInstance() {
    if (SxsManifest::_instance != nullptr) {
        return SxsManifest::_instance;
    }
    return new SxsManifest();
}

SxsEntries SxsManifest::ExtractDependenciesFromSxsManifest(string Manifest, wstring Folder, wstring ExecutableName/* = L""*/, bool Wow64Pe/* = false*/) {
    /* Sample manifest XML:
     *  <?xml version='1.0' encoding='UTF-8' standalone='yes'?>
     *  <assembly xmlns='urn:schemas-microsoft-com:asm.v1' manifestVersion='1.0'>
     *    <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
     *      <security>
     *        <requestedPrivileges>
     *          <requestedExecutionLevel level='asInvoker' uiAccess='false' />
     *        </requestedPrivileges>
     *      </security>
     *    </trustInfo>
     *    <dependency>
     *      <dependentAssembly>
     *        <assemblyIdentity type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8 processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' />
     *      </dependentAssembly>
     *    </dependency>
     *    <dependency>
     *      <dependentAssembly>
     *        <assemblyIdentity type='win32' name='Microsoft.VC90.MFC' version='9.0.21022.8 processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' />
     *      </dependentAssembly>
     *    </dependency>
     *    <dependency>
     *      <dependentAssembly>
     *        <assemblyIdentity type='win32' name='Microsoft.Windows.Common-Controls version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df language="'*'" />
     *      </dependentAssembly>
     *    </dependency>
     *  </assembly>
     */

    int retVal = 0;

    // TODO: Won't work for XML containing wide char
    MemBufInputSource xmlBuf((const XMLByte*)Manifest.c_str(), Manifest.size(), "sxs-manifest-xml");

    XercesDOMParser* parser = new XercesDOMParser();
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);    // optionalB

    ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
    parser->setErrorHandler(errHandler);

    SxsEntries AdditionalDependencies;

    try {
        parser->parse(xmlBuf);

        // get the DOM representation
        DOMDocument* doc = parser->getDocument();

        DOMElement* root = doc->getDocumentElement();
        try {
            DOMXPathNSResolver* resolver = doc->createNSResolver(root);

            // Find any declared dll
            //< assembly xmlns = 'urn:schemas-microsoft-com:asm.v1' manifestVersion = '1.0' >
            //    < assemblyIdentity name = 'additional_dll' version = 'XXX.YY.ZZ' type = 'win32' />
            //    < file name = 'additional_dll.dll' />
            //</ assembly >
            const wstring assemblyPathStr = L"/assembly";
            DOMXPathResult* result = doc->evaluate(
                assemblyPathStr.c_str(),
                root,
                resolver,
                DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                NULL);

            XMLSize_t nLength = result->getSnapshotLength();
            for (XMLSize_t i = 0; i < nLength; i++) {
                result->snapshotItem(i);
                SxsEntries t = SxsEntries::FromSxsAssembly(doc, assemblyPathStr, resolver, Folder);
                AdditionalDependencies.insert(AdditionalDependencies.end(), t.begin(), t.end());
            }

            result->release();

            // Find any dependencies :
            // <dependency>
            //     <dependentAssembly>
            //         <assemblyIdentity
            //             type="win32"
            //             name="Microsoft.Windows.Common-Controls"
            //             version="6.0.0.0"
            //             processorArchitecture="amd64" 
            //             publicKeyToken="6595b64144ccf1df"
            //             language="*"
            //         />
            //     </dependentAssembly>
            // </dependency>
            const wstring assemblyIdentityPathStr = L"/dependency/dependentAssembly/assemblyIdentity";
            // find target PE
            //SxsEntries t;// = ExtractDependenciesFromSxsElement(assemblyIdentityPathStr, Folder, ExecutableName, Wow64Pe);
            //AdditionalDependencies.insert(AdditionalDependencies.end(), t.begin(), t.end());

            resolver->release();
        }
        catch (const DOMXPathException & e) {
        }
        catch (const DOMException & e) {
        }
    }
    catch (const XMLException & toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        XMLString::release(&message);
        retVal = -1;
    }
    catch (const DOMException & toCatch) {
        char* message = XMLString::transcode(toCatch.msg);
        XMLString::release(&message);
        retVal = -1;
    }
    catch (...) {
        retVal = -1;
    }

    delete parser;
    delete errHandler;

    return AdditionalDependencies;
}

SxsEntries SxsManifest::ExtractDependenciesFromSxsManifestFile(wstring ManifestFile, wstring Folder, wstring ExecutableName/* = L""*/, bool Wow64Pe/* = false*/) {
    ifstream xmlText(ManifestFile);
    string xmlStr((istreambuf_iterator<char>(xmlText)), istreambuf_iterator<char>());
    return ExtractDependenciesFromSxsManifest(xmlStr, Folder, ExecutableName, Wow64Pe);
}

SxsEntries SxsManifest::GetSxsEntries(PEManager* Pe) {
    filesystem::path peFilePath = filesystem::path(Pe->filepath);
    wstring RootPeFolder = peFilePath.parent_path();
    wstring RootPeFilename = peFilePath.filename();

    // Look for overriding manifest file (named "{$name}.manifest)
    wstring OverridingManifest = peFilePath / L".manifest";
    //OverridingManifest = L"C:\\Users\\admin\\Git\\github\\dwalker\\test\\sample.one.dll.manifest";
    if (filesystem::exists(OverridingManifest)) {
        return ExtractDependenciesFromSxsManifestFile(
            OverridingManifest,
            RootPeFolder,
            RootPeFilename,
            Pe->IsWow64Dll()
        );
    }

    // Retrieve embedded manifest
    string PeManifest = Pe->GetManifest();
    if (PeManifest.empty())
        return SxsEntries();

    return ExtractDependenciesFromSxsManifest(
        PeManifest,
        RootPeFolder,
        RootPeFilename,
        Pe->IsWow64Dll()
    );
}
