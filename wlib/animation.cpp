//*****************************************************************************
// Animation
// ----------------------------------------------------------------------------
// file:   animation.cpp
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-06-06
//*****************************************************************************

#include "animation.h"
#include <string>
#include <windowsx.h>

Animation::Animation(void) :
    m_PosX(0),
    m_PosY(0),
    m_Height(0),
    m_Width(0),
    m_CurrentFrame(0),
    m_Active(TRUE),
    m_Type(NO_ANIM),
    m_Speed(1),
    m_Delay(0),
    m_Loop(TRUE),
    m_State(IDLE)
{
    m_Frames.clear();
}

Animation::~Animation(void)
{
    for (uint i = 0; i < m_Frames.size(); ++i)
    {
        DeleteObject(m_Frames[i]);
    }
}

bool Animation::InitAnimation(std::string& folder, int pos_x, int pos_y, int speed, bool loop, Type type)
{
    m_PosX = pos_x;
    m_PosY = pos_y;
    m_Speed = speed;
    m_Loop = loop;
    m_Type = type;

    if (m_Type == NO_ANIM) return FALSE;

    // read all bitmaps from the animation
    std::vector<std::string> files;
    ReadFolder(folder + "\\*.bmp", files);

    for (uint j = 0; j < files.size(); ++j)
    {
        std::string filename = folder + "\\" + files[j];
        m_Frames.push_back(static_cast<HBITMAP>(
            LoadImage(NULL, filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION)));
    }

    if (!m_Frames.size()) return FALSE;

    // get metrics
    BITMAP bm;
    GetObject(m_Frames[0], sizeof(bm), &bm);
    m_Height = bm.bmHeight;
    m_Width  = bm.bmWidth;

    return TRUE;
}

void Animation::ReadFolder(std::string& folder, std::vector<std::string>& folder_content)
{
    HANDLE fHandle;
    WIN32_FIND_DATA wfd;

    folder_content.clear();

    fHandle = FindFirstFile(folder.c_str(), &wfd);
    do
    {
        if (fHandle != INVALID_HANDLE_VALUE && wfd.cFileName[0] != '.')
        {
            folder_content.push_back(wfd.cFileName);
        }
    }
    while(FindNextFile(fHandle, &wfd));

    FindClose(fHandle);
}

void Animation::UpdateAnimation(HDC* hdc)
{
    if (!m_Active) return;
    if (m_State != RUNNING) return;

    HDC hdcBuffer = CreateCompatibleDC(NULL);

    HBITMAP prevBitmap = SelectBitmap(hdcBuffer, m_Frames[m_CurrentFrame]);
    BitBlt(*hdc, m_PosX, m_PosY, m_Width, m_Height, hdcBuffer, 0, 0, SRCCOPY);
    SelectBitmap(hdcBuffer, prevBitmap);

    if (m_Speed > 0)
    {
        m_CurrentFrame += m_Speed;
    }
    else if (m_Speed < 0)
    {
        if (m_Delay == 0)
        {
            m_Delay = m_Speed;
        }

        if (++m_Delay == 0)
        {
            ++m_CurrentFrame;
        }
    }

    if (m_CurrentFrame >= m_Frames.size())
    {
        m_CurrentFrame = 0;

        if (!m_Loop)
        {
            m_State = IDLE;
        }
    }

    DeleteDC(hdcBuffer);
}

void Animation::Start(bool start)
{
    if (start)
    {
        m_State = RUNNING;
    }
    else
    {
        m_CurrentFrame = 0;
        m_Delay = 0;
        m_State = IDLE;
    }
}
