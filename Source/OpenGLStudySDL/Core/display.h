
#ifndef __DISPLAY__H__
#define __DISPLAY__H__

#include <glew/glew.h>
#include <SDL2/SDL.h>
#include <string>

class Display
{
public:
    Display(int width, int height, std::string& title);    
    virtual ~Display(void);

    void Update(void);
    void Clear(float r, float g, float b, float a);
    
    inline bool IsClosed(void) const;
    inline void SetClosed(void);

private:
    Display(const Display& other) = delete;
    Display(Display&& other) = delete;
    Display& operator=(const Display& other) = delete;
    Display& operator=(Display&& other) = delete;

private:
    SDL_Window* pWindow_ = nullptr;
    SDL_GLContext pOpenGLContext_ = nullptr;
    bool isClosed_ = false;
};

//---------------------------------------------------------------------------
// Inline section - Display
//---------------------------------------------------------------------------
inline bool Display::IsClosed(void) const
{
    return isClosed_;
}

inline void Display::SetClosed(void)
{
    isClosed_ = true;
}

#endif /*__DISPLAY__H__*/