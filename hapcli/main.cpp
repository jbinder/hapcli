//*****************************************************************************
// hapcli
// ----------------------------------------------------------------------------
// file:   main.cpp
// ----------------------------------------------------------------------------
// author: j.binder.x@gmail.com
// create: 2007-04-27
// -----------------------------------------------
// tags:   TEST: only testing purposes
//         TODO: work in progress
//         FIXME: bug?
//*****************************************************************************

#include "../wlib/info_window.h"
#include <windows.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <time.h>

// config
const std::string settings_filename = "hapcli.ini";
std::string cfg_name = "app";
std::string cfg_version = "1.00";
std::string cfg_text_db = "messages.txt";
std::string cfg_info_text = "app info";
std::string cfg_info_head = "about this app";
std::string cfg_background_image = "background.bmp";
std::string cfg_icon = "icon.ico";
int cfg_border_top = 10;
int cfg_border_right = 10;
int cfg_border_bottom = 10;
int cfg_border_left = 10;
int cfg_anim_interval = 300;
bool cfg_anim_active = TRUE;
int cfg_auto_hide_timeout = 1000;
int cfg_min_time_for_one_frame = 15;
int cfg_max_time_for_one_frame = 25;

// var
std::vector<std::string> texts;
static const int TIMER_ID = 23;
HINSTANCE* main_instance;
std::vector<InfoWindow*> info_windows;
RECT window_rect;

// var: tray icon
static const int ICON_ID = 23;
static const int WM_ICONCLICK = (WM_APP + 0);
HICON hIcon;

// var: context menu
HMENU hMenu;
enum MenuItems
{
    MI_CANCEL,
    MI_ANIMATE,
    MI_SINGLE,
    MI_INFO,
    MI_QUIT
};

// function declaration
HWND Main_CreateWindow(HINSTANCE);
void Main_RegisterClass(HINSTANCE);
LRESULT CALLBACK Main_WindowProc(HWND, UINT, WPARAM, LPARAM);
void Main_OnCreate(HWND);
void Main_OnDestroy(HWND);
bool Main_ReadTextFile(void);
void Main_ShowTrayIcon(HWND, bool);
void Main_ShowContextMenu(HWND, LPARAM);
void Main_ShowInfoWindow(HWND, bool);
bool Main_ReadSettings(void);
bool Main_WriteSettings(void);
std::string Main_GetIniFilename(void);
void Main_ClearInfoWindows(bool force);

