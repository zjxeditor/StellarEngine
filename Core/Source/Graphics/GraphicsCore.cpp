//
// Main interface for graphics stuff.
//

#include "pch.h"
#include "GraphicsCore.h"

namespace Graphics {
	
ID3D12Device* g_Device = nullptr;

CommandListManager g_CommandManager;
ContextManager g_ContextManager;

DescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
};

}	// namespace Graphics
