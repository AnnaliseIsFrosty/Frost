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
		
		D3D12_CLEAR_VALUE optClear;
		optClear.Format = mDepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		md3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = mDepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

		// Transition the resource from its initial state to be used as a depth buffer.
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		// Execute the resize commands.
		mCommandList->Close();
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		FlushCommandQueue();

		// Update the viewport transform to cover the client area.
		mScreenViewport.TopLeftX = 0;
		mScreenViewport.TopLeftY = 0;
		mScreenViewport.Width = static_cast<float>(mClientWidth);
		mScreenViewport.Height = static_cast<float>(mClientHeight);
		mScreenViewport.MinDepth = 0.0f;
		mScreenViewport.MaxDepth = 1.0f;

		mScissorRect = { 0, 0, mClientWidth, mClientHeight };

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

	Microsoft::WRL::ComPtr<ID3D12Resource> DX12Renderer::CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
																			const void* initData, UINT64 byteSize,
																			Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

		// Create the actual default buffer resource (read-only from the gpu, for static meshes basically)
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf()));
		assert(defaultBuffer.Get());

		// Create intermediate upload heap (CPU can't write to the default buffer directly, has to go through upload heap first)
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf()));
		assert(uploadBuffer.Get());

		// Describe the data we want to copy into the default buffer
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = byteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data from the upload heap to the default buffer
		// The helper function UpdateSubresources will copy the CPU memory into the intermediate upload heap
		// Then, using ID3D12CommandList::CopySubresourceRegion, the intermediate upload heap data will be copied to mBuffer
		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST));
		
		UpdateSubresources<1>(cmdList,
			defaultBuffer.Get(), uploadBuffer.Get(),
			0, 0, 1, &subResourceData);

		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ));

		// Note : uploadBuffer has to be kept alive after the above function calls because the command list has not been 
		// executed yet that performs the actual copy 
		// The caller can Release the uploadBuffer after it knows the copy has been executed

		return defaultBuffer;
	}



	void DX12Renderer::DrawStaticCube()
	{
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		// VERTICES 

		Vertex vertices[] = 
		{ 
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1, 1, 1, 1) },
			{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0, 0, 0, 1) },
			{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(1, 0, 0, 1) },
			{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(0, 1, 0, 1) },
			{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(0, 0, 1, 1) },
			{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(1, 1, 0, 1) },
			{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(0, 1, 1, 1) },
			{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 1, 1) } 
		};

		const UINT vertexCount = 8;
		const UINT vbByteSize = vertexCount * sizeof(Vertex);
		UINT startVertexLocation = 0; // Location in the global vertex buffer that this cube's vertices start

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
		VertexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices, vbByteSize, VertexBufferUploader);

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes = vbByteSize;

		D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vbv };
		mCommandList->IASetVertexBuffers(0, 1, vertexBuffers);
	
		mCommandList->DrawInstanced(vertexCount, 1, startVertexLocation, 0);

		// INDICES 
	
		std::uint16_t indices[] = 
		{   
			// front face   
			0, 1, 2,   
			0, 2, 3,     
			// back face   
			4, 6, 5,   
			4, 7, 6,     
			// left face   
			4, 5, 1,   
			4, 1, 0,     
			// right face   
			3, 2, 6,   
			3, 6, 7,     
			// top face
			1, 5, 6,   
			1, 6, 2,     
			// bottom face   
			4, 0, 3,   
			4, 3, 7 
		};

		const UINT indexCount = 36;
		const UINT ibByteSize = indexCount * sizeof(std::uint16_t);
		UINT startIndexLocation = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
		IndexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices, ibByteSize, IndexBufferUploader);

		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = DXGI_FORMAT_R16_UINT;
		ibv.SizeInBytes = ibByteSize;

		mCommandList->IASetIndexBuffer(&ibv);
	
		mCommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, startVertexLocation, 0);

	}
}

