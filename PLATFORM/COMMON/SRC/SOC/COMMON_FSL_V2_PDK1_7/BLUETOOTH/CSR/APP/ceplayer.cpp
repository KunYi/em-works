//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
///////////////////////////////////////////////////////////////////////////////
// File: CEPlayer.cpp
//
// Desc: This file contians the WinMain, WinProc, initialization, and
//       finalization functions for the CEPlayer sample application.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>

#define _COMCTL32_
#include <commctrl.h>
#undef _COMCTL32_
#include <ole2.h>

#include "PlayerWindow.h"
#include "PropertyDlg.h"
#include "resource.h"

//////
// GUID's for the various interfaces needed by CEPlayer
//
EXTERN_C const CLSID CLSID_MediaPlayer = { 0x22D6F312,
                                           0xB0F6, 0x11D0,
                                          { 0x94, 0xAB, 0x00, 0x80,
                                            0xC7, 0x4C, 0x7E, 0x95 } };
EXTERN_C const IID   IID_IMediaPlayer  = { 0x22D6F311,
                                           0xB0F6, 0x11D0,
                                          { 0x94, 0xAB, 0x00, 0x80,
                                            0xC7, 0x4C, 0x7E, 0x95 } };
EXTERN_C const IID   DIID__MediaPlayerEvents
                                       = { 0x2D3A4C40,
                                           0xE711, 0x11D0,
                                          { 0x94, 0xAB, 0x00, 0x80,
                                            0xC7, 0x4C, 0x7E, 0x95 } };

EXTERN_C const IID   IID_IActiveMovie  = { 0x05589FA2,
                                           0xC356, 0x11CE,
                                          { 0xBF, 0x01, 0x00, 0xAA,
                                            0x00, 0x55, 0x59, 0x5A } };

//////
// Globals
//
TCHAR         *g_szWMPWindowClass  = TEXT("MPContainerWindow");
TCHAR        **g_ppszAudioTypes    = NULL;
TCHAR        **g_ppszVideoTypes    = NULL;
TCHAR        **g_ppszPlaylistTypes = NULL;
TCHAR         *g_szHomePage        = NULL;
DWORD          g_cAudioTypes       = 0;
DWORD          g_cVideoTypes       = 0;
DWORD          g_cPlaylistTypes    = 0;
CPlayerWindow *g_pPlayerWindow     = NULL;
HINSTANCE      g_hInst             = NULL;
HICON          g_hIcon             = NULL;
bool           g_bSmallScreen      = false;
bool           g_bCaptured         = true;  // don't capture until we have a window
bool           g_bInit             = false;
bool           g_bFullscreenToWindowedOnPause = false;

//////
// Prototypes
//
HRESULT WINAPI WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool           InitPlayer(HINSTANCE hInstance);
bool           FiniPlayer(HINSTANCE hInstance);
bool           LoadMediaTypes(HINSTANCE hInstance);

///////////////////////////////////////////////////////////////////////////////
// Name: WinProc()
// Desc: This function handles the Window Messages for the main CEPlayer
//       window.
///////////////////////////////////////////////////////////////////////////////
int broadcastmsg;

#define AUDIO_REMOTE                TEXT("AUDIO_REMOTE")

#define AUDIO_REMOTE_MEDIA_PLAYER   TEXT("AUDIO_REMOTE_MEDIA_PLAYER")

#define AUDIO_REMOTE_PLAY           (0)
#define AUDIO_REMOTE_STOP           (1)
#define AUDIO_REMOTE_NEXT           (2)
#define AUDIO_REMOTE_PREV           (3)
#define AUDIO_REMOTE_FASTFORWARD    (4)
#define AUDIO_REMOTE_FASTREVERSE    (5)
#define AUDIO_REMOTE_PAUSE          (6)
#define AUDIO_REMOTE_REGISTER       (7)
#define AUDIO_REMOTE_UNREGISTER     (8)

