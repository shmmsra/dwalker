// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef DWAPP_H
#define DWAPP_H

#define WIN32_LEAN_AND_MEAN 1
#ifndef __MSXML_LIBRARY_DEFINED__
#define __MSXML_LIBRARY_DEFINED__
#endif

#include <ph.h>
#include <guisup.h>
#include <provider.h>
#include <filestream.h>
#include <fastlock.h>
#include <treenew.h>
#include <graph.h>
#include <circbuf.h>
#include <dltmgr.h>

#endif DWAPP_H

#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <streambuf>

#include <DWalker.hpp>
#include <PHLib.hpp>
#include <Logger.hpp>

using namespace std;

#define BINARY_PATH L".\\dwalker.exe"


#include <xercesc/util/XercesVersion.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

using namespace xercesc;

int xercesDOMParser() {
    try {
        XMLPlatformUtils::Initialize();
    } catch (const XMLException & toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        cout << "Error during initialization! :\n"
            << message << "\n";
        XMLString::release(&message);
        return 1;
    }

    XercesDOMParser* parser = new XercesDOMParser();
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);    // optionalB

    ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
    parser->setErrorHandler(errHandler);

    const char* xmlFile = "C:\\Users\\admin\\Git\\github\\dwalker\\build\\Debug\\dwalker\\manifest.xml";

    try {
        ifstream xmlText(xmlFile);
        string xmlStr((istreambuf_iterator<char>(xmlText)), istreambuf_iterator<char>());
        xmlStr = regex_replace(xmlStr, regex("\\\"\\\"([\\w\\d\\.]*)\\\"\\\""), "\"$1\"");
        xmlStr = regex_replace(xmlStr, regex("SXS_PROCESSOR_ARCHITECTURE"), "\"amd64\"");
        xmlStr = regex_replace(xmlStr, regex("SXS_ASSEMBLY_VERSION"), "\"\"");
        xmlStr = regex_replace(xmlStr, regex("SXS_ASSEMBLY_NAME"), "\"\"");

        MemBufInputSource xmlBuf((const XMLByte*)xmlStr.c_str(), xmlStr.size(), "sxs-manifest-xml");
        parser->parse(xmlBuf);

        // get the DOM representation
        DOMDocument* doc = parser->getDocument();

        XMLCh* xpathStr = XMLString::transcode("//assemblyIdentity");
        DOMElement* root = doc->getDocumentElement();
        try {
            DOMXPathNSResolver* resolver = doc->createNSResolver(root);
            resolver->addNamespaceBinding(nullptr, XMLString::transcode("urn:schemas-microsoft-com:asm.v1"));   // default namespace: manifest V1
            resolver->addNamespaceBinding(XMLString::transcode("asmv3"), XMLString::transcode("urn:schemas-microsoft-com:asm.v1"));   // sometimes missing from manifests: V3
            resolver->addNamespaceBinding(XMLString::transcode("asmv3"), XMLString::transcode("http://schemas.microsoft.com/SMI/2005/WindowsSettings"));   // sometimes missing from manifests: V3

            DOMXPathResult* result = doc->evaluate(
                xpathStr,
                root,
                resolver,
                DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                NULL);

            XMLSize_t nLength = result->getSnapshotLength();
            for (XMLSize_t i = 0; i < nLength; i++) {
                result->snapshotItem(i);
            }

            result->release();
            resolver->release();
        } catch (const DOMXPathException & e) {
        } catch (const DOMException & e) {
        }
        XMLString::release(&xpathStr);
    } catch (const XMLException & toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        cout << "Exception message is: \n"
            << message << "\n";
        XMLString::release(&message);
        return -1;
    } catch (const DOMException & toCatch) {
        char* message = XMLString::transcode(toCatch.msg);
        cout << "Exception message is: \n"
            << message << "\n";
        XMLString::release(&message);
        return -1;
    } catch (...) {
        cout << "Unexpected Exception \n";
        return -1;
    }

    delete parser;
    delete errHandler;
    return 0;
}

