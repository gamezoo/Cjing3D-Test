#pragma once

#ifdef CJING3D_RENDERER_DX11
#ifdef CJING3D_PLATFORM_WIN32

#include <d3d11_3.h>
#include <DXGI1_3.h>

#include <wrl.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#endif
#endif