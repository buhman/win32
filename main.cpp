#include <windows.h>
#include <d3d9.h>
#include <stdint.h>

struct CUSTOMVERTEX {
  float x, y, z, rhw;
  uint32_t color;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

const CUSTOMVERTEX vertices[] = {
  { 150.0f,  50.0f, 0.5f, 1.0f, 0xffff0000 },
  { 250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00 },
  {  50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff },
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

IDirect3D9 * g_pD3D = NULL;
IDirect3DDevice9 * g_pd3dDevice = NULL;
IDirect3DVertexBuffer9 * g_pVB = NULL;

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

  g_pd3dDevice->BeginScene();
  g_pd3dDevice->SetStreamSource(0, g_pVB, 0, (sizeof (CUSTOMVERTEX)));
  g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
  g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
  g_pd3dDevice->EndScene();

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

  HRESULT res;

  res = g_pd3dDevice->CreateVertexBuffer(3 * (sizeof (CUSTOMVERTEX)), // length
                                         0, // usage
                                         D3DFVF_CUSTOMVERTEX, // FVF
                                         D3DPOOL_DEFAULT, // Pool
                                         &g_pVB, //ppVertexBuffer
                                         NULL //pSharedHandle
                                         );
  if (FAILED(res))
    return 0;

  void * pVertices;
  res = g_pVB->Lock(0, (sizeof (vertices)), &pVertices, 0);
  if (FAILED(res))
    return 0;

  memcpy(pVertices, vertices, (sizeof (vertices)));

  g_pVB->Unlock();

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
      //ValidateRect(hwnd, NULL);
    }
    return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
