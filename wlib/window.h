//*****************************************************************************
// Window
// ----------------------------------------------------------------------------
// file:   window.h
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-05-01
//*****************************************************************************

#ifndef _WINDOW_
#define _WINDOW_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define _WIN32_WINNT 0x0501 // for CreateWindowEx flags

#include <windows.h>
#include <windowsx.h>
#include <string>

class Window
{
public:
    Window(void);
    virtual ~Window(void);

    virtual void WL_PrepareWindow(HINSTANCE hInstance, std::string BackgroundFilename = "");
    virtual void WL_DestroyWindow(void);

    inline virtual void WL_ShowWindow(int nCmdShow)
    {
        m_WindowState = nCmdShow;
        ShowWindow(m_hWnd, m_WindowState);
        UpdateWindow(m_hWnd);
    }

    inline virtual bool WL_IsWindowActive(void) { return m_WindowState != SW_HIDE; }

    virtual void WL_SetPosition(int x, int y);
    inline virtual void WL_SetMoveAble(bool move_able)      { m_IsMoveAble = move_able; }
    inline virtual void WL_SetColorKey (COLORREF color_key) { m_ColorKey = color_key; }

    inline virtual int WL_GetWidth(void)  { return m_Width; }
    inline virtual int WL_GetHeight(void) { return m_Height; }
    inline virtual int WL_GetPosX(void)   { return m_PosX; }
    inline virtual int WL_GetPosY(void)   { return m_PosY; }

protected:
    virtual void WL_RegisterClass(void);
    virtual void WL_CreateWindow(void);
    virtual void WL_OnPaint(void);
    /* return TRUE if message was handled, else FALSE */
    virtual bool WL_OnTimer(WPARAM timer_id) { return FALSE; }
    virtual void WL_OnLeftButtonDown(void);
    virtual void WL_OnMouseMove(void);
    virtual void WL_OnLeftButtonUp(void);

    virtual void WL_Error(std::string error_describtion);
    static LRESULT CALLBACK WL_WindowProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);

    HINSTANCE    m_hInstance;
    HWND         m_hWnd;
    std::string  m_AppName;
    std::string  m_BackgroundFilename;
    HBITMAP      m_hbmBackground;
    int          m_Width;
    int          m_Height;
    int          m_PosX;
    int          m_PosY;
    int          m_TimerId;
    COLORREF     m_ColorKey;
    int          m_Alpha;
    int          m_WindowState;

    // for making moveable
    POINT        m_MousePos;
    bool         m_IsMoving;
    bool         m_IsMoveAble;
};

#endif /* _WINDOW_ */