//-----------------------------------------------------------------------------
int WINAPI WinMain(
  HINSTANCE hInstance,
  HINSTANCE hInstPrev,
  LPSTR lpszCmdLine,
  int nCmdShow)
{
    Main_ReadSettings();
    if (!Main_ReadTextFile())
    {
        MessageBox(NULL, ("File \"" + cfg_text_db + "\" not found.").c_str(), "Error", MB_ICONERROR);
        return 1;
    }
    Main_RegisterClass(hInstance);
    HWND hWnd = Main_CreateWindow(hInstance);
    main_instance = &hInstance;
    GetWindowRect(GetDesktopWindow(), &window_rect);
    srand((unsigned)time(NULL));

    if (cfg_anim_active)
    {
        SetTimer(hWnd, TIMER_ID, cfg_anim_interval, NULL);
    }

    MSG msg;
    while(GetMessage(&msg,0,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Main_WriteSettings();
    Main_ClearInfoWindows(true);
    Main_ShowTrayIcon(hWnd, false);

    #ifdef _DEBUG
      _CrtDumpMemoryLeaks();
    #endif

    return static_cast<int>(msg.wParam);
}

//-----------------------------------------------------------------------------
void Main_RegisterClass(HINSTANCE hInstance)
{
    WNDCLASS wc = {
        CS_HREDRAW | CS_VREDRAW,   // redraw window on resize
        Main_WindowProc,           // msg handler
        0,                         // additional memory
        0,                         // additional memory
        hInstance,
        LoadIcon(NULL,IDI_APPLICATION),
        LoadCursor(NULL,IDC_ARROW),
        (HBRUSH)GetStockObject(BLACK_BRUSH),
        NULL,
        cfg_name.c_str()
    };

    RegisterClass(&wc);
}

//-----------------------------------------------------------------------------
HWND Main_CreateWindow(HINSTANCE hInstance)
{
    HWND hWnd = CreateWindow(
        cfg_name.c_str(),
        cfg_name.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,           // x position
        CW_USEDEFAULT,           // y position
        0,         // width
        0,         // height
        NULL,
        NULL,
        hInstance,
        NULL);

    return hWnd;
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK Main_WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    static UINT s_uTaskbarRestart;

    switch(uMsg)
    {
    case WM_CREATE:
        s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated")); // tray icon
        Main_OnCreate(hwnd);
        return 0;

    case WM_DESTROY:
        Main_OnDestroy(hwnd);
        return 0;

    case WM_KEYDOWN:
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            Main_ShowInfoWindow(hwnd, FALSE);
        }
        break;

    case WM_ICONCLICK:
        switch (lParam)
        {
        case WM_RBUTTONUP:
            Main_ShowContextMenu(hwnd, lParam);
            break;

        case WM_LBUTTONUP:
            Main_ShowInfoWindow(hwnd, FALSE);
            break;
        }
        return 0;

    default:
        if (uMsg == s_uTaskbarRestart)
        {
            Main_ShowTrayIcon(hwnd, TRUE);
        }
        break;
    }

    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

//-----------------------------------------------------------------------------
void Main_OnCreate(HWND hwnd)
{
    // tray icon
    hIcon = (HICON)LoadImage(NULL, cfg_icon.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_LOADTRANSPARENT);
    Main_ShowTrayIcon(hwnd, TRUE);

    // context menu
    hMenu = CreatePopupMenu();
    InsertMenu(hMenu, -1, MF_BYPOSITION, MI_SINGLE, "single");
    InsertMenu(hMenu, -1, MF_BYPOSITION | ((cfg_anim_active) ? MF_CHECKED : 0), MI_ANIMATE, "animate");
    InsertMenu(hMenu, -1, MF_BYPOSITION, MI_INFO, "Info");
    InsertMenu(hMenu, -1, MF_SEPARATOR, MI_CANCEL, NULL);
    InsertMenu(hMenu, -1, MF_BYPOSITION, MI_QUIT, "Quit");
}

//-----------------------------------------------------------------------------
void Main_OnDestroy(HWND hwnd)
{
    DestroyIcon(hIcon);
    DestroyMenu(hMenu);
    PostQuitMessage(0);
}

//-----------------------------------------------------------------------------
bool Main_ReadTextFile(void)
{
    texts.clear();
    std::ifstream file(cfg_text_db.c_str());
    std::string str;
    if (!file.is_open())
    {
        return FALSE;
    }
    while (file.good() && std::getline(file, str))
    {
         if (!str.empty() && str[0] != '\'')
         {
             texts.push_back(str);
         }
    }
    file.close();

    return TRUE;
}

//-----------------------------------------------------------------------------
void Main_ShowTrayIcon(HWND hwnd, bool show)
{
    NOTIFYICONDATA data;
    data.cbSize = sizeof(NOTIFYICONDATA);
    data.hWnd = hwnd;
    data.uID = ICON_ID;

    if (show)
    {
        data.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        data.hIcon = hIcon;
        strcpy_s(data.szTip, cfg_name.c_str());
        data.uCallbackMessage = WM_ICONCLICK;

        Shell_NotifyIcon(NIM_ADD, &data);
    }
    else
    {
        data.uFlags = 0;

        Shell_NotifyIcon(NIM_DELETE, &data);
    }
}

//-----------------------------------------------------------------------------
void Main_ShowContextMenu(HWND hwnd, LPARAM lParam)
{
    POINT point;
    int   iAuswahl;

    GetCursorPos(&point);

    SetForegroundWindow(hwnd);
    iAuswahl = (int) TrackPopupMenu(hMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, 0, hwnd, 0);
    PostMessage(hwnd, WM_NULL, 0, 0);

    int show = SW_SHOWNOACTIVATE;
    int checked = MF_CHECKED;

    switch (iAuswahl)
    {
    case MI_QUIT:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;

    case MI_INFO:
        MessageBox(hwnd, (cfg_info_text + "\nv" + cfg_version).c_str(), cfg_info_head.c_str(), MB_ICONINFORMATION);
        break;

    case MI_SINGLE:
        Main_ShowInfoWindow(hwnd, FALSE);
        break;

    case MI_ANIMATE:
        if (cfg_anim_active)
        {
            KillTimer(hwnd, TIMER_ID);
            checked = MF_UNCHECKED;
        }
        else
        {
            SetTimer(hwnd, TIMER_ID, cfg_anim_interval, NULL);
        }
        cfg_anim_active = !cfg_anim_active;
        ModifyMenu(hMenu, MI_ANIMATE, MF_BYPOSITION | checked, MI_ANIMATE, "animate");
        break;

    case MI_CANCEL:
        break;
    }
}

//-----------------------------------------------------------------------------
void Main_ShowInfoWindow(HWND hwnd, bool force)
{
    int cur_text = rand() % static_cast<int>(texts.size());
    InfoWindow* n = new InfoWindow();
    n->WL_SetTextBorder(InfoWindow::TextBorder(cfg_border_left, cfg_border_right, cfg_border_top, cfg_border_bottom));
    n->WL_PrepareWindow(*main_instance, cfg_background_image);
    n->WL_SetText(texts[cur_text]);
    n->WL_SetPosition(rand() % window_rect.right, rand() % window_rect.bottom);
    n->WL_SetTimeForOneFrame(cfg_min_time_for_one_frame + rand() % (cfg_max_time_for_one_frame - cfg_min_time_for_one_frame));
    n->WL_SetAutoHideTimeout(cfg_auto_hide_timeout);
    n->WL_FadeIn(TRUE);
    info_windows.push_back(n);
    Main_ClearInfoWindows(false);
}

void Main_ClearInfoWindows(bool force)
{
    std::vector<InfoWindow*>::iterator iter = info_windows.begin();
    while (iter != info_windows.end())
    {
        if ((*iter)->WL_IsAnimating() && !force)
        {
            ++iter;
        }
        else
        {
            (*iter)->WL_DestroyWindow();
            delete (*iter);
            iter = info_windows.erase(iter);
        }
    }
}

std::string Main_GetPrivateProfileString(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPCSTR lpFileName)
{
    char tmp[2000];
    GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, tmp, sizeof(tmp), lpFileName);
    return tmp;
}

//-----------------------------------------------------------------------------
bool Main_ReadSettings(void)
{
    std::string ini_file = Main_GetIniFilename();

    std::string cur_section = "common";
    cfg_name = Main_GetPrivateProfileString(cur_section.c_str(), "Name", cfg_name.c_str(), ini_file.c_str());
    cfg_version = Main_GetPrivateProfileString(cur_section.c_str(), "Version", cfg_version.c_str(), ini_file.c_str());
    cfg_text_db = Main_GetPrivateProfileString(cur_section.c_str(), "TextDB", cfg_text_db.c_str(), ini_file.c_str());
    cfg_info_text = Main_GetPrivateProfileString(cur_section.c_str(), "InfoText", cfg_info_text.c_str(), ini_file.c_str());
    cfg_info_head =  Main_GetPrivateProfileString(cur_section.c_str(), "InfoHead", cfg_info_head.c_str(), ini_file.c_str());
    cfg_background_image = Main_GetPrivateProfileString(cur_section.c_str(), "BackgroundImage", cfg_background_image.c_str(), ini_file.c_str());
    cfg_icon = Main_GetPrivateProfileString(cur_section.c_str(), "Icon", cfg_icon.c_str(), ini_file.c_str());

    cur_section = "texts";
    cfg_border_top = GetPrivateProfileInt(cur_section.c_str(), "BorderTop", cfg_border_top, ini_file.c_str());
    cfg_border_left = GetPrivateProfileInt(cur_section.c_str(), "BorderLeft", cfg_border_top, ini_file.c_str());
    cfg_border_bottom = GetPrivateProfileInt(cur_section.c_str(), "BorderBottom", cfg_border_bottom, ini_file.c_str());
    cfg_border_right = GetPrivateProfileInt(cur_section.c_str(), "BorderRight", cfg_border_right, ini_file.c_str());

    cur_section = "anim";
    cfg_anim_interval = GetPrivateProfileInt(cur_section.c_str(), "Interval", cfg_anim_interval, ini_file.c_str());
    cfg_anim_active = GetPrivateProfileInt(cur_section.c_str(), "Active", cfg_anim_active, ini_file.c_str()) == 1 ? TRUE : FALSE;
    cfg_auto_hide_timeout = GetPrivateProfileInt(cur_section.c_str(), "AutoHideTimeout", cfg_auto_hide_timeout, ini_file.c_str());
    cfg_min_time_for_one_frame = GetPrivateProfileInt(cur_section.c_str(), "MinTimeForOneFrame", cfg_min_time_for_one_frame, ini_file.c_str());
    cfg_max_time_for_one_frame = GetPrivateProfileInt(cur_section.c_str(), "MaxTimeForOneFrame", cfg_max_time_for_one_frame, ini_file.c_str());

    return TRUE;
}

//-----------------------------------------------------------------------------
bool Main_WriteSettings(void)
{
    std::string ini_file = Main_GetIniFilename();

    std::string cur_section = "anim";
    WritePrivateProfileString(cur_section.c_str(), "Active", (cfg_anim_active ? "1" : "0"), ini_file.c_str());

    return TRUE;
}

//-----------------------------------------------------------------------------
std::string Main_GetIniFilename(void)
{
    TCHAR szDirectory[MAX_PATH] = "";
    if (!::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory))
    {
        // Error -> call '::GetLastError()'
    }

    std::string ini_file = szDirectory;
    ini_file += "\\" + settings_filename;

    return ini_file;
}
