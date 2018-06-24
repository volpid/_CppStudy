
#include "Common/d3d_app.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif /**/

    _Unreferenced_parameter_(prevInstance);
    _Unreferenced_parameter_(cmdLine);
    _Unreferenced_parameter_(showCmd);

    try
    {
        D3DApp* pApp = D3DApp::GetApp();
        if (pApp == nullptr || pApp->Initialize(hInstance) == false)
        {
            return -1;
        }

        return pApp->Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), "HR Failed", MB_OK);
    }

    return 0;
}