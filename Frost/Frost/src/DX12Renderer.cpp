#include "DX12Renderer.h"



namespace Frost {
	
	
	void DX12Renderer::Init()
	{
		// Create Factory
		CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)); // Used to create swap chain
		assert(mdxgiFactory);


		// Create Device
		HRESULT hardwareResult = D3D12CreateDevice(
			nullptr, // Default adaptor
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice));


		// Fallback to WARP device
		if (FAILED(hardwareResult))
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
			mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

			D3D12CreateDevice(
				pWarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&md3dDevice));
		}

		// Create Fence for synchronisation
		md3dDevice->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&mFence));

		// Descriptor Heap sizes (?)
		mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


		// Multisampling / Anti-aliasing
		msQualityLevels.Format = mBackBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		md3dDevice->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevels,
			sizeof(msQualityLevels));

		m4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level");


		// Create Command Queue and Command List
		CreateCommandObjects();

	}

	void DX12Renderer::Render()
	{

	}

	void DX12Renderer::CreateCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
		md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf()));
		md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
										mDirectCmdListAlloc.Get(),	// Associated command allocator
										nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf()));

		// Start off closed because the first time we refer to the 
		// command list we Reset() which requires it being closed first
		mCommandList->Close();
	}
}
