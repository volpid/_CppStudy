
#ifndef __CORE_WINDOW__H__
#define __CORE_WINDOW__H__

#include <string>
#include <Windows.h>

//-----------------------------------------------------------------------------------------
// CoreWindowApp
//-----------------------------------------------------------------------------------------
class CoreWindowApp
{
public:
    static CoreWindowApp* GetInstance(void);

private:
    static CoreWindowApp* thisApp;

public:
    CoreWindowApp(void);
    virtual ~CoreWindowApp(void);

    CoreWindowApp(const CoreWindowApp& other) = delete;
    CoreWindowApp& operator=(const CoreWindowApp& other) = delete;

    virtual LRESULT MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual bool InitializeApp(void);
    virtual void Update(void);
    virtual void CleanUp(void);

    bool Initialize(HINSTANCE hInstance, const char* wndClass, const char* wndCaption);
    int Run(void);

private:
    bool InitializeWindow(void);
    
protected:
    HINSTANCE hAppInst_ = nullptr;
    HWND hMainWnd_ = nullptr;
    
    int clientWidth_ = 800;
    int clientHeight_ = 600;

    std::string mainWndClass_;
    std::string mainWndCaption_;
};


#endif /*__CORE_WINDOW__H__*/