#include "DX12FrameResource.h"
#include "../include/directx/d3dx12.h"

namespace Frost
{

	FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf()));
		assert(CmdListAlloc.Get());

		PassCB = std::make_unique<DX12UploadBuffer<PassConstants>>(device, passCount, true);
		ObjectCB = std::make_unique<DX12UploadBuffer<ObjectConstants>>(device, objectCount, true);
	}

	FrameResource::FrameResource() { }

}