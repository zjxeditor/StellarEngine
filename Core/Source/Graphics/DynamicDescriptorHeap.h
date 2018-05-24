//
// Important concept.
//
// Descriptor heaps are one of the big changes with DX12. The idea with them is that you can create an array of 
// descriptors (which are compact, GPU-specific encodings describing each resource) and bind several descriptors 
// with one API call. The root signature describes how the descriptor table lines up with input resources. This 
// paradigm favors load-time computation rather than run-time computation, so the CPU does less work each frame. 
// But the downside is that it's not easy to dynamically build up descriptor tables on the fly. Maybe you just 
// want to change one texture, or maybe you don't know what resource you'll be using at startup because it changes 
// every frame. To be easier to use and for quick experimentation, we created a dynamic descriptor heap. This is 
// much like a dynamic constant buffer. So rather than creating an immutable buffer at startup, you might prefer 
// to lazily set values. The dynamic descriptor heap is so useful, that we don't even bother with static descriptor 
// heaps in this engine.
//
// To understand even better, I can walk you through what has to happen in a dynamic descriptor heap. But first, 
// let's recall how it works in DX11 (and below). In DX11, you have shader stages and bind slots. You can bind a 
// resource to exactly pixel shader texture slot 3 and nowhere else if you want. Then if you change pixel shader 
// texture slot 2, you would expect slot 3 still has the same texture bound. If you set vertex shader texture 
// slot 3, it does not change what texture is bound to pixel shader slot 3. This is the kind of behavior we're 
// going for with dynamic descriptor heaps.
//
// Now descriptors are opaque graphics state. That means we can get a pointer to one, but we shouldn't manipulate 
// them directly. Instead what you can do is create a CPU-visible heap where you first construct a descriptor, then 
// you copy descriptors to a GPU-visible heap. If you bind a descriptor table--which is a list of descriptors--to 
// your shader pipeline, the descriptors have to be in the right order and at the right offsets. So we copy them 
// into place in the GPU-visible heap, set the heap pointer, and set the offset to the first descriptor. The GPU 
// won't actually see this descriptor table for some time, and it's hard to know when it's finished reading it 
// without closing your command list and inserting a fence. (We don't do those very often.) So if you want to make 
// a change to your descriptor table for the next draw call, you have to make a whole new copy with your modifications. 
// This is why the dynamic descriptor heap needs to have a CPU-visible cache of descriptors. Every time we issue a 
// draw, we see if any of the cached descriptors have changed and upload a new copy of the table (and set a new offset). 
// If we run out of room in the descriptor heap, we need to allocate a new one, re-upload our existing descriptor tables, 
// and schedule the old heap for lazy cleanup.
//
// It's also important to know that if you are creating command lists on multiple threads, each command list must have its 
// own dynamic descriptor heap. This makes allocations lock-free and all of the dynamic descriptor heaps can be cleaned up 
// when this command list's fence completes.
//
// As a final note, this is very similar to how DX11 implements resource binding, and we know it's non-optimal. 
// For best DX12 performance, you want to pre-build descriptor tables/heaps.
//

#pragma once

#include "DescriptorHeap.h"
#include "RootSignature.h"
#include <vector>
#include <queue>

