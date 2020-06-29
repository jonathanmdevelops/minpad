#include "MinPad.h"
#include "framework.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;
WCHAR szTitle[] = L"MinPad";
WCHAR szWindowClass[] = L"ParentClass";
WCHAR szEditWindowClass[] = L"Edit";
HRESULT hr;

// Handles to parent and edit Windows.
HWND g_HWndParent;
HWND g_HWndEdit;
HFONT g_hFont;

BOOL unsavedChanges = FALSE;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                ProcessWindowPositions();
void                ProcessOpenFile();
void                LoadFileIntoMinPad(PWSTR pszFilePath);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MINPAD, szWindowClass, MAX_LOADSTRING);

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINPAD));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINPAD);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINPAD));

    MSG msg;

    // Main message loop:
    BOOL bDone = FALSE;
    while (!bDone)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

    }

    return (int) msg.wParam;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   HWND hWnd = CreateWindowEx(
       NULL,                   /* Extended possibilites for variation */
       szWindowClass,         /* Classname */
       szTitle,            /* Title Text */
       WS_VISIBLE |
       WS_SYSMENU |
       WS_OVERLAPPEDWINDOW, /* default window */
       CW_USEDEFAULT,       /* Windows decides the position */
       CW_USEDEFAULT,       /* where the window ends up on the screen */
       640,                 /* The programs width */
       480,                  /* and height in pixels */
       NULL,                 /* The window is not a child-window */
       0,                /*  menu */
       hInstance,       /* Program Instance handler */
       NULL                 /* No Window Creation data */
   );

   if (!hWnd)
   {
      return FALSE;
   }

   g_HWndParent = hWnd;

   ShowWindow(hWnd, nCmdShow);

   hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
       COINIT_DISABLE_OLE1DDE);

   return SUCCEEDED(hr);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HWND hWndEdit;
        HFONT hFont;

        hWndEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            szEditWindowClass,
            L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
            1,
            1,
            100,
            100,
            hWnd,
            (HMENU) IDC_EDIT,
            GetModuleHandle(NULL),
            NULL
        );

        if (hWndEdit == NULL)
        {
            MessageBox(g_HWndParent, L"Could not Create Edit control!", L"Error", MB_OK | MB_ICONERROR);
            printf("Terminal failure: Unable to open handle.\n GetLastError=%08x\n", GetLastError());
            PostQuitMessage(0);
        }

        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hWndEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

        g_HWndEdit = hWndEdit;
        g_hFont = hFont;

        ProcessWindowPositions();
    }
    case WM_SIZE:
    {
        ProcessWindowPositions();
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_OPEN_FILE:
                ProcessOpenFile();
                break;
            case IDM_SAVE_FILE:
                // Open file
                break;
            case IDM_NEW_FILE:
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDC_EDIT:
            {
                switch (HIWORD(wParam))
                {
                case EN_CHANGE:
                    unsavedChanges = TRUE;
                    break;
                }
            }
            break;
            case IDM_EXIT:
                //TODO Add unsaved changes check
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
        CoUninitialize();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ProcessWindowPositions() {
    RECT rcClient;
    GetClientRect(g_HWndParent, &rcClient);
    SetWindowPos(g_HWndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
}

// Process a call to open a file.
void ProcessOpenFile() {
    IFileOpenDialog* pFileOpen;

    // Create the FileOpenDialog object.
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        // Show the Open dialog box.
        hr = pFileOpen->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            IShellItem* pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                // Get the filename and load the content into the Window.
                if (SUCCEEDED(hr))
                {
                    LoadFileIntoMinPad(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
}

void LoadFileIntoMinPad(PWSTR pszFilePath) {

    HANDLE hFile;
    DWORD dwBytesRead = 0;
    DWORD dwFileSize;

    hFile = CreateFile(pszFilePath, // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_HWndParent, L"Could not get a handle to file.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    dwFileSize = GetFileSize(hFile, NULL);

    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        MessageBox(g_HWndParent, L"Invalid File size.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Allocate memory from the heap of an appropriate size.
    LPSTR lpFileText;
    lpFileText = (char*)GlobalAlloc(GPTR, dwFileSize + 1);

    if (lpFileText != NULL)
    {
        DWORD dwRead;
        if (ReadFile(hFile, lpFileText, dwFileSize, &dwRead, NULL))
        {
            lpFileText[dwFileSize] = 0;
            SetWindowTextA(g_HWndEdit, lpFileText);
            ProcessWindowPositions();
        }
        else {
            MessageBox(g_HWndParent, L"Failed to open file.", L"Error", MB_OK | MB_ICONERROR);
        }
        GlobalFree(lpFileText);
    }
    CloseHandle(hFile);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
