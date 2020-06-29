#include "MinPad.h"
#include "framework.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;
WCHAR szTitle[] = L"MinPad";
WCHAR szWindowClass[] = L"ParentClass";
WCHAR szEditWindowClass[] = L"Edit";
HRESULT hr;

BOOL bUnsavedChanges = FALSE;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                ProcessOpenFile(HWND);
void                ProcessSaveFile();
void                ProcessNewFile(HWND);
void                LoadFileIntoMinPad(HWND, PWSTR);
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
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
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

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
            MessageBox(hWnd, L"Could not Create Edit control!", L"Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(0);
        }

        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }
    case WM_SIZE:
    {
        HWND hWndEdit;
        RECT rcClient;

        GetClientRect(hWnd, &rcClient);
        hWndEdit = GetDlgItem(hWnd, IDC_EDIT);
        SetWindowPos(hWndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
    }
    break;
    case WM_PAINT:
    {
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_OPEN_FILE:
                ProcessOpenFile(hWnd);
                break;
            case IDM_SAVE_FILE:
                ProcessSaveFile();
                break;
            case IDM_NEW_FILE:
                ProcessNewFile(hWnd);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDC_EDIT:
            {
                switch (HIWORD(wParam))
                {
                case EN_CHANGE:
                    bUnsavedChanges = TRUE;
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
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);;
}

// Process a call to open a file.
void ProcessOpenFile(HWND hWnd) {
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
                    LoadFileIntoMinPad(hWnd, pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
}

void LoadFileIntoMinPad(HWND hWnd, PWSTR pszFilePath) {

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
        MessageBox(hWnd, L"Could not get a handle to file.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    dwFileSize = GetFileSize(hFile, NULL);

    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        MessageBox(hWnd, L"Invalid File size.", L"Error", MB_OK | MB_ICONERROR);
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
            HWND hWndEdit = GetDlgItem(hWnd, IDC_EDIT);
            SetWindowTextA(hWndEdit, lpFileText);
        }
        else {
            MessageBox(hWnd, L"Failed to open file.", L"Error", MB_OK | MB_ICONERROR);
        }
        GlobalFree(lpFileText);
    }
    CloseHandle(hFile);
}

void ProcessSaveFile() {

}

void ProcessNewFile(HWND hWnd) {
    HWND hWndEdit = GetDlgItem(hWnd, IDC_EDIT);
    SetWindowTextA(hWndEdit, "");
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
