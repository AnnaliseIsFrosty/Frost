#pragma once
#include "FrostIncludes.h"

namespace Frost
{

	template<typename T>
	class DX12UploadBuffer
	{
	public:
		static UINT CalcConstantBufferByteSize(UINT byteSize) 
		{	// Constant buffers must be a multiple of the minimum hardware allocation size (usually 256 bytes)
			// So we round up to nearest multiple of 256 bytes by adding 255 then masking off the lower 2 bytes which store all bits < 256
			return (byteSize + 255) & ~255;
		}


		DX12UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : mIsConstantBuffer(isConstantBuffer)
		{
			mElementByteSize = sizeof(T);

			if (isConstantBuffer)
				mElementByteSize = CalcConstantBufferByteSize(mElementByteSize);

			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mUploadBuffer));
			assert(mUploadBuffer.Get());

			mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
			// We don't need to unmap until we're done with the resource, but we can't write to it until the gpu's done with it
		
			std::cout << "--- RENDERER ---  : Creating Upload Buffer" << std::endl;
		}

		DX12UploadBuffer(const DX12UploadBuffer& rhs) = delete;
		DX12UploadBuffer& operator=(const DX12UploadBuffer& rhs) = delete;

		DX12UploadBuffer()
		{
			if (mUploadBuffer != nullptr)
				mUploadBuffer->Unmap(0, nullptr);

			mMappedData = nullptr;
		}

		ID3D12Resource* Resource() const
		{
			return mUploadBuffer.Get();
		}

		void CopyData(int elementIndex, const T& data)
		{
			memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
		BYTE* mMappedData = nullptr;

		UINT mElementByteSize = 0;
		bool mIsConstantBuffer = false;
	};

}
