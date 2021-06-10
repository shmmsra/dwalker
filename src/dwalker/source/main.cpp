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

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
//    return xercesDOMParser();

    PHLib* phlib = PHLib::GetInstance();

    // always the first call to make
    if (!phlib->InitializePhLib()) {
        return 1;
    }

    wstring filePath = L"C:\\Users\\shmishra\\Git\\github\\dwalker\\test\\sample.one.dll";
    //filePath = BINARY_PATH;

    DWalker dw;
    dw.DumpDependencyChain(filePath);

    return 0;
}
