#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <oleacc.h>
#include <comdef.h>
#include <psapi.h> 
#include <thread>
#define MAX_PROCESSES 1024 

void InitializeMSAA();
DWORD GetThreadIDOfListViewControlOwner();
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime);
DWORD FindProcess(__in_z LPCTSTR lpcszFileName);


// Global variable.
HWINEVENTHOOK g_hook;

int main() {
    std::cout << "Hello World!";
    std::thread t1(InitializeMSAA);
    
    //InitializeMSAA();
    t1.join();
   

    return 0;
}

// Initializes COM and sets up the event hook.
//
void InitializeMSAA()
{
    DWORD pid = FindProcess("explorer.exe");
    DWORD threadId= GetThreadIDOfListViewControlOwner();

    HRESULT hresult = CoInitialize(NULL);
    g_hook = SetWinEventHook(
        EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_DRAGCOMPLETE,
        NULL,                                          // Handle to DLL.
        HandleWinEvent,                                // The callback.
        pid, threadId,              // Process and thread IDs of interest (0 = all)
        WINEVENT_OUTOFCONTEXT); // Flags.
        MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

DWORD GetThreadIDOfListViewControlOwner()
{
    DWORD threadId;
    HWND hWnd = FindWindow(_T("Progman"), _T("Program Manager"));
    HWND hChildWnd = FindWindowEx(hWnd, NULL, _T("SHELLDLL_DefView"), NULL);
    HWND hDesktopWnd = FindWindowEx(hChildWnd, NULL, _T("SysListView32"), NULL);

    threadId = GetWindowThreadProcessId(hDesktopWnd, NULL);
    return threadId;
}


// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime)
{
    //std::cout << "handlewinevent callback";

    IAccessible* pAcc = NULL;
    VARIANT varChild;
    HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
    if ((hr == S_OK) && (pAcc != NULL))
    {
        BSTR bstrName;
        pAcc->get_accName(varChild, &bstrName);
        if (event == EVENT_OBJECT_LOCATIONCHANGE)
        {
            //std::cout<<"Begin: EVENT_OBJECT_LOCATIONCHANGE " << bstrName;
            std::wstring ws(bstrName, SysStringLen(bstrName));
            if (!ws.compare(L"Drag"))
            {
                std::cout << "object dragged ";
            }
           /* printf("%S\n", bstrName);*/
           
        }
            
        SysFreeString(bstrName);
        pAcc->Release();
    }
}

DWORD FindProcess(__in_z LPCTSTR lpcszFileName)
{
    LPDWORD lpdwProcessIds;
    LPTSTR  lpszBaseName;
    HANDLE  hProcess;
    DWORD   i, cdwProcesses, dwProcessId = 0;

    lpdwProcessIds = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, MAX_PROCESSES * sizeof(DWORD));
    if (lpdwProcessIds != NULL)
    {
        if (EnumProcesses(lpdwProcessIds, MAX_PROCESSES * sizeof(DWORD), &cdwProcesses))
        {
            lpszBaseName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(TCHAR));
            if (lpszBaseName != NULL)
            {
                cdwProcesses /= sizeof(DWORD);
                for (i = 0; i < cdwProcesses; i++)
                {
                    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[i]);
                    if (hProcess != NULL)
                    {
                        if (GetModuleBaseName(hProcess, NULL, lpszBaseName, MAX_PATH) > 0)
                        {
                            if (!lstrcmpi(lpszBaseName, lpcszFileName))
                            {
                                dwProcessId = lpdwProcessIds[i];
                                CloseHandle(hProcess);
                                break;
                            }
                        }
                        CloseHandle(hProcess);
                    }
                }
                HeapFree(GetProcessHeap(), 0, (LPVOID)lpszBaseName);
            }
        }
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpdwProcessIds);
    }
    return dwProcessId;
}



