#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "strmiids")

#include <windows.h>
#include <dshow.h>

TCHAR szClassName[] = TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static IGraphBuilder*   pGraphBuilder;
	static IMediaControl*   pMediaControl;
	static IVideoWindow*    pVideoWindow;
	static IMediaPosition*  pMediaPosition;
	switch (msg)
	{
	case WM_CREATE:
		CoInitialize(NULL);
		DragAcceptFiles(hWnd, 1);
		break;
	case WM_DROPFILES:
		{
			TCHAR szFilePath[MAX_PATH] = { 0 };
			if (DragQueryFile((HDROP)wParam, -1, NULL, 0) == 1)
			{
				DragQueryFile((HDROP)wParam, 0, szFilePath, MAX_PATH);
			}
			DragFinish((HDROP)wParam);
			if (szFilePath[0])
			{
				SendMessage(hWnd, WM_APP, 0, 0);

				CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID*)&pGraphBuilder);

				pGraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID*)&pMediaControl);
				pGraphBuilder->QueryInterface(IID_IMediaPosition, (LPVOID*)&pMediaPosition);
				pGraphBuilder->QueryInterface(IID_IVideoWindow, (LPVOID*)&pVideoWindow);

				HRESULT hr = pMediaControl->RenderFile(szFilePath);
				if (SUCCEEDED(hr))
				{
					pMediaPosition->put_CurrentPosition((REFTIME)0.0);

					pVideoWindow->put_Owner((OAHWND)hWnd);
					pVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
					{
						RECT rect;
						GetClientRect(hWnd, &rect);
						pVideoWindow->SetWindowPosition(0, 0, rect.right, rect.bottom);
					}
					pVideoWindow->put_Visible(OATRUE);

					pMediaControl->Run();
				}
			}
		}
		break;
	case WM_SIZE:
		if (pVideoWindow)
		{
			pVideoWindow->SetWindowPosition(0, 0, LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_APP:
		if (pVideoWindow != NULL)
		{
			pVideoWindow->Release();
			pVideoWindow = NULL;
		}
		if (pMediaPosition != NULL)
		{
			pMediaPosition->Release();
			pMediaPosition = NULL;
		}
		if (pMediaControl != NULL)
		{
			pMediaControl->Release();
			pMediaControl = NULL;
		}
		if (pGraphBuilder != NULL)
		{
			pGraphBuilder->Release();
			pGraphBuilder = NULL;
		}
		break;
	case WM_DESTROY:
		SendMessage(hWnd, WM_APP, 0, 0);
		CoUninitialize();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("ドロップした動画ファイルをDirectShowで再生"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
