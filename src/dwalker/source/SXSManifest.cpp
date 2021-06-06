#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <PEManager.hpp>
#include <SXSManifest.hpp>

using namespace std;
using namespace xercesc;

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
    //Path = filesystem::path(Folder).append(filesystem::path(relPath));

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
            Version = sxsAssemblyIdentityNode->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("version"))->getNodeValue();
            Type = sxsAssemblyIdentityNode->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("type"))->getNodeValue();
            PublicKeyToken = sxsAssemblyIdentityNode->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("publicKeyToken"))->getNodeValue();

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

/*
    // find dll with same name as sxs assembly in target directory
    static SxsEntries SxsFindTargetDll(wstring AssemblyName, wstring Folder) {
        SxsEntries* EntriesFromElement = new SxsEntries();

        wstring TargetFilePath = Path.Combine(Folder, AssemblyName);
        if (File.Exists(TargetFilePath)) {
            var Name = System.IO.Path.GetFileName(TargetFilePath);
            var Path = TargetFilePath;

            EntriesFromElement.Add(new SxsEntry(Name, Path));
            return EntriesFromElement;
        }

        wstring TargetDllPath = Path.Combine(Folder, String.Format("{0:s}.dll", AssemblyName));
        if (File.Exists(TargetDllPath)) {
            var Name = System.IO.Path.GetFileName(TargetDllPath);
            var Path = TargetDllPath;

            EntriesFromElement.Add(new SxsEntry(Name, Path));
            return EntriesFromElement;
        }

        return EntriesFromElement;
    }
*/
/*
SxsEntries SxsManifest::ExtractDependenciesFromSxsElement(DOMDocument* doc, const wstring basePath, DOMXPathNSResolver* resolver, wstring Folder, wstring ExecutableName = L"", bool Wow64Pe = false) {
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
    if (SxsAssembly->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("publicKeyToken")) != nullptr) {
        wstring WinSxsDir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.Windows),
            "WinSxs"
        );

        wstring WinSxsManifestDir = filesystem::path(WinSxsDir) / L"Manifests";
        var RegisteredManifests = Directory.EnumerateFiles(
            WinSxsManifestDir,
            "*.manifest"
        );

        wstring PublicKeyToken = SxsAssembly->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("publicKeyToken"))->getNodeValue();
        wstring Name = wstring(SxsAssembly->getNodeValue()->getAttributes()->getNamedItem(XMLString::transcode("name"))->getNodeValue()).ToLower();
        wstring ProcessArch = SxsAssembly.Attribute("processorArchitecture") != null ? SxsAssembly.Attribute("processorArchitecture").Value : "*";
        wstring Version = SxsAssembly.Attribute("version").Value;
        wstring Langage = SxsAssembly.Attribute("langage") != null ? SxsAssembly.Attribute("langage").Value : "none"; // TODO : support localized sxs redirection


        switch (ProcessArch.ToLower()) {
        case "$(build.arch)":
        case "*":
            ProcessArch = (Wow64Pe) ? "x86" : "amd64";
            break;
        case "amd64":
        case "x86":
        case "wow64":
        case "msil":
            break; // nothing to do
        default:
            ProcessArch = ".*";
            break;
        }

        Regex VersionRegex = new Regex(@"([0 - 9] + )\.([0 - 9] + )\.([0 - 9] + )\.([0 - 9] + )", RegexOptions.IgnoreCase);
            Match VersionMatch = VersionRegex.Match(Version);

        if (VersionMatch.Success) {
            wstring Major = VersionMatch.Groups[1].Value;
            wstring Minor = VersionMatch.Groups[2].Value;
            wstring Build = (VersionMatch.Groups[3].Value == "0") ? ".*" : VersionMatch.Groups[3].Value;
            wstring Patch = (VersionMatch.Groups[4].Value == "0") ? ".*" : VersionMatch.Groups[4].Value;

            // Manifest filename : {ProcArch}_{Name}_{PublicKeyToken}_{FuzzyVersion}_{Langage}_{some_hash}.manifest
            Regex ManifestFileNameRegex = new Regex(
                String.Format(@"({0:s}_{ 1:s }_{ 2:s }_{ 3:s }\.{4:s}\.({ 5:s })\.({ 6:s })_none_([a - fA - F0 - 9] + ))\.manifest",
                ProcessArch,
                Name,
                PublicKeyToken,
                Major,
                Minor,
                Build,
                Patch
                //Langage,
                // some hash
            ),
                RegexOptions.IgnoreCase
                );

                bool FoundMatch = false;
                int HighestBuild = 0;
                int HighestPatch = 0;
                wstring MatchSxsManifestDir = "";
                wstring MatchSxsManifestPath = "";

                foreach(var FileName in RegisteredManifests) {
                    Match MatchingSxsFile = ManifestFileNameRegex.Match(FileName);
                    if (MatchingSxsFile.Success) {

                        int MatchingBuild = Int32.Parse(MatchingSxsFile.Groups[2].Value);
                        int MatchingPatch = Int32.Parse(MatchingSxsFile.Groups[3].Value);

                        if ((MatchingBuild > HighestBuild) || ((MatchingBuild == HighestBuild) && (MatchingPatch > HighestPatch))) {


                            string TestMatchSxsManifestDir = MatchingSxsFile.Groups[1].Value;

                            // Check the directory exists before confirming there is a match
                            string FullPathMatchSxsManifestDir = Path.Combine(WinSxsDir, TestMatchSxsManifestDir);
                            Console.WriteLine("FullPathMatchSxsManifestDir : Checking {0:s}", FullPathMatchSxsManifestDir);
                            if (NativeFile.Exists(FullPathMatchSxsManifestDir, true)) {

                                Console.WriteLine("FullPathMatchSxsManifestDir : Checking {0:s} TRUE", FullPathMatchSxsManifestDir);
                                FoundMatch = true;

                                HighestBuild = MatchingBuild;
                                HighestPatch = MatchingPatch;

                                MatchSxsManifestDir = TestMatchSxsManifestDir;
                                MatchSxsManifestPath = Path.Combine(WinSxsManifestDir, FileName);
                            }
                        }
                    }
                }

                if (FoundMatch) {

                    string FullPathMatchSxsManifestDir = Path.Combine(WinSxsDir, MatchSxsManifestDir);

                    // "{name}.local" local sxs directory hijack ( really used for UAC bypasses )
                    if (ExecutableName != "") {
                        string LocalSxsDir = Path.Combine(Folder, String.Format("{0:s}.local", ExecutableName));
                        string MatchingLocalSxsDir = Path.Combine(LocalSxsDir, MatchSxsManifestDir);

                        if (Directory.Exists(LocalSxsDir) && Directory.Exists(MatchingLocalSxsDir)) {
                            FullPathMatchSxsManifestDir = MatchingLocalSxsDir;
                        }
                    }


                    return ExtractDependenciesFromSxsManifestFile(MatchSxsManifestPath, FullPathMatchSxsManifestDir, ExecutableName, Wow64Pe);
                }
        }
    }

    // 1. \\<appdir>\<assemblyname>.DLL
    // find dll with same assembly name in same directory
    SxsEntries EntriesFromMatchingDll = SxsFindTargetDll(SxsManifestName, Folder);
    if (EntriesFromMatchingDll.Count > 0) {
        return EntriesFromMatchingDll;
    }


    // 2. \\<appdir>\<assemblyname>.manifest
    // find manifest with same assembly name in same directory
    TargetSxsManifestPath = Path.Combine(Folder, String.Format("{0:s}.manifest", SxsManifestName));
    if (File.Exists(TargetSxsManifestPath)) {
        return ExtractDependenciesFromSxsManifestFile(TargetSxsManifestPath, Folder, ExecutableName, Wow64Pe);
    }


    // 3. \\<appdir>\<assemblyname>\<assemblyname>.DLL
    // find matching dll in sub directory
    SxsEntries EntriesFromMatchingDllSub = SxsFindTargetDll(SxsManifestName, SxsManifestDir);
    if (EntriesFromMatchingDllSub.Count > 0) {
        return EntriesFromMatchingDllSub;
    }

    // 4. \<appdir>\<assemblyname>\<assemblyname>.manifest
    // find manifest in sub directory
    TargetSxsManifestPath = Path.Combine(SxsManifestDir, String.Format("{0:s}.manifest", SxsManifestName));
    if (Directory.Exists(SxsManifestDir) && File.Exists(TargetSxsManifestPath)) {
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
        SxsEntries EntriesFromElement = new SxsEntries();
        EntriesFromElement.Add(new SxsEntry(SxsManifestName, "file ???"));
        return EntriesFromElement;
    }
}
*/
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

