#pragma once
#include "Renderer.h"
#include "FrostIncludes.h"

namespace Frost
{

	class DX12Renderer : public Renderer
	{
	public:
		void Init() override;
		void Render() override;
		void CreateCommandObjects();

	protected:


	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;


		UINT mRtvDescriptorSize = 0; // Render Target resources
		UINT mDsvDescriptorSize = 0; // Depth/Stencil resources
		UINT mCbvSrvUavDescriptorSize = 0; // Constant buffers, Shader resources and Unordered Acces View resources

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // Each element has four 8-bit unsigned integer components mapped to the [0, 255] range
		UINT m4xMsaaQuality = 0;									// Quality Level of 4x MSAA

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	};

}