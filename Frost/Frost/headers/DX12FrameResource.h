#pragma once
#include "FrostIncludes.h"
#include "DX12UploadBuffer.h"
#include "DX12CBContents.h"

namespace Frost
{

	class FrameResource
	{
	public :
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;
		FrameResource();

		// We cannot reset the allocator until the GPU is done processing the command so each frame needs their own allocator
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// We cannot update a cbuffer until the GPU is done processing the commands that referenceit so each frame needs their own cbuffers
		std::unique_ptr<DX12UploadBuffer<PassConstants>> PassCB = nullptr;
		std::unique_ptr<DX12UploadBuffer<ObjectConstants>> ObjectCB = nullptr;


		// Fence value to mark commands up to this fence point, this lets us check if these frame resources are still in use by the GPU
		UINT64 Fence = 0;
	};

}