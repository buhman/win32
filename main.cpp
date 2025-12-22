#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdint.h>
#include <stdio.h>
#include <dinput.h>
#include <dsound.h>
#include <assert.h>

#include <comdef.h>

struct CUSTOMVERTEX
{
  float x, y, z;
  uint32_t color;
};

const CUSTOMVERTEX g_Vertices[] = {
  { -1.0f,-1.0f, 0.0f, 0xffff0000, },
  {  1.0f,-1.0f, 0.0f, 0xff0000ff, },
  {  0.0f, 1.0f, 0.0f, 0xffffffff, },
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

IDirect3D9 * g_pD3D = NULL;
IDirect3DDevice9 * g_pd3dDevice = NULL;
IDirect3DVertexBuffer9 * g_pVB = NULL;
IDirect3DVertexDeclaration9 * g_pVertexDeclaration = NULL;
ID3DXConstantTable * g_pConstantTable = NULL;
IDirect3DVertexShader9 * g_pVertexShader = NULL;
IDirect3DPixelShader9 * g_pPixelShader = NULL;

IDirectInput8 * g_pDI = NULL;
IDirectInputDevice8 * g_pdiDevice;

IDirectSound8 * g_pDS = NULL;

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

  g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
  g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

  return S_OK;
}

BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE * pdidInstance,
                                  void * pvRef)
{
  HRESULT hr;

  hr = g_pDI->CreateDevice(pdidInstance->guidInstance, &g_pdiDevice, NULL );
  if( FAILED(hr) )
    return DIENUM_CONTINUE;

  return DIENUM_STOP;
}

HRESULT InitDirectInput(HINSTANCE hInstance)
{
  HRESULT hr;

  hr = DirectInput8Create(hInstance,
                          DIRECTINPUT_VERSION,
                          IID_IDirectInput8,
                          (void **)&g_pDI,
                          NULL);
  if (FAILED(hr))
    return hr;

  hr = g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
                          EnumDevicesCallback,
                          NULL,
                          DIEDFL_ATTACHEDONLY);
  if (FAILED(hr))
    return hr;

  if (g_pdiDevice == NULL) {
    fprintf(stderr, "no game controller found\n");
    fflush(stderr);
    return E_FAIL;
  }

  hr = g_pdiDevice->SetDataFormat(&c_dfDIJoystick);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

void Cleanup()
{
  if (g_pd3dDevice != NULL)
    g_pd3dDevice->Release();

  if (g_pD3D != NULL)
    g_pD3D->Release();
}

D3DXMATRIX WorldViewProjection()
{
  D3DXMATRIX matWorld;
  UINT  iTime  = timeGetTime() % 1000;
  FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 1000.0f;
  D3DXMatrixRotationY( &matWorld, fAngle );

  D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
  D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
  D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
  D3DXMATRIX matView;
  D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );

  D3DXMATRIX matProj;
  D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );

  D3DXMATRIX mWorldViewProj = matWorld * matView * matProj;
  return mWorldViewProj;
}

void Render()
{
  if (g_pd3dDevice == NULL)
    return;

  g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 10, 10), 1.0f, 0);

  g_pd3dDevice->BeginScene();

  D3DXMATRIX mWorldViewProj = WorldViewProjection();
  g_pConstantTable->SetMatrix(g_pd3dDevice, "mWorldViewProj", &mWorldViewProj);

  g_pd3dDevice->SetVertexDeclaration(g_pVertexDeclaration);
  g_pd3dDevice->SetVertexShader(g_pVertexShader);
  g_pd3dDevice->SetPixelShader(g_pPixelShader);

  g_pd3dDevice->SetStreamSource(0, g_pVB, 0, (sizeof (CUSTOMVERTEX)));
  g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
  g_pd3dDevice->EndScene();

  g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

HRESULT UpdateInput()
{
  HRESULT hr;

  hr = g_pdiDevice->Poll();
  if (FAILED(hr)) {
    hr = g_pdiDevice->Acquire();
    while (hr == DIERR_INPUTLOST)
      hr = g_pdiDevice->Acquire();
    return S_OK;
  }

  DIJOYSTATE js;

  g_pdiDevice->GetDeviceState((sizeof (DIJOYSTATE)), &js);

  printf("[lx %ld] ", js.lX); // left stick
  printf("[ly %ld] ", js.lY);
  printf("[lz %ld] ", js.lZ); // trigger
  printf("[lrx %ld] ", js.lRx); // right stick
  printf("[lry %ld] ", js.lRy);
  printf("[lrz %ld] ", js.lRz); // zero

  printf("[pov %ld] ", js.rgdwPOV[0]);
  static int max = -1;
  for (int i = 0; i < 8; i++) {
    if (js.rgbButtons[i] != 0 && i > max) {
      max = i;
    }
  }
  printf("[max %d] ", max);
  printf("\n");
  fflush(stdout);

  return S_OK;
}

