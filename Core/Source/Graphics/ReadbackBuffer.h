//
// GPU buffer for readback purpose.
//

#pragma once

#include "GpuBuffer.h"

namespace Graphics {
	
class ReadbackBuffer : public GpuBuffer {
public:
	virtual ~ReadbackBuffer() { GpuResource::Destroy(); }
	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize);
	void* Map();
	void Unmap();

protected:
	void CreateDerivedViews() {}
};

}	// namespace Graphics