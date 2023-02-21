// 3DShapeViewer.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "3DShapeViewer.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst; // 인스턴스 핸들
WCHAR szTitle[MAX_LOADSTRING]; // 제목 표시줄 텍스트
WCHAR szWindowClass[MAX_LOADSTRING]; // 기본 창 클래스 이름

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    // 전역 문자열 초기화
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY3DSHAPEVIEWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화 수행:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY3DSHAPEVIEWER));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//  함수: MyRegisterClass()
//  목적: 창 클래스를 등록합니다.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MY3DSHAPEVIEWER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY3DSHAPEVIEWER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
//  함수: InitInstance(HINSTANCE, int)
//  목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//  목적: 주 창의 메시지를 처리합니다.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId) {
        case IDM_ABOUT: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        case IDM_EXIT: {
            DestroyWindow(hWnd);
            break;
        }
        case IDM_OPEN: { // "파일 열기" 메뉴가 선택됨
            OPENFILENAME ofn = { 0 };
            TCHAR szFile[MAX_PATH] = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn) == TRUE) { // 파일 다이얼로그에서 OK 버튼을 클릭함
                HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hFile != INVALID_HANDLE_VALUE) { // 파일 열기에 성공함
                    DWORD dwSize = GetFileSize(hFile, nullptr);
                    char* pBuffer = new char[dwSize + 1];
                    DWORD dwRead = 0;
                    ReadFile(hFile, pBuffer, dwSize, &dwRead, nullptr);
                    pBuffer[dwSize] = '\0';
                    CloseHandle(hFile);

                    // UTF-8 인코딩의 문자열을 유니코드 문자열로 변환함
					int nLen = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, nullptr, 0);
					wchar_t* pWideChar = new wchar_t[nLen];
					MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, pWideChar, nLen);

					MessageBoxW(hWnd, pWideChar, L"파일 내용", MB_OK);
					delete[] pBuffer;
					delete[] pWideChar;
				}
				else { // 파일 열기에 실패함
					MessageBox(hWnd, TEXT("파일을 열 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR);
				}
			}
			break;
        }
        case IDM_SAVE: { // "파일 저장" 메뉴가 선택됨
            OPENFILENAME ofn = { 0 };
            TCHAR szFile[MAX_PATH] = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofn) == TRUE) { // 파일 다이얼로그에서 OK 버튼을 클릭함
                HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hFile != INVALID_HANDLE_VALUE) { // 파일 생성에 성공함
                    DWORD dwWritten = 0;
                    TCHAR szText[] = TEXT("Hello, world!");
                    WriteFile(hFile, szText, sizeof(szText), &dwWritten, nullptr);
                    CloseHandle(hFile);
                    MessageBox(hWnd, TEXT("파일을 저장했습니다."), TEXT("알림"), MB_OK);
                }
                else { // 파일 생성에 실패함
                    MessageBox(hWnd, TEXT("파일을 생성할 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR);
                }
            }
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 HDC를 사용하는 그리기 코드를 추가합니다.
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//  함수: About(HWND, UINT, WPARAM, LPARAM)
//  목적: "정보" 대화 상자의 메시지를 처리합니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
