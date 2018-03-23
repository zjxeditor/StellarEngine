//
// Manage command allocators.
//

#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>

namespace Graphics
{
	// Each CommandQueue owns a CommandAllocatorPool. One CommandAllocator will be request for CommandList's usage.
	class CommandAllocatorPool
	{
	public:
		CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type);
		~CommandAllocatorPool();

		void Create(ID3D12Device* pDevice);
		void Shutdown();

		ID3D12CommandAllocator* RequestAllocator(uint64_t CompletedFenceValue);
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

		size_t Size() { return m_AllocatorPool.size(); }

	private:
		const D3D12_COMMAND_LIST_TYPE m_cCommandListType;

		ID3D12Device* m_Device;
		std::vector<ID3D12CommandAllocator*> m_AllocatorPool;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
		std::mutex m_AllocatorMutex;
	};

}	// namespace Graphics