SxsEntries SxsManifest::ExtractDependenciesFromSxsManifestFile(wstring ManifestFile, wstring Folder, wstring ExecutableName/* = L""*/, bool Wow64Pe/* = false*/) {
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

    XercesDOMParser* parser = new XercesDOMParser();
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);    // optionalB

    ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
    parser->setErrorHandler(errHandler);

    SxsEntries AdditionalDependencies;

    try {
        parser->parse(ManifestFile.c_str());

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
            const wstring assemblyIdentityPathStr = L"/assembly/dependency/dependentAssembly/assemblyIdentity";
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

/*
    SxsEntries SxsManifest::ExtractDependenciesFromSxsManifest(wstring ManifestStream, wstring Folder, wstring ExecutableName = L"", bool Wow64Pe = false) {
        SxsEntries AdditionnalDependencies = new SxsEntries();

        xml_document XmlManifest = ParseSxsManifest(ManifestStream);
        XNamespace Namespace = XmlManifest.Root.GetDefaultNamespace();

        // Find any declared dll
        //< assembly xmlns = 'urn:schemas-microsoft-com:asm.v1' manifestVersion = '1.0' >
        //    < assemblyIdentity name = 'additional_dll' version = 'XXX.YY.ZZ' type = 'win32' />
        //    < file name = 'additional_dll.dll' />
        //</ assembly >
        foreach(XElement SxsAssembly in XmlManifest.Descendants(Namespace + "assembly")) {
            AdditionnalDependencies.AddRange(SxsEntries.FromSxsAssembly(SxsAssembly, Namespace, Folder));
        }



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
        foreach(XElement SxsAssembly in XmlManifest.Descendants(Namespace + "dependency")
            .Elements(Namespace + "dependentAssembly")
            .Elements(Namespace + "assemblyIdentity")
        ) {
            // find target PE
            AdditionnalDependencies.AddRange(ExtractDependenciesFromSxsElement(SxsAssembly, Folder, ExecutableName, Wow64Pe));
        }

        return AdditionnalDependencies;
    }

    xml_document SxsManifest::ParseSxsManifest(System.IO.Stream ManifestStream) {
        xml_document doc;
        // You can use load_buffer to load document from immutable memory block:
        xml_parse_result result = doc.load_buffer(PeManifest.c_str(), PeManifest.size());

        XDocument XmlManifest = null;
        // Hardcode namespaces for manifests since they are no always specified in the embedded manifests.
        XmlNamespaceManager nsmgr = new XmlNamespaceManager(new NameTable());
        nsmgr.AddNamespace(String.Empty, "urn:schemas-microsoft-com:asm.v1"); //default namespace : manifest V1
        nsmgr.AddNamespace("asmv3", "urn:schemas-microsoft-com:asm.v3");      // sometimes missing from manifests : V3
        nsmgr.AddNamespace("asmv3", "http://schemas.microsoft.com/SMI/2005/WindowsSettings");      // sometimes missing from manifests : V3
        XmlParserContext context = new XmlParserContext(null, nsmgr, null, XmlSpace.Preserve);




        using (StreamReader xStream = new StreamReader(ManifestStream)) {
            // Trim double quotes in manifest attributes
            // Example :
            //      * Before : <assemblyIdentity name=""Microsoft.Windows.Shell.DevicePairingFolder"" processorArchitecture=""amd64"" version=""5.1.0.0"" type="win32" />
            //      * After  : <assemblyIdentity name="Microsoft.Windows.Shell.DevicePairingFolder" processorArchitecture="amd64" version="5.1.0.0" type="win32" />

            string PeManifest = xStream.ReadToEnd();
            PeManifest = new Regex("\\\"\\\"([\\w\\d\\.]*)\\\"\\\"").Replace(PeManifest, "\"$1\""); // Regex magic here

            // some manifests have "macros" that break xml parsing
            PeManifest = new Regex("SXS_PROCESSOR_ARCHITECTURE").Replace(PeManifest, "\"amd64\"");
            PeManifest = new Regex("SXS_ASSEMBLY_VERSION").Replace(PeManifest, "\"\"");
            PeManifest = new Regex("SXS_ASSEMBLY_NAME").Replace(PeManifest, "\"\"");

            using (XmlTextReader xReader = new XmlTextReader(PeManifest, XmlNodeType.Document, context)) {
                XmlManifest = XDocument.Load(xReader);
            }
        }

        return XmlManifest;
    }
*/

SxsEntries SxsManifest::GetSxsEntries(PEManager* Pe) {
    filesystem::path peFilePath = filesystem::path(Pe->filepath);
    wstring RootPeFolder = peFilePath.parent_path();
    wstring RootPeFilename = peFilePath.filename();

    // Look for overriding manifest file (named "{$name}.manifest)
    wstring OverridingManifest = peFilePath / L".manifest";
    OverridingManifest = L"C:\\Users\\admin\\Git\\github\\dwalker\\test\\sample.one.dll.manifest";
    if (filesystem::exists(OverridingManifest)) {
        return ExtractDependenciesFromSxsManifestFile(
            OverridingManifest,
            RootPeFolder,
            RootPeFilename,
            Pe->IsWow64Dll()
        );
    }

    // Retrieve embedded manifest
    wstring PeManifest = Pe->GetManifest();
    if (PeManifest.empty())
        return SxsEntries();
    /*
            return ExtractDependenciesFromSxsManifest(
                PeManifest,
                RootPeFolder,
                RootPeFilename,
                Pe->IsWow64Dll()
            );
    */
    return ExtractDependenciesFromSxsManifestFile(
        PeManifest,
        RootPeFolder,
        RootPeFilename,
        Pe->IsWow64Dll()
    );
}
