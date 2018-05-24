//
// Main interface for graphics stuff.
//

#pragma once

#include "DescriptorHeap.h"
#include "RootSignature.h"
#include "CommandListManager.h"
#include "CommandContext.h"

namespace Graphics {
	
extern ID3D12Device* g_Device;
extern CommandListManager g_CommandManager;
extern ContextManager g_ContextManager;

extern DescriptorAllocator g_DescriptorAllocator[];
inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1) {
	return g_DescriptorAllocator[Type].Allocate(Count);
}

}	// namespace Graphics.