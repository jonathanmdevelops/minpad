#include "MinPad.h"
#include "framework.h"

#define ID_EDITCHILD 100

HINSTANCE g_hInst;
HWND g_hWnd;

WCHAR szTitle[] = L"MinPad";
WCHAR szWindowClass[] = L"ParentClass";
WCHAR szEditWindowClass[] = L"Edit";

HRESULT hr;
BOOL bUnsavedChanges = FALSE;

BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                ShowErrorDialog(HWND, const wchar_t*);
void                SetEditWindowText(HWND, const wchar_t*);
int                 CheckChanges();
void                ProcessNewFile(HWND);
void                ProcessOpenFile(HWND);
void                ProcessSaveFile(HWND);
void                LoadFileIntoMinPad(HWND, PWSTR);
void                SaveFileToDisk(HWND, PWSTR);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINPAD));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MINPAD));

    RegisterClassEx(&wcex);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    g_hInst = hInstance;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(g_hWnd, hAccelTable, &msg))
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
       NULL,
       szWindowClass,
       szTitle,
       WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, CW_USEDEFAULT,
       640, 480,
       NULL,
       0,
       hInstance,
       NULL
   );

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   g_hWnd = hWnd;
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
            TEXT(""),
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN,
            1,
            1,
            100,
            100,
            hWnd,
            (HMENU)ID_EDITCHILD,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL
        );

        if (hWndEdit == NULL)
        {
            ShowErrorDialog(hWnd, TEXT("Could not Create Edit control."));
            PostQuitMessage(0);
        }

        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }
    case WM_SIZE:
    {
        HWND hWndEdit;
        RECT rcClient;

        GetClientRect(hWnd, &rcClient);
        hWndEdit = GetDlgItem(hWnd, ID_EDITCHILD);
        SetWindowPos(hWndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_FILE_OPEN:
            case ID_ACCELERATOR_OPEN:
                ProcessOpenFile(hWnd);
                break;
            case IDM_FILE_SAVE:
            case ID_ACCELERATOR_SAVE:
                ProcessSaveFile(hWnd);
                break;
            case IDM_FILE_NEW:
            case ID_ACCELERATOR_NEW:
                ProcessNewFile(hWnd);
                break;
            case IDM_HELP_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, About);
                break;
            case ID_EDITCHILD:
            {
                switch (HIWORD(wParam))
                {
                case EN_CHANGE:
                    bUnsavedChanges = TRUE;
                    break;
                }
            }
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


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDD_ABOUT_OK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void ShowErrorDialog(HWND hWnd, const wchar_t* wczErrorText) {
    MessageBox(hWnd, wczErrorText, TEXT("Error"), MB_OK | MB_ICONERROR);
}

void SetEditWindowText(HWND hWnd, const wchar_t* wczText) {
    HWND hWndEdit = GetDlgItem(hWnd, ID_EDITCHILD);
    SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM) wczText);
}

int CheckChanges() {
    if (bUnsavedChanges) {
        return MessageBox(g_hWnd, TEXT("Would you like to continue?"), TEXT("Unsaved Changes"), MB_YESNO | MB_ICONINFORMATION);
    }
    return IDYES;
}

void ProcessNewFile(HWND hWnd) {
    if (CheckChanges() != IDYES) {
        return;
    }
    SetEditWindowText(hWnd, TEXT(""));
}

void ProcessOpenFile(HWND hWnd) {
    if (CheckChanges() != IDYES) {
        return;
    }
    IFileOpenDialog* pFileOpen;

    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        hr = pFileOpen->Show(NULL);

        if (SUCCEEDED(hr))
        {
            IShellItem* pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

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
    DWORD dwAllocationSize;

    hFile = CreateFile(pszFilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        ShowErrorDialog(hWnd, TEXT("Could not get a handle to file."));
        return;
    }

    dwFileSize = GetFileSize(hFile, NULL);

    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        ShowErrorDialog(hWnd, TEXT("Invalid file size."));
        return;
    }
    dwAllocationSize = 2 * (dwFileSize + 1);
    wchar_t* wczFileText;

    wczFileText = (wchar_t*) HeapAlloc(GetProcessHeap(), 0, dwAllocationSize);

    if (wczFileText != NULL)
    {
        DWORD dwRead;
        if (ReadFile(hFile, wczFileText, dwAllocationSize, &dwRead, NULL))
        {
            SetEditWindowText(hWnd, wczFileText);
        }
        else {
            ShowErrorDialog(hWnd, TEXT("Failed to read file."));
        }
        HeapFree(GetProcessHeap(), 0, wczFileText);
    }
    else {
        ShowErrorDialog(hWnd, TEXT("Failed to allocate memory."));
    }
    CloseHandle(hFile);
}

void ProcessSaveFile(HWND hWnd) {
    IFileSaveDialog* pFileSave;

    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
        IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

    if (SUCCEEDED(hr))
    {
        hr = pFileSave->Show(hWnd);

        if (SUCCEEDED(hr))
        {
            IShellItem* pItem;
            hr = pFileSave->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (SUCCEEDED(hr))
                {
                    SaveFileToDisk(hWnd, pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileSave->Release();
    }
}

void SaveFileToDisk(HWND hWnd, PWSTR pszFilePath) {
    HWND hWndEdit = GetDlgItem(hWnd, ID_EDITCHILD);
    HANDLE hFile;
    DWORD dwTextLength;
    DWORD dwAllocationLength;

    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

    hFile = CreateFile(pszFilePath,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        ShowErrorDialog(hWnd, TEXT("Could not get handle to file."));
        return;
    }

    dwTextLength = GetWindowTextLength(hWndEdit);
    dwAllocationLength = 2 * (dwTextLength + 1);

    wchar_t* wczFileText;

    wczFileText = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, dwAllocationLength);

    if (wczFileText != NULL)
    {
        if (GetWindowText(hWndEdit, wczFileText, dwTextLength + 1))
        {
            if (!WriteFile(hFile, wczFileText, dwAllocationLength, &dwBytesWritten, NULL)) {
                ShowErrorDialog(hWnd, TEXT("Failed to write file."));
            }
            else {
                bUnsavedChanges = FALSE;
            }
        }
        else {
            ShowErrorDialog(hWnd, TEXT("Failed to get text from Window."));
        }
        HeapFree(GetProcessHeap(), 0, wczFileText);
    }
    else {
        ShowErrorDialog(hWnd, TEXT("Failed to allocate memory."));
    }
    CloseHandle(hFile);
}
