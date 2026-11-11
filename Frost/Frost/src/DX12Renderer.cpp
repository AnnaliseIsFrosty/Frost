#include "DX12Renderer.h"
//#include "../include/directx/d3dx12.h"



namespace Frost {
	
	
	Renderer* Renderer::Create(const void* hwnd)
	{
		return new DX12Renderer(hwnd);
	}

	DX12Renderer::DX12Renderer(const void* hwnd)
	{
		Init();
		mhMainWnd = (HWND)hwnd;
	}

	void DX12Renderer::Init()
	{
		std::cout << "INITIALISING RENDERER" << std::endl;

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

		// Create Swap Chain
		mClientWidth = 1280;
		mClientHeight = 720;
		CreateSwapChain();

		// Create Descriptor Heaps
		CreateRtvAndDsvDescriptorHeaps();

	}

	void DX12Renderer::Render()
	{

	}

	void DX12Renderer::CreateCommandObjects()
	{
		std::cout << "--- RENDERER ---  : Creating Command Objects" << std::endl;

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

	void DX12Renderer::CreateSwapChain()
	{
		std::cout << "--- RENDERER ---  : Creating Swap Chain" << std::endl;

		// Reset in case we had already created a swap chain previously
		mSwapChain.Reset();

		// Creating the descriptor for the swap chain
		DXGI_SWAP_CHAIN_DESC swapDesc;
		swapDesc.BufferDesc.Width = mClientWidth;
		swapDesc.BufferDesc.Height = mClientHeight;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.Format = mBackBufferFormat;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		swapDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.BufferCount = SwapChainBufferCount;
		swapDesc.OutputWindow = mhMainWnd;
		swapDesc.Windowed = true;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		

		// Using that descriptor to create the swap chain
		mdxgiFactory->CreateSwapChain(
			mCommandQueue.Get(),
			&swapDesc,
			mSwapChain.GetAddressOf());

	}

	void DX12Renderer::CreateRtvAndDsvDescriptorHeaps()
	{
		std::cout << "--- RENDERER ---  : Creating RTV and DSV Descriptor Heaps" << std::endl;

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf()));
		
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf()));
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::CurrentBackBufferView() const
	{
		//return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		//	mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		//	mCurrBackBuffer,
		//	mRtvDescriptorSize);
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::DepthStencilView() const
	{
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

}

