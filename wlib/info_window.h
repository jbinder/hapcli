//*****************************************************************************
// InfoWindow
// ----------------------------------------------------------------------------
// file:   info_window.h
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-04-27
//*****************************************************************************

#ifndef _INFO_WINDOW_
#define _INFO_WINDOW_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "window.h"

class InfoWindow : public Window
{
public:
    InfoWindow(void);
    virtual ~InfoWindow(void);

    struct TextBorder
    {
        int left;
        int right;
        int top;
        int bottom;

        TextBorder(int border)
        {
            left = border;
            right = border;
            top = border;
            bottom = border;
        }
        TextBorder(int border_left, int border_right, int border_top, int border_bottom)
        {
            left = border_left;
            right = border_right;
            top = border_top;
            bottom = border_bottom;
        };
    };

    /* @see window */
    virtual void WL_PrepareWindow(HINSTANCE hInstance, std::string BackgroundFilename = "");

    virtual void WL_FadeIn(bool fade_in);

    inline virtual bool WL_IsInactive(void) { return m_State == INACTIVE; }
    inline virtual bool WL_IsAnimating(void) { return (m_State == FADEOUT) || (m_State == FADEIN) || (m_State == SHOW && m_AutoHide); }

    /* @return animation time in msec */
    inline virtual int WL_GetAnimationTime(void) { return m_AnimationTime / m_TimeForOneFrame; }
    inline virtual bool WL_GetAutoHide(void) { return m_AutoHide; }
    /* @param auto_hide after fade in the window will also fade out after auto_hide_timeout msec if set to TRUE */
    inline virtual void WL_SetAutoHide(bool auto_hide) { m_AutoHide = auto_hide; }
    inline virtual int WL_GetAutoHideTimeout(void) { return m_AutoHideTimeout; }
    /* @param auto_hide_timeout @see WL_SetAutoHide */
    inline virtual void WL_SetAutoHideTimeout(int auto_hide_timeout) { m_AutoHideTimeout = auto_hide_timeout; }

    inline virtual void WL_SetText(std::string new_text) { m_Text = new_text; }
    inline virtual void WL_SetTextBorder(TextBorder text_border) { m_TextBorder = text_border; }
    /* @param moving_allowed if FALSE the window will just fade in without animation */
    inline virtual void WL_SetMovingAllowed(bool moving_allowed) { m_MovingAllowed = moving_allowed; }
    /* @param animation_time time for fade in/out */
    inline virtual void WL_SetAnimationTime(int animation_time) { m_AnimationTime = animation_time * m_TimeForOneFrame; }
    /* @param time_for_one_frame specifies the frame rate of the animation */
    inline virtual void WL_SetTimeForOneFrame(int time_for_one_frame) { m_TimeForOneFrame = time_for_one_frame; }

protected:
    /* @see Window */
    virtual void WL_OnPaint(void);
    /* @see Window */
    virtual bool WL_OnTimer(WPARAM timer_id);

    virtual void WL_CreateFont(int font_size);
    virtual void WL_DeleteFont();

    std::string  m_Text;
    TextBorder   m_TextBorder;
    bool         m_MovingAllowed;
    int          m_AnimationTime;
    int          m_TimeForOneFrame;
    HFONT        m_Font;
    int          m_MaxFontSize;
    int          m_MaxAlpha;
    bool         m_AutoHide;
    int          m_AutoHideTimeout;
    int          m_CurAutoHideTime;

    enum States
    {
        INACTIVE,
        ACTIVE,
        FADEIN,
        SHOW,
        FADEOUT
    };
    States       m_State;
};

#endif /* _INFO_WINDOW_ */