HRESULT WINAPI WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hDC;

   /* Code added for CSR WPP AVRCP support*/
   if (msg == broadcastmsg && NULL != g_pPlayerWindow)
   {
//      printf ("Received AVRCP Broadcast message of type %d\n", wParam);
      
      switch(wParam)
      {
         case AUDIO_REMOTE_STOP:
            g_pPlayerWindow->OnCommand(ID_PLAYBACK_STOP, lParam);
            return 0;
            
         case AUDIO_REMOTE_PAUSE:
            g_pPlayerWindow->OnCommand(ID_PLAYBACK_PAUSE, lParam);
            return 0;
            
         case AUDIO_REMOTE_PLAY:
            g_pPlayerWindow->OnCommand(ID_PLAYBACK_PLAY, lParam);
            return 0;
            
         default:
            return 0;
      }
   }
   
   switch (msg)
   {
    case WM_GETICON:
        if( ( wParam == ICON_SMALL ) && g_hIcon )
        {
            return (HRESULT)(g_hIcon);
        }
    break;

    case WM_CREATE:
    if (NULL == g_pPlayerWindow)
    {
      g_pPlayerWindow = new CPlayerWindow(hWnd, g_hInst);

      return 0;
    }
    break;

    case WM_ACTIVATE:
      //  Incase Seek or Volume button are still down and window is becoming inactive
      //  Revert them back to the UP position 
      if (NULL != g_pPlayerWindow && WA_INACTIVE==LOWORD(wParam))
      {
            g_pPlayerWindow->OnCursorFocusChange();
      }

      // The constructor doesn't do much work, the Init method is what
      // creates a control container, Event Sink, etc (because it can
      // return a result)
      // The Init method should only be called once, so check a flag first
      if (!g_bInit
          && (NULL == g_pPlayerWindow || false == g_pPlayerWindow->Init()))
      {
          // Must set the init flag because the message box generates a
          // WM_ACTIVATE message.
          g_bInit = true;

         ::MessageBox(hWnd, TEXT("Unable to initialize Player!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL);

         delete g_pPlayerWindow;
         g_pPlayerWindow = NULL;

         ::PostQuitMessage(0);
      }
      g_bInit = true;
      return 0;

    case WM_KEYUP:
      if (g_pPlayerWindow && (true == g_pPlayerWindow->OnWindowMessage(msg, wParam, lParam)))
      {
         return 0;
      }

      break;

    case WM_COMMAND:
      if (g_pPlayerWindow->OnCommand(LOWORD(wParam), lParam))
      {
         return 0;
      }
      break;

    case WM_COPYDATA:
      if( lParam )
      {
          PCOPYDATASTRUCT lpCDS = (PCOPYDATASTRUCT)lParam;
          if( g_pPlayerWindow )
              g_pPlayerWindow->OnOpen((TCHAR *)(lpCDS->lpData));
      }
      return 0;

    case WM_CLOSE:
      // Tell the player window to shut down (hide itself and stop playback)
      if (NULL != g_pPlayerWindow)
      {
         delete g_pPlayerWindow;
         g_pPlayerWindow = NULL;
      }

      ::PostQuitMessage(0);
      return 0;

    case WM_PAINT:

      if (!wParam)
      {
         hDC = BeginPaint(hWnd, &ps);
      }
      else
      {
         hDC = (HDC)wParam;
      }

      if (NULL != g_pPlayerWindow)
      {
         g_pPlayerWindow->OnPaint(hDC, &(ps.rcPaint));
      }

      if (!wParam)
      {
         EndPaint(hWnd, &ps);
      }
      return 0;

    case WM_SIZE:
      if (NULL != g_pPlayerWindow)
      {
         if (g_bSmallScreen)
         {
             g_pPlayerWindow->OnSizeSmall(LOWORD(lParam), HIWORD(lParam));
         }
         else
         {
             g_pPlayerWindow->OnSize(LOWORD(lParam), HIWORD(lParam));
         }
      }
      return 0;

    case WM_MOUSEMOVE:
      if (NULL != g_pPlayerWindow)
      {
         //  Incase Seek or Volume button are still down and window is getting mouse focus
         //  without the LBUTTON down, revert the button states back to the UP position 
         if ( MK_LBUTTON != wParam )
         {
         g_pPlayerWindow->OnCursorFocusChange();
     }


         g_pPlayerWindow->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
      }
      break;

    case WM_LBUTTONDOWN:
      if (NULL != g_pPlayerWindow)
      {
         g_pPlayerWindow->OnMouseDown(LOWORD(lParam), HIWORD(lParam));
      }
      break;
 
    case WM_LBUTTONUP:
      if (NULL != g_pPlayerWindow)
      {
         g_pPlayerWindow->OnMouseUp(LOWORD(lParam), HIWORD(lParam));
      }
      break;

    case WM_TIMER:
      if (NULL != g_pPlayerWindow)
      {
         if (g_pPlayerWindow->OnTimer((UINT)wParam))
         {
             return 0;
         }
      }
      break;

    case WM_MEASUREITEM:
      if (NULL != g_pPlayerWindow && 0 == wParam)
      {
          g_pPlayerWindow->OnMeasureItem((MEASUREITEMSTRUCT*)lParam);
          return 0;
      }
      break;

    case WM_DRAWITEM:
      if (NULL != g_pPlayerWindow && 0 == wParam)
      {
          g_pPlayerWindow->OnDrawItem((DRAWITEMSTRUCT*)lParam);
          return 0;
      }
      break;

    // This message is a user defined window message for the Properties
    // dialog box.  (The message is sent when the dialog closes)
    case PD_CLOSED:
      if (NULL != g_pPlayerWindow)
      {
         g_pPlayerWindow->PropertyDlgClosed();
      }
      return 0;

    // This message is a user defined window message for the Statistics
    // dialog box.  (The message is sent when the dialog closes)
    case SD_CLOSED:
      if (NULL != g_pPlayerWindow)
      {
         g_pPlayerWindow->StatisticsDlgClosed();
      }
      return 0;
   }

   // redirect all other messages to the MediaPlayer control
   if (NULL != g_pPlayerWindow
       && true == g_pPlayerWindow->OnWindowMessage(msg, wParam, lParam))
   {
      return 0;
   }

   return DefWindowProc(hWnd, msg, wParam, lParam);
}


///////////////////////////////////////////////////////////////////////////////
// Name: WinMain()
// Desc: This function is the entry point for the CEPlayer application
///////////////////////////////////////////////////////////////////////////////
#ifdef UNDER_CE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPTSTR szCmdLine, int iCmdShow)
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR pCmdLine, int iCmdShow)
#endif
{
   int  iXPos, iYPos;
   HWND hWnd;
   HACCEL hAccel;
   MSG  msg;
   HKEY hkResult = NULL;
   DWORD dwDisp, dwType, dwData, dwSize;
   BOOL bOpenMultiple = FALSE;
   BOOL bPlayForever = FALSE;
   DWORD dwZoomLevel = 1; // Default to 100% (0 is 50%, 2 is 200%)

#ifndef UNDER_CE
   //GetCommandLine();
   WCHAR szCmdLine[] = L"\0";
#endif

   srand( GetTickCount() ); // Do this so that shuffle mode of playlist content is actually random...

   LoadMediaTypes(hInstance);

   HDC hdc = ::GetDC(NULL);
   int iXRes = GetDeviceCaps(hdc, HORZRES);
   int iYRes = GetDeviceCaps(hdc, VERTRES);
   ::ReleaseDC(NULL, hdc);

   if (iXRes < 480 || iYRes < 480)
   {
       g_bSmallScreen = true;
   }

   if (iXRes < 200 || iYRes < 120)
   {
      MessageBox(NULL, TEXT("The Media Player cannot function properly on a display this size."), TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);

      return 0;
   }

   // Initialize the Player.  If it fails, there's not much point in going on.
   if (false == InitPlayer(hInstance))
   {
      MessageBox(NULL, TEXT("Unable to initialize Player!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);

      FiniPlayer(hInstance);

      return 0;
   }

   g_hInst = hInstance;

   g_hIcon = (HICON)LoadImage( g_hInst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, 0 );

   // Get the OpenMultiple setting from the registry
   if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                       TEXT("SOFTWARE\\Microsoft\\CEPlayer"),
                                       0, NULL, 0, 0, NULL, &hkResult, &dwDisp))
   {
      g_szHomePage = new TCHAR [1024];
      dwSize = 1024 * sizeof (TCHAR);

      RegQueryValueEx(hkResult,
                      TEXT("HomePage"),
                      NULL,
                      &dwType,
                      (BYTE*)g_szHomePage,
                      &dwSize);

      if (REG_SZ != dwType || 0 == dwSize)
      {
          delete [] g_szHomePage;
          g_szHomePage = NULL;
      }

      if( REG_OPENED_EXISTING_KEY == dwDisp )
      {
         dwSize = sizeof( bool );
         RegQueryValueEx( hkResult, L"WindowedOnPause", NULL, NULL, (LPBYTE)&g_bFullscreenToWindowedOnPause, &dwSize );
         // Don't overwrite our g_bSmallScreen variable if its already set (due to screen-size dimensions)
         if( !g_bSmallScreen )
            RegQueryValueEx( hkResult, L"AlwaysFullSize", NULL, NULL, (LPBYTE)&g_bSmallScreen, &dwSize );
         RegQueryValueEx( hkResult, L"OpenMultiple", NULL, NULL, (LPBYTE)&bOpenMultiple, &dwSize );
      }
   }

   if( !bOpenMultiple )
   {
       hWnd = FindWindow(g_szWMPWindowClass, NULL); 
       if (hWnd) 
       {
           //If it is already running, then focus on the window
           SetForegroundWindow ((HWND) (((DWORD)hWnd) | 0x01));
           if( *szCmdLine )
           {
               COPYDATASTRUCT cds = { 1, 2*(wcslen( szCmdLine ) + 1), szCmdLine };
               SendMessage( hWnd, WM_COPYDATA, NULL, (LPARAM)&cds );
           }
           RegCloseKey( hkResult );
           return 0;
       } 
   }
    
   // Initialize the window position to be the upper left corner of the screen
   iXPos = 0;
   iYPos = 0;

   // Get the last position from the registry
   if( hkResult )
   {
      if (REG_OPENED_EXISTING_KEY == dwDisp)
      {
         dwSize = sizeof (DWORD);

         if (ERROR_SUCCESS == RegQueryValueEx(hkResult,
                                              TEXT("YPos"), NULL,
                                              &dwType, (BYTE*)&dwData,
                                              &dwSize))
         {
           if (REG_DWORD == dwType)
           {
              iYPos = (int)dwData;
           }
         }

         dwSize = sizeof (DWORD);

         if (ERROR_SUCCESS == RegQueryValueEx(hkResult,
                                              TEXT("XPos"), NULL,
                                              &dwType, (BYTE*)&dwData,
                                              &dwSize))
         {
           if (REG_DWORD == dwType)
           {
              iXPos = (int)dwData;
           }
         }

      }
      RegCloseKey( hkResult );
   }

   // Load the accelerator table
   hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE (ID_ACCEL));

   // create a window for the player
   DWORD dwExStyle, dwStyle;
   int   iWidth, iHeight;

   if (g_bSmallScreen)
   {
       RECT rcWorkArea;

       dwExStyle = 0;
       dwStyle   = WS_VISIBLE;

       if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0))
       {
           iWidth  = rcWorkArea.right - rcWorkArea.left;
           iHeight = rcWorkArea.bottom - rcWorkArea.top;
       }
       else
       {
           hdc = ::GetDC(NULL);

           iWidth  = GetDeviceCaps(hdc, HORZRES);
           iHeight = GetDeviceCaps(hdc, VERTRES) - GetSystemMetrics(SM_CYMENU);

           ::ReleaseDC(NULL, hdc);
       }
   }
   else
   {
       dwExStyle = WS_EX_WINDOWEDGE;
       dwStyle   = WS_CAPTION | WS_SIZEBOX | WS_SYSMENU; //| WS_VISIBLE;
       iWidth    = CW_USEDEFAULT;
       iHeight   = CW_USEDEFAULT;
   }

   hWnd = CreateWindowEx(dwExStyle,
                         g_szWMPWindowClass,
                         TEXT("Media Player"),
                         dwStyle,
                         iXPos,
                         iYPos,
                         iWidth,
                         iHeight,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

   if (NULL != g_pPlayerWindow)
   {
      g_pPlayerWindow->Show(iCmdShow);
   }

   // If we have a command line, open it immediately
   if (NULL != g_pPlayerWindow
       && 0 < _tcslen(szCmdLine))
   {
      g_pPlayerWindow->OnOpen(szCmdLine);
   }

   // Event loop...  wait for a close
   while (GetMessage(&msg, NULL, 0, 0))
   {
      //
      // First give any modeless dialogs a chance at the message...
      //

      if (NULL == g_pPlayerWindow || !g_pPlayerWindow->DialogMessage(&msg))
      {
          // Translate accelerators
          // Okay, this is a bit nasty, but the general idea is to first get any
          // app specific accelerator keys.  If there aren't any, try to get any
          // control specific acclerators.  If there aren't any there, just
          // process the message
          if (!TranslateAccelerator (hWnd, hAccel, &msg)
              && (NULL == g_pPlayerWindow
                  || !g_pPlayerWindow->TranslateAccelerator(&msg)))
          {
              TranslateMessage(&msg);
              DispatchMessage(&msg);
          }
      }
   }

   // Perform shutdown on the player
   FiniPlayer(hInstance);
   DestroyWindow(hWnd);

   return msg.wParam;
}

