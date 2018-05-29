//
// Provide texture related utilities to easy load, create and manage.
//

#include "pch.h"
#include "TextureManager.h"
#include "../Core/Utility.h"
#include "../Core/FileUtility.h"
#include "DDSTextureLoader.h"
#include "CommandContext.h"
#include "GraphicsCore.h"
#include <map>

namespace Graphics {

using namespace Core;

// Global ID3D12Device.
extern ID3D12Device* g_Device;

static UINT BytesPerPixel(DXGI_FORMAT Format) {
	return (UINT)BitsPerPixel(Format) / 8;
}

//
// Texture implementation
//

void Texture::Create(size_t Pitch, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData) {
	m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = Width;
	texDesc.Height = (UINT)Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		m_UsageState, nullptr, MY_IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())));
	m_pResource->SetName(L"Texture");

	D3D12_SUBRESOURCE_DATA texResource;
	texResource.pData = InitialData;
	texResource.RowPitch = Pitch * BytesPerPixel(Format);
	texResource.SlicePitch = texResource.RowPitch * Height;

	CommandContext::InitializeTexture(*this, 1, &texResource);

	if (m_hCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		m_hCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateShaderResourceView(m_pResource.Get(), nullptr, m_hCpuDescriptorHandle);
}

void Texture::CreateTGAFromMemory(const void* _filePtr, size_t, bool sRGB) {
	const uint8_t* filePtr = (const uint8_t*)_filePtr;

	// Skip first two bytes
	filePtr += 2;

	/*uint8_t imageTypeCode =*/ *filePtr++;

	// Ignore another 9 bytes
	filePtr += 9;

	uint16_t imageWidth = *(uint16_t*)filePtr;
	filePtr += sizeof(uint16_t);
	uint16_t imageHeight = *(uint16_t*)filePtr;
	filePtr += sizeof(uint16_t);
	uint8_t bitCount = *filePtr++;

	// Ignore another byte
	filePtr++;

	uint32_t* formattedData = new uint32_t[imageWidth * imageHeight];
	uint32_t* iter = formattedData;

	uint8_t numChannels = bitCount / 8;
	uint32_t numBytes = imageWidth * imageHeight * numChannels;

	switch (numChannels) {
	default:
		break;
	case 3:
		for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 3) {
			*iter++ = 0xff000000 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
			filePtr += 3;
		}
		break;
	case 4:
		for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 4) {
			*iter++ = filePtr[3] << 24 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
			filePtr += 4;
		}
		break;
	}

	Create(imageWidth, imageHeight, sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, formattedData);

	delete[] formattedData;
}

