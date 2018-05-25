//
// Provide context helper methods for command, graphic and compute work.
//

#pragma once

#include "CommandListManager.h"
#include "Color.h"
#include "RootSignature.h"
#include "GpuBuffer.h"
#include "PixelBuffer.h"
#include "DynamicDescriptorHeap.h"
#include "LinearAllocator.h"

#include <vector>

namespace Graphics {

// Forward declaration.
class CommandContext;
class GraphicsContext;
class ComputeContext;
class PixelBuffer;

struct DWParam {
	DWParam(FLOAT f) : Float(f) {}
	DWParam(UINT u) : Uint(u) {}
	DWParam(INT i) : Int(i) {}

	void operator= (FLOAT f) { Float = f; }
	void operator= (UINT u) { Uint = u; }
	void operator= (INT i) { Int = i; }

	union {
		FLOAT Float;
		UINT Uint;
		INT Int;
	};
};

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

class ContextManager {
public:
	ContextManager() {}
	CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE Type);
	void FreeContext(CommandContext*);
	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<CommandContext>> sm_ContextPool[4];
	std::queue<CommandContext*> sm_AvailableContexts[4];
	std::mutex sm_ContextAllocationMutex;
};

struct NonCopyable {
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable & operator=(const NonCopyable&) = delete;
};

class CommandContext : NonCopyable {
	friend ContextManager;

private:
	CommandContext(D3D12_COMMAND_LIST_TYPE Type);
	void Reset();

public:
	~CommandContext();

	static void DestroyAllContexts();

	static CommandContext& Begin(const std::wstring ID = L"");

	// Flush existing commands to the GPU but keep the context alive
	uint64_t Flush(bool WaitForCompletion = false);

	// Flush existing commands and release the current context
	uint64_t Finish(bool WaitForCompletion = false);

	// Prepare to render by reserving a command list and command allocator
	void Initialize();

	GraphicsContext& GetGraphicsContext() {
		ASSERT(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
		return reinterpret_cast<GraphicsContext&>(*this);
	}

	ComputeContext& GetComputeContext() {
		return reinterpret_cast<ComputeContext&>(*this);
	}

	ID3D12GraphicsCommandList* GetCommandList() {
		return m_CommandList;
	}

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate = false);
	void InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate = false);
	inline void FlushResourceBarriers();

	void CopyBuffer(GpuResource& Dest, GpuResource& Src);
	void CopyBufferRegion(GpuResource& Dest, size_t DestOffset, GpuResource& Src, size_t SrcOffset, size_t NumBytes);
	void CopySubresource(GpuResource& Dest, UINT DestSubIndex, GpuResource& Src, UINT SrcSubIndex);
	void CopyCounter(GpuResource& Dest, size_t DestOffset, StructuredBuffer& Src);
	void ResetCounter(StructuredBuffer& Buf, uint32_t Value = 0);

	void WriteBuffer(GpuResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes);
	void FillBuffer(GpuResource& Dest, size_t DestOffset, DWParam Value, size_t NumBytes);

	DynAlloc ReserveUploadMemory(size_t SizeInBytes) {
		return m_CpuLinearAllocator.Allocate(SizeInBytes);
	}

	static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);
	static void InitializeBuffer(GpuResource& Dest, const void* Data, size_t NumBytes, size_t Offset = 0);
	static void InitializeTextureArraySlice(GpuResource& Dest, UINT SliceIndex, GpuResource& Src);
	static void ReadbackTexture2D(GpuResource& ReadbackBuffer, PixelBuffer& SrcBuffer);

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);

	void SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op);

	void InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx);
	void ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries);
	void PIXBeginEvent(const wchar_t* label);
	void PIXEndEvent();
	void PIXSetMarker(const wchar_t* label);

protected:
	void BindDescriptorHeaps();

	CommandListManager* m_OwningManager;
	ID3D12GraphicsCommandList* m_CommandList;
	ID3D12CommandAllocator* m_CurrentAllocator;

	ID3D12RootSignature* m_CurGraphicsRootSignature;
	ID3D12PipelineState* m_CurGraphicsPipelineState;
	ID3D12RootSignature* m_CurComputeRootSignature;
	ID3D12PipelineState* m_CurComputePipelineState;

	DynamicDescriptorHeap m_DynamicViewDescriptorHeap;		// HEAP_TYPE_CBV_SRV_UAV
	DynamicDescriptorHeap m_DynamicSamplerDescriptorHeap;	// HEAP_TYPE_SAMPLER

	D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
	UINT m_NumBarriersToFlush;

	ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator m_CpuLinearAllocator;
	LinearAllocator m_GpuLinearAllocator;

	std::wstring m_ID;
	void SetID(const std::wstring& ID) { m_ID = ID; }

	D3D12_COMMAND_LIST_TYPE m_Type;
};

inline void CommandContext::FlushResourceBarriers() {
	if (m_NumBarriersToFlush > 0) {
		m_CommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
		m_NumBarriersToFlush = 0;
	}
}

inline void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr) {
	if (m_CurrentDescriptorHeaps[Type] != HeapPtr) {
		m_CurrentDescriptorHeaps[Type] = HeapPtr;
		BindDescriptorHeaps();
	}
}

inline void CommandContext::SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]) {
	bool AnyChanged = false;
	for (UINT i = 0; i < HeapCount; ++i) {
		if (m_CurrentDescriptorHeaps[Type[i]] != HeapPtrs[i]) {
			m_CurrentDescriptorHeaps[Type[i]] = HeapPtrs[i];
			AnyChanged = true;
		}
	}
	if (AnyChanged)
		BindDescriptorHeaps();
}

inline void CommandContext::SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op) {
	m_CommandList->SetPredication(Buffer, BufferOffset, Op);
}

inline void CommandContext::InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx) {
	m_CommandList->EndQuery(pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
}

inline void CommandContext::ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries) {
	m_CommandList->ResolveQueryData(pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, pReadbackHeap, 0);
}

}	// namespace Graphics.