namespace Graphics {
	
// Forward declarations.
class CommandContext;

// This class is a linear allocation system for dynamically generated descriptor tables. It internally caches
// CPU descriptor handles so that when not enough space is available in the current heap, necessary descriptors
// can be re-copied to the new heap.

class DynamicDescriptorHeap {
public:
	DynamicDescriptorHeap(CommandContext& OwningContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
	~DynamicDescriptorHeap();

	static void DestroyAll() {
		sm_DescriptorHeapPool[0].clear();
		sm_DescriptorHeapPool[1].clear();
	}

	void CleanupUsedHeaps(uint64_t fenceValue);

	// Deduce cache layout needed to support the descriptor tables needed by the root signature.
	void ParseGraphicsRootSignature(const RootSignature& RootSig) {
		m_GraphicsHandleCache.ParseRootSignature(m_DescriptorType, RootSig);
	}
	void ParseComputeRootSignature(const RootSignature& RootSig) {
		m_ComputeHandleCache.ParseRootSignature(m_DescriptorType, RootSig);
	}

	// Copy multiple handles into the cache area reserved for the specified root parameter.
	void SetGraphicsDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]) {
		m_GraphicsHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
	}
	void SetComputeDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]) {
		m_ComputeHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
	}

	// Upload any new descriptors in the cache to the shader-visible heap.
	inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* CmdList) {
		if (m_GraphicsHandleCache.m_StaleRootParamsBitMap != 0)
			CopyAndBindStagedTables(m_GraphicsHandleCache, CmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
	}
	inline void CommitComputeRootDescriptorTables(ID3D12GraphicsCommandList* CmdList) {
		if (m_ComputeHandleCache.m_StaleRootParamsBitMap != 0)
			CopyAndBindStagedTables(m_ComputeHandleCache, CmdList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
	}

	// Bypass the cache and upload directly to the shader-visible heap
	D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handles);

private:
	// Static members
	static const uint32_t kNumDescriptorsPerHeap = 1024;
	static std::mutex sm_Mutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool[2];
	static std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> sm_RetiredDescriptorHeaps[2];
	static std::queue<ID3D12DescriptorHeap*> sm_AvailableDescriptorHeaps[2];

	// Static methods
	static ID3D12DescriptorHeap* RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
	static void DiscardDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps);

	// Non-static members
	CommandContext& m_OwningContext;
	ID3D12DescriptorHeap* m_CurrentHeapPtr;
	const D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorType;
	uint32_t m_DescriptorSize;
	uint32_t m_CurrentOffset;
	DescriptorHandle m_FirstDescriptor;
	std::vector<ID3D12DescriptorHeap*> m_RetiredHeaps;

	// Describes a descriptor table entry: a region of the handle cache and which handles have been set.
	struct DescriptorTableCache {
		DescriptorTableCache() : AssignedHandlesBitMap(0) {}
		uint32_t AssignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE* TableStart;
		uint32_t TableSize;
	};

	// Cpu cache for discriptors.
	struct DescriptorHandleCache {
		DescriptorHandleCache() {
			ClearCache();
		}

		void ClearCache() {
			m_RootDescriptorTablesBitMap = 0;
			m_MaxCachedDescriptors = 0;
		}

		uint32_t m_RootDescriptorTablesBitMap;
		uint32_t m_StaleRootParamsBitMap;
		uint32_t m_MaxCachedDescriptors;

		static const uint32_t kMaxNumDescriptors = 256;
		static const uint32_t kMaxNumDescriptorTables = 16;

		DescriptorTableCache m_RootDescriptorTable[kMaxNumDescriptorTables];
		D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCache[kMaxNumDescriptors];

		void ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE Type, const RootSignature& RootSig);
		void StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		uint32_t ComputeStagedSize();
		void CopyAndBindStaleTables(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t DescriptorSize, DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
			void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));
		void UnbindAllValid();
	};

	DescriptorHandleCache m_GraphicsHandleCache;
	DescriptorHandleCache m_ComputeHandleCache;

	bool HasSpace(uint32_t Count) {
		return (m_CurrentHeapPtr != nullptr && m_CurrentOffset + Count <= kNumDescriptorsPerHeap);
	}

	void RetireCurrentHeap();
	void RetireUsedHeaps(uint64_t fenceValue);
	ID3D12DescriptorHeap* GetHeapPointer();

	DescriptorHandle Allocate(UINT Count) {
		DescriptorHandle ret = m_FirstDescriptor + m_CurrentOffset * m_DescriptorSize;
		m_CurrentOffset += Count;
		return ret;
	}

	// Upload heap through heap caches.
	void CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
		void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

	// Mark all descriptors in the cache as stale and in need of re-uploading.
	void UnbindAllValid();

};

}	// namespace Graphics.