void PrintUsage(const wchar_t* programName) {
    wcout << L"Usage: " << programName << L" [OPTIONS] <target_file>" << endl;
    wcout << L"Options:" << endl;
    wcout << L"  -v, --verbose       Enable verbose output" << endl;
    wcout << L"  -d, --depth N       Maximum recursion depth (default: 10, max: 100)" << endl;
    wcout << L"  --log-level LEVEL   Set logging level: error, warn, info, debug (default: info)" << endl;
    wcout << L"  --json              Output in JSON format" << endl;
    wcout << L"  -h, --help          Show this help message" << endl;
    wcout << endl;
    wcout << L"Examples:" << endl;
    wcout << L"  " << programName << L" C:\\Windows\\System32\\notepad.exe" << endl;
    wcout << L"  " << programName << L" --verbose --depth 5 C:\\Windows\\System32\\notepad.exe" << endl;
    wcout << L"  " << programName << L" --log-level debug C:\\Windows\\System32\\notepad.exe" << endl;
    wcout << L"  " << programName << L" --json C:\\Windows\\System32\\notepad.exe" << endl;
    wcout << L"  " << programName << L" --json --depth 3 C:\\Windows\\System32\\notepad.exe" << endl;
    wcout << endl;
    wcout << L"Depth Control:" << endl;
    wcout << L"  The --depth option controls how deep the dependency analysis goes." << endl;
    wcout << L"  - Depth 0: Only the root executable" << endl;
    wcout << L"  - Depth 1: Root + direct dependencies" << endl;
    wcout << L"  - Depth 2: Root + direct + indirect dependencies" << endl;
    wcout << L"  - And so on..." << endl;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
    // Parse command line arguments
    bool verbose = false;
    int maxDepth = 10;
    wstring targetFile;
    LogLevel logLevel = LogLevel::LOG_INFO;
    bool jsonOutput = false;

    for (int i = 1; i < argc; i++) {
        wstring arg = argv[i];
        if (arg == L"-h" || arg == L"--help") {
            PrintUsage(argv[0]);
            return 0;
        }
        else if (arg == L"-v" || arg == L"--verbose") {
            verbose = true;
        }
        else if (arg == L"-d" || arg == L"--depth") {
            if (i + 1 < argc) {
                maxDepth = _wtoi(argv[++i]);
                if (maxDepth <= 0) {
                    wcerr << L"Error: Depth must be a positive integer." << endl;
                    return 1;
                }
                if (maxDepth > 100) {
                    wcerr << L"Error: Depth cannot exceed 100. Using maximum allowed depth." << endl;
                    maxDepth = 100;
                }
            } else {
                wcerr << L"Error: -d/--depth requires a depth argument." << endl;
                return 1;
            }
        }
        else if (arg == L"--log-level") {
            if (i + 1 < argc) {
                wstring levelStr = argv[++i];
                if (levelStr == L"error") {
                    logLevel = LogLevel::LOG_ERROR;
                } else if (levelStr == L"warn") {
                    logLevel = LogLevel::LOG_WARN;
                } else if (levelStr == L"info") {
                    logLevel = LogLevel::LOG_INFO;
                } else if (levelStr == L"debug") {
                    logLevel = LogLevel::LOG_DEBUG;
                } else {
                    wcerr << L"Error: Invalid log level '" << levelStr << L"'. Using default (info)." << endl;
                    wcerr << L"Valid levels: error, warn, info, debug" << endl;
                    logLevel = LogLevel::LOG_INFO;
                }
            } else {
                wcerr << L"Error: --log-level requires a level argument." << endl;
                return 1;
            }
        }
        else if (arg == L"--json") {
            jsonOutput = true;
        }
        else if (arg[0] != L'-') {
            // This should be the target file
            if (targetFile.empty()) {
                targetFile = arg;
            } else {
                wcerr << L"Error: Multiple target files specified. Only one file can be analyzed at a time." << endl;
                return 1;
            }
        }
        else {
            wcerr << L"Error: Unknown option '" << arg << L"'" << endl;
            PrintUsage(argv[0]);
            return 1;
        }
    }
    
    // Check if target file was provided
    if (targetFile.empty()) {
        wcerr << L"Error: No target file specified." << endl;
        PrintUsage(argv[0]);
        return 1;
    }
    
    // Check if target file exists
    if (!filesystem::exists(targetFile)) {
        wcerr << L"Error: File not found: " << targetFile << endl;
        return 1;
    }

    // Initialize logger
    Logger::Initialize(logLevel, wcout, wcerr);
    LOG_INFO(L"Logger initialized with level: " + to_wstring(static_cast<int>(logLevel)));

    // Initialize PHLib
    PHLib* phlib = PHLib::GetInstance();
    LOG_DEBUG_FUNC(L"main", L"Getting PHLib instance...");
    if (!phlib->InitializePhLib()) {
        LOG_ERROR(L"Failed to initialize PHLib.");
        return 1;
    }
    LOG_INFO(L"PHLib initialized successfully.");

    // Create DWalker instance and configure it
    LOG_DEBUG_FUNC(L"main", L"Creating DWalker instance...");
    DWalker dw(wcout);
    dw.SetVerbose(verbose);
    dw.SetMaxDepth(maxDepth);
    LOG_INFO(L"DWalker configured successfully.");
    
    // Analyze dependencies
    wcout << L"========================================" << endl;
    LOG_INFO(L"Starting dependency analysis...");
    bool success;
    
    if (jsonOutput) {
        success = dw.DumpDependencyChainJson(targetFile);
    } else {
        success = dw.DumpDependencyChain(targetFile);
    }
    
    LOG_INFO(L"Analysis completed with result: " + wstring(success ? L"true" : L"false"));
    wcout << L"========================================" << endl;
    
    if (success) {
        wcout << L"[SUCCESS] Analysis completed successfully." << endl;
        return 0;
    } else {
        wcerr << L"[ERROR] Analysis failed or incomplete." << endl;
        return 1;
    }
}