///////////////////////////////////////////////////////////////////////////////
// Name: InitPlayer()
// Desc: InitPlayer is used to initialize COM, OLE, and the window class for
//       the CEPlayer application
///////////////////////////////////////////////////////////////////////////////
bool InitPlayer(HINSTANCE hInstance)
{
   INITCOMMONCONTROLSEX iccEx;
   WNDCLASS             wc;
   bool                 bResult = true;

   // Initialize COM/OLE (Under CE, it's done via CoInitializeEx)
#ifndef UNDER_CE
   if (FAILED(CoInitialize(NULL)))
#else
   if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
#endif // !UNDER_CE
   {
      bResult = false;
   }

   // Initialize the common control DLL
   memset(&iccEx, 0, sizeof (iccEx));
   iccEx.dwSize = sizeof (iccEx);
   iccEx.dwICC = ICC_BAR_CLASSES | ICC_UPDOWN_CLASS | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES;

   if (FALSE == InitCommonControlsEx(&iccEx))
   {
      bResult = false;
   }

   broadcastmsg = RegisterWindowMessage(AUDIO_REMOTE);
   CreateEvent(NULL, TRUE, TRUE, AUDIO_REMOTE_MEDIA_PLAYER);
   
   // create and register a new window class
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = WinProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = hInstance;
   wc.hIcon         = NULL; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)); This doesn't work properly.  Use LoadImage into g_hImage instead.
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
#ifndef UNDER_CE
   wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENUBAR),
