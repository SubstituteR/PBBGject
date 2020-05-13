#pragma once
#include <d3d9.h>
class DX9
{
private:
	typedef HRESULT(__stdcall* CreateTextureHook) (IDirect3DDevice9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture9**, HANDLE*);
	static CreateTextureHook createTextureHook;

	typedef HRESULT(__stdcall* EndSceneHook) (IDirect3DDevice9*);
	static EndSceneHook endSceneHook;

	typedef ULONG(__stdcall* ReleaseHook) (IUnknown*);
	static ReleaseHook releaseHook;
public:
	static HRESULT __stdcall EndScene(IDirect3DDevice9*);
	static HRESULT __stdcall CreateTexture(IDirect3DDevice9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture9**, HANDLE*);
	static ULONG   __stdcall Release(IUnknown*);
	static IDirect3DDevice9* CreateTemporaryResources();
	static void Initialize();
};