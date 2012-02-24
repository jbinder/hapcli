//*****************************************************************************
// Animation
// ----------------------------------------------------------------------------
// file:   animation.h
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-06-06
//*****************************************************************************

#ifndef _ANIMATION_
#define _ANIMATION_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include <vector>

typedef unsigned int uint;

class Animation
{
public:
    enum Type
    {
        NO_ANIM         = 0,
        BACKGROUND_ANIM = 1,
        RANDOM_ANIM     = 2
    };

    enum State
    {
        IDLE,
        RUNNING
    };

    Animation(void);
    virtual ~Animation(void);

    /* @short gets drawn only if it is active */
    inline virtual void Activate(bool activate) { m_Active = activate; }
    /* @short start or stop the animation */
    virtual void Start(bool start);
    /* @return TRUE if successful, otherwise FALSE */
    virtual bool InitAnimation(std::string& folder, int pos_x, int pos_y, int speed, bool loop, Type type);
    /* @short draws the next frame of the animation */
    virtual void UpdateAnimation(HDC* hdc);

    inline virtual Type GetType(void) const { return m_Type; }
    inline virtual bool IsIdle(void) const  { return m_State == IDLE; };

protected:
    void ReadFolder(std::string& folder, std::vector<std::string>& folder_content);

    std::vector<HBITMAP> m_Frames;
    int                  m_PosX;
    int                  m_PosY;
    int                  m_Height;
    int                  m_Width;
    bool                 m_Active;
    uint                 m_CurrentFrame;
    Type                 m_Type;
    int                  m_Speed;
    int                  m_Delay;
    State                m_State;
    bool                 m_Loop;
};

#endif /* _ANIMATION_ */
