#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <oleacc.h>


HANDLE hStdin;
DWORD fdwSaveOldMode;

VOID ErrorExit(LPSTR);
VOID KeyEventProc(KEY_EVENT_RECORD);
VOID MouseEventProc(MOUSE_EVENT_RECORD);
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);
void InitializeMSAA();
void ShutdownMSAA();
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime);


// Global variable.
HWINEVENTHOOK g_hook;

int main() {
    std::cout << "Hello World!";

  
   InitializeMSAA();

   
   ShutdownMSAA();

   return 0;
}

// Initializes COM and sets up the event hook.
//
void InitializeMSAA()
{
    HRESULT hresult = CoInitialize(NULL);
    g_hook = SetWinEventHook(
        EVENT_OBJECT_DRAGENTER, EVENT_OBJECT_DRAGCOMPLETE,  // Range of events (4 to 5).
        NULL,                                          // Handle to DLL.
        HandleWinEvent,                                // The callback.
        0, 0,              // Process and thread IDs of interest (0 = all)
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS); // Flags.
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Unhooks the event and shuts down COM.
//
void ShutdownMSAA()
{
    UnhookWinEvent(g_hook);
    CoUninitialize();
}


// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime)
{
    std::cout << "handlewinevent callback";

    IAccessible* pAcc = NULL;
    VARIANT varChild;
    HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
    if ((hr == S_OK) && (pAcc != NULL))
    {
        BSTR bstrName;
        pAcc->get_accName(varChild, &bstrName);
        if (event == EVENT_OBJECT_DRAGENTER)
        {
            std::cout<<"Begin: ";
        }
        else if (event == EVENT_OBJECT_DRAGCOMPLETE)
        {
            std::cout<<"End:   ";
        }

        printf("%S\n", bstrName);
        SysFreeString(bstrName);
        pAcc->Release();
    }
}


