#include <windows.h>
#include <d3d9.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

HRESULT InitDirect3D(HWND hwnd)
{
  g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
  if (g_pD3D == NULL)
    return E_FAIL;

  D3DPRESENT_PARAMETERS d3dpp = {};
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

  HRESULT res = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                                     D3DDEVTYPE_HAL,
                                     hwnd,
                                     D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                     &d3dpp,
                                     &g_pd3dDevice);
  if (FAILED(res)) {
    return E_FAIL;
  }

  return S_OK;
}

void Cleanup()
{
  if (g_pd3dDevice != NULL)
    g_pd3dDevice->Release();

  if (g_pD3D != NULL)
    g_pD3D->Release();
}

void Render()
{
  if (g_pd3dDevice == NULL)
    return;

  g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

  if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
    g_pd3dDevice->EndScene();
  }

  g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  const wchar_t CLASS_NAME[] = L"Sample Window Class";

  WNDCLASS wc = {};

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass(&wc);

  HWND hwnd = CreateWindowEx(0, // window style
                             CLASS_NAME, // window class
                             L"Learn to Program Windows",
                             WS_OVERLAPPEDWINDOW,
                             // size and position
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                             NULL, // parent window
                             NULL, // menu
                             hInstance, // instance handle
                             NULL // additional application data
                             );
  if (hwnd == NULL) {
    return 0;
  }

  if (!SUCCEEDED(InitDirect3D(hwnd)))
    return 0;

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_PAINT:
    {
      Render();
      ValidateRect(hwnd, NULL);
    }
    return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
