//*****************************************************************************
// InfoWindow
// ----------------------------------------------------------------------------
// file:   info_window.cpp
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-04-27
//*****************************************************************************

#include "info_window.h"

InfoWindow::InfoWindow(void) :
    Window(),
    m_State(INACTIVE),
    m_Text(""),
    m_TextBorder(5),
    m_MovingAllowed(TRUE),
    m_AnimationTime(3 * 1000),
    m_TimeForOneFrame(25),
    m_Font(0),
    m_MaxFontSize(10),
    m_MaxAlpha(255),
    m_AutoHide(TRUE),
    m_AutoHideTimeout(1000),
    m_CurAutoHideTime(0)
{
    m_TimerId = rand() % 999 + 100;
    m_IsMoveAble = FALSE;
    m_PosX = -999;
    m_PosY = -999;
}

InfoWindow::~InfoWindow(void)
{
    WL_DeleteFont();
}

void InfoWindow::WL_PrepareWindow(HINSTANCE hInstance, std::string BackgroundFilename /*= "" */)
{
    Window::WL_PrepareWindow(hInstance, BackgroundFilename);

    WL_CreateFont(m_MaxFontSize);

    if ((m_PosX == -999) && (m_PosY == -999))
    {
        RECT work_area;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
        m_PosX = work_area.right;
        m_PosY = work_area.bottom - m_Height;
    }

    MoveWindow(m_hWnd, m_PosX, m_PosY, m_Width, m_Height, false);
}

void InfoWindow::WL_OnPaint(void)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);

    // draw background
    HDC hdcBuffer = CreateCompatibleDC(NULL);
    HBITMAP prevBitmap = SelectBitmap(hdcBuffer, m_hbmBackground);
    BitBlt(hdc, 0, 0, m_Width, m_Height, hdcBuffer, 0, 0, SRCCOPY);

    // draw text
    RECT text_plane, calc_plane;
    text_plane.left = m_TextBorder.left;
    text_plane.top = m_TextBorder.top;
    text_plane.right = m_Width - m_TextBorder.right;
    text_plane.bottom = m_Height - m_TextBorder.bottom;

    int prevMode = SetBkMode(hdc, TRANSPARENT);
    COLORREF prevColor = SetTextColor(hdc, RGB(0, 0, 0));
    HFONT prevFont = (HFONT)SelectObject(hdc, m_Font);

    // - check if font fits in the textplane
    UINT format = DT_WORDBREAK | DT_NOCLIP | DT_CENTER | DT_VCENTER;
    bool to_big = false;
    int font_size = m_MaxFontSize;

    do
    {
        calc_plane = text_plane;

        DrawText(hdc, m_Text.c_str(), static_cast<int>(m_Text.size()),
            &calc_plane, format | DT_CALCRECT);

        // -- resize the font if necessary
        if (calc_plane.bottom > text_plane.bottom ||
            calc_plane.right > text_plane.right)
        {
            to_big = true;
            WL_CreateFont(--font_size);
            SelectObject(hdc, m_Font);
        }
        else
        {
            to_big = false;
        }
    }
    while (to_big && font_size > 4);

    if (to_big)
    {
        m_Text = "-to big-";
    }

    // - center the text
    text_plane.top += (text_plane.bottom - calc_plane.bottom) / 2;
    text_plane.bottom -= (text_plane.bottom - calc_plane.bottom) / 2;

    // - draw the text
    DrawText(hdc, m_Text.c_str(), static_cast<int>(m_Text.size()), &text_plane, format);

    // clean up
    WL_CreateFont(m_MaxFontSize);

    SelectObject(hdc, prevFont);
    SetTextColor(hdc, prevColor);
    SetBkMode(hdc, prevMode);

    SelectBitmap(hdcBuffer, prevBitmap);
    DeleteDC(hdcBuffer);
    EndPaint(m_hWnd, &ps);
}

bool InfoWindow::WL_OnTimer(WPARAM timer_id)
{
    Window::WL_OnTimer(timer_id);

    if (timer_id != m_TimerId) return FALSE;

    bool anim_finished = false;
    bool update_window = false;

    int move_y = static_cast<int>(m_Height * 100 / m_AnimationTime);
    int alpha_change = static_cast<int>(m_MaxAlpha * 100 / m_AnimationTime);

    switch (m_State)
    {
    case ACTIVE:
    case INACTIVE:
        break;

    case FADEIN:
        m_Alpha += alpha_change;
        if (m_Alpha >= m_MaxAlpha)
        {
            m_Alpha = m_MaxAlpha;

            if (m_AutoHide)
            {
                m_CurAutoHideTime = 0;
                m_State = SHOW;
            }
            else
            {
                anim_finished = true;
            }
        }

        if (m_MovingAllowed) m_PosY -= move_y;

        update_window = true;
        break;

    case SHOW:
        if (m_MovingAllowed) m_PosY -= move_y;

        if (m_AutoHideTimeout > m_CurAutoHideTime)
        {
            m_CurAutoHideTime += m_TimeForOneFrame;
        }
        else
        {
            m_CurAutoHideTime = 0;
            m_State = FADEOUT;
        }

        update_window = true;
        break;

    case FADEOUT:
        m_Alpha -= alpha_change;
        if (m_Alpha <= 0)
        {
            m_Alpha = 0;
            anim_finished = true;
        }

        if (m_MovingAllowed) m_PosY -= move_y;

        update_window = true;
        break;
    }

    if (update_window)
    {
        SetLayeredWindowAttributes(m_hWnd, m_ColorKey, m_Alpha, LWA_ALPHA | LWA_COLORKEY);
        MoveWindow(m_hWnd, m_PosX, m_PosY, m_Width, m_Height, false);
    }

    if (anim_finished)
    {
        KillTimer(m_hWnd, m_TimerId);
        if (m_State == FADEOUT)
        {
            m_State = INACTIVE;
            ShowWindow(m_hWnd, SW_HIDE);
        }
        else
        {
            m_State = ACTIVE;
        }
    }

    return TRUE;
}

void InfoWindow::WL_FadeIn(bool fade_in)
{
    if ((fade_in && m_State == ACTIVE) || (!fade_in && m_State == INACTIVE))
    {
        return;
    }

    if (m_State == FADEIN || m_State == FADEOUT)
    {
        KillTimer(m_hWnd, m_TimerId);
    }

    if (fade_in)
    {
        m_Alpha = 0;
        SetLayeredWindowAttributes(m_hWnd, m_ColorKey, m_Alpha, LWA_ALPHA | LWA_COLORKEY);
        m_State = FADEIN;
        ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
        InvalidateRect(m_hWnd, NULL, TRUE);
    }
    else
    {
        m_State = FADEOUT;
    }

    SetTimer(m_hWnd, m_TimerId, m_TimeForOneFrame, NULL); // TODO: exception
}

void InfoWindow::WL_CreateFont(int font_size)
{
    WL_DeleteFont();
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);

    LOGFONT lf;
    lf.lfHeight         = -MulDiv(font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_MEDIUM;
    lf.lfItalic         = 0;
    lf.lfUnderline      = 0;
    lf.lfStrikeOut      = 0;
    lf.lfCharSet        = DEFAULT_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = CLEARTYPE_QUALITY;
    lf.lfPitchAndFamily = 0;
    sprintf_s((LPSTR)&lf.lfFaceName, sizeof(lf.lfFaceName), "%s", "Verdana");

    m_Font = CreateFontIndirect(&lf);

    EndPaint(m_hWnd, &ps);
}

void InfoWindow::WL_DeleteFont(void)
{
    if (m_Font)
    {
        DeleteObject(m_Font);
    }
}