HRESULT InitDirectSound(HWND hwnd)
{
  HRESULT hr;

  hr = DirectSoundCreate8(NULL,
                          &g_pDS,
                          NULL);
  if (FAILED(hr)) {
    fprintf(stderr, "DirectSoundCreate8\n");
    return hr;
  }

  hr = g_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
  if (FAILED(hr)) {
    fprintf(stderr, "SetCooperativeLevel\n");
    return hr;
  }

  LPDIRECTSOUNDBUFFER pDsb = NULL;

  WAVEFORMATEX wfx;
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 2;
  wfx.nSamplesPerSec = 44100;
  wfx.nBlockAlign = 4;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
  wfx.wBitsPerSample = 16;

  DSBUFFERDESC desc = {};
  desc.dwSize = (sizeof (DSBUFFERDESC));
  desc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
  desc.dwBufferBytes = 4 * wfx.nAvgBytesPerSec;
  desc.guid3DAlgorithm = DS3DALG_DEFAULT;
  desc.lpwfxFormat = &wfx;

  hr = g_pDS->CreateSoundBuffer(&desc, &pDsb, NULL);
  if (FAILED(hr)) {
    fprintf(stderr, "CreateSoundBuffer\n");
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    fprintf(stderr, "%ls\n", errMsg);
    fflush(stderr);
    return hr;
  }

  void * pBuffer;
  DWORD dwBufferSize;

  pDsb->Lock(0, desc.dwBufferBytes,
             &pBuffer, &dwBufferSize,
             NULL, NULL, 0);

  FILE * fp = fopen("shire.pcm", "rb");
  assert(fp != NULL);
  fread(pBuffer, 1, dwBufferSize, fp);
  fclose(fp);

  pDsb->Unlock(pBuffer, dwBufferSize, NULL, 0);

  pDsb->Play(0, 0, DSBPLAY_LOOPING);

  while (1);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  if (0) {
    HRESULT hr;
    hr = InitDirectInput(hInstance);
    if (FAILED(hr))
      return 0;

    while (true) {
      hr = UpdateInput();
      if (FAILED(hr))
        return 0;
      Sleep(100);
    }
  }

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

  InitDirectSound(hwnd);
  return 0;

  if (!SUCCEEDED(InitDirect3D(hwnd)))
    return 0;

  HRESULT res;

  //////////////////////////////////////////////////////////////////////
  // vertex buffer
  //////////////////////////////////////////////////////////////////////

  res = g_pd3dDevice->CreateVertexBuffer(3 * (sizeof (CUSTOMVERTEX)), // length
                                         0, // usage
                                         0,//D3DFVF_CUSTOMVERTEX, // FVF
                                         D3DPOOL_DEFAULT, // Pool
                                         &g_pVB, //ppVertexBuffer
                                         NULL //pSharedHandle
                                         );
  if (FAILED(res))
    return 0;

  void * pVertices;
  res = g_pVB->Lock(0, (sizeof (g_Vertices)), &pVertices, 0);
  if (FAILED(res))
    return 0;

  memcpy(pVertices, g_Vertices, (sizeof (g_Vertices)));

  g_pVB->Unlock();

  //////////////////////////////////////////////////////////////////////
  // vertex declaration
  //////////////////////////////////////////////////////////////////////


  const D3DVERTEXELEMENT9 decl[] = {
    { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
    D3DDECL_END()
  };

  res = g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclaration);
  if (FAILED(res))
    return 0;

  //////////////////////////////////////////////////////////////////////
  // vertex shader
  //////////////////////////////////////////////////////////////////////

  LPD3DXBUFFER pCode;
  DWORD dwShaderFlags = 0;

  res = D3DXCompileShaderFromFile(L"main.vsh", // pSrcFile
                                  NULL, // pDefines
                                  NULL, // pInclude
                                  "Main", // pFunctionName
                                  "vs_3_0", // pProfile
                                  dwShaderFlags, // Flags
                                  &pCode, // ppShader
                                  NULL, // ppErrorMsgs
                                  &g_pConstantTable // ppConstantTable
                                  );
  if (FAILED(res)) {
    fprintf(stderr, "D3DXCompileShader\n");
    return 0;
  }

  res = g_pd3dDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &g_pVertexShader);
  pCode->Release();
  if(FAILED(res)) {
    fprintf(stderr, "CreateVertexShader\n");
    return 0;
  }

  //////////////////////////////////////////////////////////////////////
  // pixel shader
  //////////////////////////////////////////////////////////////////////

  res = D3DXCompileShaderFromFile(L"main.psh", // pSrcFile
                                  NULL, // pDefines
                                  NULL, // pInclude
                                  "Main", // pFunctionName
                                  "ps_3_0", // pProfile
                                  dwShaderFlags, // Flags
                                  &pCode, // ppShader
                                  NULL, // ppErrorMsgs
                                  &g_pConstantTable // ppConstantTable
                                  );
  if (FAILED(res)) {
    fprintf(stderr, "D3DXCompileShader\n");
    return 0;
  }
  res = g_pd3dDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &g_pPixelShader);
  pCode->Release();
  if(FAILED(res)) {
    fprintf(stderr, "CreatePixelShader\n");
    return 0;
  }

  //

  fprintf(stderr, "success\n");
  fflush(stderr);

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
