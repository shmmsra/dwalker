// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef DWAPP_H
#define DWAPP_H

#include <ph.h>
#include <guisup.h>
#include <provider.h>
#include <filestream.h>
#include <fastlock.h>
#include <treenew.h>
#include <graph.h>
#include <circbuf.h>
#include <dltmgr.h>
#include <phnet.h>

#endif DWAPP_H

#include <iostream>

#include <DWalker.h>
#include <PHLib.h>

using namespace std;

#define BINARY_PATH L".\\dwalker.exe"

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
    PHLib* phlib = PHLib::GetInstance();

    // always the first call to make
    if (!phlib->InitializePhLib()) {
        return 1;
    }

    wstring filePath;
    filePath = BINARY_PATH;

    DWalker dw;
    dw.DumpDependencyChain(filePath);
    dw.DumpDependencyChain(filePath);
    dw.DumpDependencyChain(filePath);
    dw.DumpDependencyChain(filePath);

    return 0;
}
