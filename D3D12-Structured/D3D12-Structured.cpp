// D3D12-Structured.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include"D3DApp.h"
#include"d3dUtil.h"
#include <iostream>
#include<DirectXColors.h>
#include"BoxApp.h"


int main()
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        BoxApp theApp(nullptr);
        if (!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

