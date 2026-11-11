#include "DX12Renderer.h"
#include "../include/directx/d3dx12.h"



namespace Frost {
	
	
	Renderer* Renderer::Create(void* hwnd)
	{
		return new DX12Renderer(hwnd);
	}

	DX12Renderer::DX12Renderer(void* hwnd)
	{
		mhMainWnd = (HWND)hwnd;
		Init();
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


		// Create an RTV for each buffer in swap chain
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < SwapChainBufferCount; i++)
		{
			// Get the ith buffer in the swap chain
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i]));

			// Create an RTV to it
			md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

			// Next entry in heap
			rtvHeapHandle.Offset(1, mRtvDescriptorSize);
		}

		// Create the depth/stencil buffer and view
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = mClientWidth;
		depthStencilDesc.Height = mClientHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = mDepthStencilFormat;
		depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = mDepthStencilFormat;
		optClear.DepthStencil.Depth = 1.f;
		optClear.DepthStencil.Stencil = 0;
		md3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
		);
		assert(mDepthStencilBuffer.Get());

		// Create Descriptor to mip level 0 of entire resource using the format of the resource
		md3dDevice->CreateDepthStencilView(
			mDepthStencilBuffer.Get(),
			nullptr,
			DepthStencilView()
		);

		// Transition the resource from its initial state to be used as a depth buffer
		mCommandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				mDepthStencilBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE)
		);

		// Create and set viewport
		D3D12_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(mClientWidth);
		vp.Height = static_cast<float>(mClientHeight);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		mCommandList->RSSetViewports(1, &vp);
		// The viewport needs to be reset whenever the command list is reset

		// Set Scissor rectangle
		mScissorRect = { 0, 0, mClientWidth, mClientHeight };
		mCommandList->RSSetScissorRects(1, &mScissorRect);
	}

	void DX12Renderer::Render()
	{

	}

	void DX12Renderer::OnResize()
	{
		std::cout << "--- RENDERER ---  : Window Resized" << std::endl;
		
		// Flush before changing any resources
		FlushCommandQueue();

		mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr);

		// Release the previous resources we will be recreating
		for (int i = 0; i < SwapChainBufferCount; i++)
		{
			mSwapChainBuffer[i].Reset();
		}
		mDepthStencilBuffer.Reset();

		// Resize the swap chain
		mSwapChain->ResizeBuffers(
			SwapChainBufferCount,
			mClientWidth,
			mClientHeight,
			mBackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		mCurrBackBuffer = 0;

		// Create an RTV for each buffer in swap chain
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < SwapChainBufferCount; i++)
		{
			// Get the ith buffer in the swap chain
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i]));

			// Create an RTV to it
			md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

			// Next entry in heap
			rtvHeapHandle.Offset(1, mRtvDescriptorSize);
		}

		// Create the depth/stencil buffer and view
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = mClientWidth;
		depthStencilDesc.Height = mClientHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = mDepthStencilFormat;
		depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		
		std::cout << "--- RENDERER ---  : Finished Resizing Window" << std::endl;
	}

	void DX12Renderer::CreateCommandObjects()
	{
		std::cout << "--- RENDERER ---  : Creating Command Objects" << std::endl;

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
		assert(mCommandQueue.Get());
		md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf()));
		assert(mDirectCmdListAlloc.Get());
		md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
										mDirectCmdListAlloc.Get(),	// Associated command allocator
										nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf()));
		assert(mCommandList.Get());

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
		std::cout << "--- RENDERER ---  : Getting back buffer view" << std::endl;
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrBackBuffer,
			mRtvDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::DepthStencilView() const
	{
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void DX12Renderer::FlushCommandQueue()
	{
		std::cout << "--- RENDERER ---  : Command Queue Flushed" << std::endl;

		mCurrentFence++;

		// Add instruction to command queue to set a new fence point
		// New fence point won't be set until GPU finishes processing all commands prior to this Signal()
		mCommandQueue->Signal(mFence.Get(), mCurrentFence);

		// Wait until GPU has completed commands up to this fence point
		if (mFence->GetCompletedValue() < mCurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

			// Fire event when GPU hits current fence
			mFence->SetEventOnCompletion(mCurrentFence, eventHandle);

			// Wait until the GPU hits current fence event is fired
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

}