#else
   wc.lpszMenuName  = NULL;
#endif // !UNDER_CE
   wc.lpszClassName = g_szWMPWindowClass;
   RegisterClass(&wc);

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: FiniPlayer()
// Desc: This function is used to shutdown the main window, and COM
///////////////////////////////////////////////////////////////////////////////
bool FiniPlayer(HINSTANCE hInstance)
{
   bool bResult = true;

   if (NULL != g_pPlayerWindow)
   {
      delete g_pPlayerWindow;
      g_pPlayerWindow = NULL;
   }

   UnregisterClass(g_szWMPWindowClass, hInstance);

   CoUninitialize();

   DestroyIcon( g_hIcon );

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: LoadMediaTypes()
// Desc: This function queries the registry for the various media types.
///////////////////////////////////////////////////////////////////////////////
bool LoadMediaTypes(HINSTANCE hInstance)
{
    DWORD dwIndex        = 0;
    DWORD dwType, i;
    DWORD cbName         = MAX_PATH;
    DWORD cbData         = MAX_PATH;
    DWORD cAudioTypes    = 0;
    DWORD cVideoTypes    = 0;
    DWORD cPlaylistTypes = 0;
    TCHAR szName[MAX_PATH];
    TCHAR szData[MAX_PATH];
    TCHAR **ppszBuffer   = NULL;
    TCHAR **ppszTemp     = NULL;
    HKEY  hKey;
    bool  bResult        = false;

    while (ERROR_SUCCESS == RegEnumKeyEx(HKEY_CLASSES_ROOT,
                                         dwIndex,
                                         szName,
                                         &cbName,
                                         NULL,
                                         szData,
                                         &cbData,
                                         NULL))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT,
                                          szName,
                                          0, 0,
                                          &hKey))
        {

            cbData = MAX_PATH;

            if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                 NULL,
                                                 NULL,
                                                 &dwType,
                                                 (BYTE*)szData,
                                                 &cbData)
                && REG_SZ == dwType)
            {
                if (cbData > _tcslen(TEXT("audiofile"))
                    && 0 == _tcscmp(szData, TEXT("audiofile")))
                {
                    cAudioTypes++;

                    ppszBuffer = new TCHAR * [cAudioTypes];

                    if (NULL != ppszBuffer)
                    {
                        for (i = 0; i < (cAudioTypes - 1); i++)
                        {
                            ppszBuffer[i] = g_ppszAudioTypes[i];
                        }

                        size_t BufLen = _tcslen(szName) + 1;
                        ppszBuffer[i] = new TCHAR [BufLen];

                        if (NULL != ppszBuffer[i])
                        {
                            StringCchCopy(ppszBuffer[i], BufLen, szName);
                            g_cAudioTypes++;

                            ppszTemp         = g_ppszAudioTypes;
                            g_ppszAudioTypes = ppszBuffer;
                            ppszBuffer       = ppszTemp;
                        }

                        delete [] ppszBuffer;
                    }
                }
                else if (cbData > _tcslen(TEXT("videofile"))
                         && 0 == _tcscmp(szData, TEXT("videofile")))
                {
                    cVideoTypes++;

                    ppszBuffer = new TCHAR * [cVideoTypes];

                    if (NULL != ppszBuffer)
                    {
                        for (i = 0; i < (cVideoTypes - 1); i++)
                        {
                            ppszBuffer[i] = g_ppszVideoTypes[i];
                        }

                        size_t BufLen = _tcslen(szName) + 1;
                        ppszBuffer[i] = new TCHAR [BufLen];

                        if (NULL != ppszBuffer[i])
                        {
                            StringCchCopy(ppszBuffer[i], BufLen, szName);
                            g_cVideoTypes++;

                            ppszTemp         = g_ppszVideoTypes;
                            g_ppszVideoTypes = ppszBuffer;
                            ppszBuffer       = ppszTemp;
                        }

                        delete [] ppszBuffer;
                    }
                }
                else if (cbData > _tcslen(TEXT("playlist"))
                         && 0 == _tcscmp(szData, TEXT("playlist")))
                {
                    cPlaylistTypes++;

                    ppszBuffer = new TCHAR * [cPlaylistTypes];

                    if (NULL != ppszBuffer)
                    {
                        for (i = 0; i < (cPlaylistTypes - 1); i++)
                        {
                            ppszBuffer[i] = g_ppszPlaylistTypes[i];
                        }

                        size_t BufLen = _tcslen(szName) + 1;
                        ppszBuffer[i] = new TCHAR [BufLen];

                        if (NULL != ppszBuffer[i])
                        {
                            StringCchCopy(ppszBuffer[i], BufLen, szName);
                            g_cPlaylistTypes++;

                            ppszTemp            = g_ppszPlaylistTypes;
                            g_ppszPlaylistTypes = ppszBuffer;
                            ppszBuffer          = ppszTemp;
                        }

                        delete [] ppszBuffer;
                    }
                }
            }

            RegCloseKey(hKey);
        }

        // Try the next key.
        dwIndex++;
        cbName    = MAX_PATH;
        cbData    = MAX_PATH;
        szName[0] = TEXT('\0');
        szData[0] = TEXT('\0');
    }

    return bResult;
}
