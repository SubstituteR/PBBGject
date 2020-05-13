#pragma once
#pragma warning(disable: 26812) //unscoped enums
#include "../../Public/D3D/DX9.h"
#include <stdio.h>
#include <detours.h>
#include <string>
#include <d3dx9tex.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")


#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>
#include <vector>
#include <mutex>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Direct3DDevice9VirtalTable
{
public:
	static const int CreateTexture = 23;
	static const int EndScene = 42;
};
class IUnknownVirtualTable
{
public:
	static const int Release = 2;
};

DX9::CreateTextureHook DX9::createTextureHook;
DX9::EndSceneHook DX9::endSceneHook;
DX9::ReleaseHook DX9::releaseHook;
static bool imguiSetup = false;

static WNDPROC wndprocHook;
static int counter = 0;
static std::vector<IDirect3DTexture9**> textures;
static std::mutex textureMutex;

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto io = ImGui::GetIO();
	HWND window = static_cast<HWND>(io.ImeWindowHandle);
	ImGui::GetIO().MouseDrawCursor = true;
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	return CallWindowProc(wndprocHook, hWnd, uMsg, wParam, lParam);
}


HRESULT __stdcall DX9::EndScene(IDirect3DDevice9* _this)
{
	auto res = endSceneHook(_this);
	D3DDEVICE_CREATION_PARAMETERS parameters;
	ZeroMemory(&parameters, sizeof(parameters));
	_this->GetCreationParameters(&parameters);
	if (!imguiSetup)
	{
		printf("Setup IMGUI\n");
		ImGui::CreateContext();
		ImGui_ImplDX9_Init(_this);
		ImGui_ImplWin32_Init(parameters.hFocusWindow);
		
		ImGui::GetIO().ImeWindowHandle = parameters.hFocusWindow;
		imguiSetup = true;

		wndprocHook = (WNDPROC)SetWindowLongPtr(parameters.hFocusWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	

	ImGui::NewFrame();
	bool a = true;

	ImGui::Begin("This is a test window", &a, ImGuiWindowFlags_None);
	/*
	
		textureMutex.lock();
	int count = 0;
	static int page = 0;

	for (int i = 0; i < 1; i++)
	{
		int index = i + (1 * page);

		if (index < textures.size())
		{
			IDirect3DTexture9* texture = *(textures[index]);
			try {
				texture->AddRef();
				texture->Release();
				ImGui::Image(texture, ImVec2(128, 128));
			}
			catch (std::exception& what)
			{
			}
			printf("%p -> %p\n", textures[index], texture);
			if (textures[index] && texture)
			{
				if (ImGui::Button((std::string("Save") + std::to_string(count++)).c_str()))
				{
					try {
						printf("Saving...\n");
						std::wstring path = std::wstring(L"C:\\Users\\Substitute\\Test\\") + std::to_wstring(counter++) + std::wstring(L".dds");
						D3DXSaveTextureToFile(path.c_str(), D3DXIMAGE_FILEFORMAT::D3DXIFF_DDS, texture, NULL);
					}
					catch (std::exception& what)
					{
						printf("Invalid Texture");
					}
				}
			}
		}
	}

	if (ImGui::Button("Next"))
		++page;
	if (ImGui::Button("Previous"))
		--page;
	ImGui::LabelText(std::to_string(page).c_str(), "");
	textureMutex.unlock();
	
	*/
	ImGui::LabelText("Hello!", "");
	ImGui::End();
	IDirect3DTexture9** texture = (IDirect3DTexture9**)((int)GetModuleHandle(NULL) + 0x0067C434);
	if (*texture)
		ImGui::Image(*texture, ImVec2(96, 128));

	static int page = 0;

	IDirect3DTexture9* te = *(textures[page]);
	if (te)
		ImGui::Image(te, ImVec2(96, 128));

	if (ImGui::Button("Next"))
		++page;
	if (page >= textures.size())
		page = 0;
	if (ImGui::Button("Previous"))
		--page;
	if (page < 0)
		page = textures.size() - 1;

	if (ImGui::Button("Save"))
	{
		try {
			printf("Saving...\n");
			std::wstring path = std::wstring(L"C:\\Users\\Substitute\\Test\\") + std::to_wstring(counter++) + std::wstring(L".dds");
			D3DXSaveTextureToFile(path.c_str(), D3DXIMAGE_FILEFORMAT::D3DXIFF_DDS, te, NULL);
		}
		catch (std::exception& what)
		{
			printf("Invalid Texture");
		}
	}

	for (IDirect3DTexture9** texture : textures)
	{
		ImGui::LabelText("", "%p", *texture);
	}



	//ImGui::Image(surface, ImVec2(64, 64));
	auto dl = ImGui::GetBackgroundDrawList();
	dl->AddCircleFilled(ImVec2(100, 100), 64, 0xff0000ff, 64);
	
	
	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return res;
}



HRESULT __stdcall DX9::CreateTexture(IDirect3DDevice9* _this, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	auto res = createTextureHook(_this, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
	textureMutex.lock();
	textures.push_back(ppTexture);
	textureMutex.unlock();
	return res;
}

ULONG __stdcall DX9::Release(IUnknown* _this)
{
	ULONG remaining = releaseHook(_this);
	printf("release is called %i\n", remaining);
	if (remaining == 0)
	{
		printf("release is called.........NO REF !!!\n");
	}
	return remaining;
}

IDirect3DDevice9* DX9::CreateTemporaryResources()
{
	D3DPRESENT_PARAMETERS PresentParametes;
	ZeroMemory(&PresentParametes, sizeof(PresentParametes));
	PresentParametes.Windowed = true;
	PresentParametes.SwapEffect = D3DSWAPEFFECT_COPY;
	IDirect3DDevice9* Device;
	LPDIRECT3D9 D3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (FAILED(D3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), D3DCREATE_SOFTWARE_VERTEXPROCESSING, &PresentParametes, &Device)))
		printf("Unable to create D3D9 Device.");
	else
		printf("Created D3D9 Device.");
	return Device;
}



void DX9::Initialize()
{
	IDirect3DDevice9* Device = CreateTemporaryResources();
	IDirect3DTexture9* Texture;
	Device->CreateTexture(0, 0, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &Texture, NULL);
	createTextureHook = reinterpret_cast<DX9::CreateTextureHook>((*(void***)Device)[Direct3DDevice9VirtalTable::CreateTexture]);
	endSceneHook = reinterpret_cast<DX9::EndSceneHook>((*(void***)Device)[Direct3DDevice9VirtalTable::EndScene]);
	//releaseHook = reinterpret_cast<DX9::ReleaseHook>(DetourFindFunction("", "IUnknown::Release"));
	if (DetourTransactionBegin() != NO_ERROR || DetourUpdateThread(GetCurrentThread()) != NO_ERROR ||
		DetourAttach(&(PVOID&)createTextureHook, DX9::CreateTexture) != NO_ERROR ||
		DetourAttach(&(PVOID&)endSceneHook, DX9::EndScene) != NO_ERROR ||
		//DetourAttach(&(PVOID&)releaseHook, DX9::Release) != NO_ERROR ||
		DetourTransactionCommit() != NO_ERROR)
		printf("Hooks installed.");
}