bool Texture::CreateDDSFromMemory(const void* filePtr, size_t fileSize, bool sRGB) {
	if (m_hCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		m_hCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	HRESULT hr = CreateDDSTextureFromMemory(g_Device,
		(const uint8_t*)filePtr, fileSize, 0, sRGB, &m_pResource, m_hCpuDescriptorHandle);

	return SUCCEEDED(hr);
}

void Texture::CreatePIXImageFromMemory(const void* memBuffer, size_t fileSize) {
	struct Header {
		DXGI_FORMAT Format;
		uint32_t Pitch;
		uint32_t Width;
		uint32_t Height;
	};
	const Header& header = *(Header*)memBuffer;

	ASSERT(fileSize >= header.Pitch * BytesPerPixel(header.Format) * header.Height + sizeof(Header),
		"Raw PIX image dump has an invalid file size");

	Create(header.Pitch, header.Width, header.Height, header.Format, (uint8_t*)memBuffer + sizeof(Header));
}

//
// ManagedTexture implementation
//

void ManagedTexture::WaitForLoad() const {
	volatile D3D12_CPU_DESCRIPTOR_HANDLE& VolHandle = (volatile D3D12_CPU_DESCRIPTOR_HANDLE&)m_hCpuDescriptorHandle;
	volatile bool& VolValid = (volatile bool&)m_IsValid;
	while (VolHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN && VolValid)
		std::this_thread::yield();
}

void ManagedTexture::SetToInvalidTexture() {
	m_hCpuDescriptorHandle = TextureManager::GetMagentaTex2D().GetSRV();
	m_IsValid = false;
}

//
// TextureManager implementation
//

namespace TextureManager {

using namespace std;

wstring s_RootPath = L"";
map<wstring, unique_ptr<ManagedTexture>> s_TextureCache;

void Initialize(const std::wstring& TextureLibRoot) {
	s_RootPath = TextureLibRoot;
}

void Shutdown() {
	s_TextureCache.clear();
}

pair<ManagedTexture*, bool> FindOrLoadTexture(const wstring& fileName) {
	static mutex s_Mutex;
	lock_guard<mutex> Guard(s_Mutex);

	auto iter = s_TextureCache.find(fileName);

	// If it's found, it has already been loaded or the load process has begun
	if (iter != s_TextureCache.end())
		return make_pair(iter->second.get(), false);

	ManagedTexture* NewTexture = new ManagedTexture(fileName);
	s_TextureCache[fileName].reset(NewTexture);

	// This was the first time it was requested, so indicate that the caller must read the file
	return make_pair(NewTexture, true);
}

const ManagedTexture* LoadFromFile(const std::wstring& fileName, bool sRGB) {
	std::wstring CatPath = fileName;
	const ManagedTexture* Tex = LoadDDSFromFile(CatPath + L".dds", sRGB);
	if (!Tex->IsValid())
		Tex = LoadTGAFromFile(CatPath + L".tga", sRGB);
	return Tex;
}

const ManagedTexture* LoadDDSFromFile(const std::wstring& fileName, bool sRGB) {
	auto ManagedTex = FindOrLoadTexture(fileName);
	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad) {
		ManTex->WaitForLoad();
		return ManTex;
	}

	Core::ByteArray ba = Core::ReadFileSync(s_RootPath + fileName);
	if (ba->size() == 0 || !ManTex->CreateDDSFromMemory(ba->data(), ba->size(), sRGB))
		ManTex->SetToInvalidTexture();
	else
		ManTex->GetResource()->SetName(fileName.c_str());

	return ManTex;
}

const ManagedTexture* LoadTGAFromFile(const std::wstring& fileName, bool sRGB) {
	auto ManagedTex = FindOrLoadTexture(fileName);
	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad) {
		ManTex->WaitForLoad();
		return ManTex;
	}

	Core::ByteArray ba = Core::ReadFileSync(s_RootPath + fileName);
	if (ba->size() > 0) {
		ManTex->CreateTGAFromMemory(ba->data(), ba->size(), sRGB);
		ManTex->GetResource()->SetName(fileName.c_str());
	} else
		ManTex->SetToInvalidTexture();

	return ManTex;
}

const ManagedTexture* LoadPIXImageFromFile(const std::wstring& fileName) {
	auto ManagedTex = FindOrLoadTexture(fileName);
	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad) {
		ManTex->WaitForLoad();
		return ManTex;
	}

	Core::ByteArray ba = Core::ReadFileSync(s_RootPath + fileName);
	if (ba->size() > 0) {
		ManTex->CreatePIXImageFromMemory(ba->data(), ba->size());
		ManTex->GetResource()->SetName(fileName.c_str());
	} else
		ManTex->SetToInvalidTexture();

	return ManTex;
}

const Texture& GetBlackTex2D() {
	auto ManagedTex = FindOrLoadTexture(L"DefaultBlackTexture");
	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad) {
		ManTex->WaitForLoad();
		return *ManTex;
	}

	uint32_t BlackPixel = 0;
	ManTex->Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackPixel);
	return *ManTex;
}

const Texture& GetWhiteTex2D() {
	auto ManagedTex = FindOrLoadTexture(L"DefaultWhiteTexture");
	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad) {
		ManTex->WaitForLoad();
		return *ManTex;
	}

	uint32_t WhitePixel = 0xFFFFFFFFul;
	ManTex->Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhitePixel);
	return *ManTex;
}

const Texture& GetMagentaTex2D() {
	auto ManagedTex = FindOrLoadTexture(L"DefaultMagentaTexture");

	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad) {
		ManTex->WaitForLoad();
		return *ManTex;
	}

	uint32_t MagentaPixel = 0x00FF00FF;
	ManTex->Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &MagentaPixel);
	return *ManTex;
}

}	// namespace TextureManager

}	// namespace Graphics