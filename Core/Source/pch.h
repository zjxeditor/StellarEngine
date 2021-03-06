//
// Include file for standard system include files, 
// or project specific include files that are used frequently, 
// but are changed infrequently. 
// Reference additional headers your program requires here
//

#pragma once

#pragma warning(disable:4201)	// nonstandard extension used : nameless struct/union
#pragma warning(disable:4328)	// nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4324)	// structure was padded due to __declspec(align())

#ifndef WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "targetver.h"

#include <windows.h>
#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define MY_IID_PPV_ARGS IID_PPV_ARGS
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#include "Graphics/d3dx12.h"

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <memory>
#include <string>
#include <exception>
#include <wrl.h>
#include <ppltasks.h>

#include "Math/VectorMath.h"
#include "Core/Utility.h"


    


