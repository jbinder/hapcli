//*****************************************************************************
// Window
// ----------------------------------------------------------------------------
// file:   window.cpp
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-05-01
//*****************************************************************************

#include "window.h"
#include <time.h>

Window::Window(void) :
    m_AppName("Window"),
    m_BackgroundFilename("background.bmp"),
    m_Width(0),
    m_Height(0),
    m_PosX(0),
    m_PosY(0),
    m_TimerId(1),
    m_Alpha(255),
    m_ColorKey(RGB(255, 0, 255)),
    m_IsMoveAble(TRUE),
    m_IsMoving(FALSE),
    m_hbmBackground(0)
{
}

Window::~Window(void)
{
}

void Window::WL_PrepareWindow(HINSTANCE hInstance, std::string BackgroundFilename /* = "" */)
{
    m_hInstance = hInstance;

    if (BackgroundFilename != "") m_BackgroundFilename = BackgroundFilename;

    m_hbmBackground = static_cast<HBITMAP>(
        LoadImage(NULL, m_BackgroundFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION));
    if (!m_hbmBackground)
    {
        WL_Error("Background image not found.");
        return;
    }

    BITMAP bm;
    GetObject(m_hbmBackground, sizeof(bm), &bm);
    m_Height = bm.bmHeight;
    m_Width  = bm.bmWidth;

    WL_RegisterClass();
    WL_CreateWindow();
}

void Window::WL_RegisterClass(void)
{
    WNDCLASS wc = {
        CS_HREDRAW | CS_VREDRAW,   // redraw window on resize
        WL_WindowProc,             // msg handler
        0,                         // additional memory
        0,                         // additional memory
        m_hInstance,
        LoadIcon(NULL,IDI_APPLICATION),
        LoadCursor(NULL,IDC_ARROW),
        (HBRUSH)GetStockObject(BLACK_BRUSH),
        NULL,
        m_AppName.c_str()
    };

    RegisterClass(&wc);
}

void Window::WL_CreateWindow(void)
{
    m_hWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
        m_AppName.c_str(),
        m_AppName.c_str(),
        WS_POPUP,
        m_PosX,
        m_PosY,
        m_Width,
        m_Height,
        NULL,
        NULL,
        m_hInstance,
        NULL);

    SetLayeredWindowAttributes(m_hWnd, m_ColorKey, m_Alpha, LWA_ALPHA | LWA_COLORKEY);

    SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)(this));
}

void Window::WL_DestroyWindow(void)
{
    if (m_hbmBackground)
    {
        DeleteObject(m_hbmBackground);
    }
}

void Window::WL_OnPaint(void)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);

    HDC hdcBuffer = CreateCompatibleDC(NULL);
    HBITMAP hbmT = SelectBitmap(hdcBuffer, m_hbmBackground);
    BitBlt(hdc, 0, 0, m_Width, m_Height, hdcBuffer, 0, 0, SRCCOPY);

    SelectBitmap(hdcBuffer, hbmT);
    DeleteDC(hdcBuffer);
    EndPaint(m_hWnd, &ps);
}

void Window::WL_OnLeftButtonDown(void)
{
    if (!m_IsMoveAble) return;

    m_IsMoving = TRUE;
    GetCursorPos(&m_MousePos); // TODO: check, maybe get from lParam/wParam
    SetCapture(m_hWnd);
}

void Window::WL_OnMouseMove(void)
{
    if (!m_IsMoving) return;

    POINT old_mouse_pos = m_MousePos;
    GetCursorPos(&m_MousePos);

    WL_SetPosition(
        m_PosX - (old_mouse_pos.x - m_MousePos.x),
        m_PosY - (old_mouse_pos.y - m_MousePos.y));
}

void Window::WL_OnLeftButtonUp(void)
{
    if (!m_IsMoving) return;

    m_IsMoving = FALSE;
    ReleaseCapture();
}

LRESULT CALLBACK Window::WL_WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    Window* window = (Window*)GetWindowLong(hwnd, GWL_USERDATA);

    switch(uMsg)
    {
    case WM_CREATE:
        break;

    case WM_DESTROY:
        window->WL_DestroyWindow();
        return 0;

    case WM_PAINT:
        window->WL_OnPaint();
        return 0;

    case WM_LBUTTONDOWN:
        window->WL_OnLeftButtonDown();
        return 0;

    case WM_LBUTTONUP:
        window->WL_OnLeftButtonUp();
        return 0;

    case WM_MOUSEMOVE:
        window->WL_OnMouseMove();
        return 0;

    case WM_TIMER:
        if (window->WL_OnTimer(wParam))
        {
            return 0;
        }
        break;
    }

    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

void Window::WL_SetPosition(int x, int y)
{
    m_PosX = x;
    m_PosY = y;

    MoveWindow(m_hWnd, m_PosX, m_PosY, m_Width, m_Height, FALSE);
}

void Window::WL_Error(std::string error_describtion)
{
    MessageBox(m_hWnd, error_describtion.c_str(), m_AppName.c_str(), MB_ICONERROR);
    SendMessage(m_hWnd, WM_CLOSE, 0, 0); //WM_DESTROY
}
