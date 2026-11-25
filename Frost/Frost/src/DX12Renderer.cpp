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
		// std::cout << "INITIALISING RENDERER" << std::endl;

#if defined(DEBUG) || defined(_DEBUG) 
		// Enable the D3D12 debug layer.
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
			assert(debugController.Get());
			debugController->EnableDebugLayer();
		}
#endif

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

		{
			// ---------------------------

			//// Create an RTV for each buffer in swap chain
			//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
			//for (UINT i = 0; i < SwapChainBufferCount; i++)
			//{
			//	// Get the ith buffer in the swap chain
			//	mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i]));

			//	// Create an RTV to it
			//	md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

			//	// Next entry in heap
			//	rtvHeapHandle.Offset(1, mRtvDescriptorSize);
			//}

			//// Create the depth/stencil buffer and view
			//D3D12_RESOURCE_DESC depthStencilDesc;
			//depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			//depthStencilDesc.Alignment = 0;
			//depthStencilDesc.Width = mClientWidth;
			//depthStencilDesc.Height = mClientHeight;
			//depthStencilDesc.DepthOrArraySize = 1;
			//depthStencilDesc.MipLevels = 1;
			//depthStencilDesc.Format = mDepthStencilFormat;
			//depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			//depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			//depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			//depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			//D3D12_CLEAR_VALUE optClear;
			//optClear.Format = mDepthStencilFormat;
			//optClear.DepthStencil.Depth = 1.f;
			//optClear.DepthStencil.Stencil = 0;
			//md3dDevice->CreateCommittedResource(
			//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			//	D3D12_HEAP_FLAG_NONE,
			//	&depthStencilDesc,
			//	D3D12_RESOURCE_STATE_COMMON,
			//	&optClear,
			//	IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
			//);
			//assert(mDepthStencilBuffer.Get());

			//// Create Descriptor to mip level 0 of entire resource using the format of the resource
			//md3dDevice->CreateDepthStencilView(
			//	mDepthStencilBuffer.Get(),
			//	nullptr,
			//	DepthStencilView()
			//);

			//// Transition the resource from its initial state to be used as a depth buffer
			//mCommandList->ResourceBarrier(
			//	1,
			//	&CD3DX12_RESOURCE_BARRIER::Transition(
			//		mDepthStencilBuffer.Get(),
			//		D3D12_RESOURCE_STATE_COMMON,
			//		D3D12_RESOURCE_STATE_DEPTH_WRITE)
			//);

			//// Create and set viewport
			//D3D12_VIEWPORT vp;
			//vp.TopLeftX = 0.0f;
			//vp.TopLeftY = 0.0f;
			//vp.Width = static_cast<float>(mClientWidth);
			//vp.Height = static_cast<float>(mClientHeight);
			//vp.MinDepth = 0.0f;
			//vp.MaxDepth = 1.0f;

			//mCommandList->RSSetViewports(1, &vp);
			//// The viewport needs to be reset whenever the command list is reset

			//// Set Scissor rectangle
			//mScissorRect = { 0, 0, mClientWidth, mClientHeight };
			//mCommandList->RSSetScissorRects(1, &mScissorRect);
		}
		
		// ------------------------------------------------------------
		
		mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr);


		CreateCbvDescriptorHeaps();
		CreateConstantBuffers();
		CreateRootSignature();

		mvsByteCode = CompileShader(L"../../../Frost/shaders/experiment_vs.hlsl", nullptr, "VS", "vs_5_0");
		mpsByteCode = CompileShader(L"../../../Frost/shaders/experiment_ps.hlsl", nullptr, "PS", "ps_5_0");
		//mvsByteCode = CompileShader(L"Frost/shaders/identity_vs.hlsl", nullptr, "VS", "vs_5_0");
		//mpsByteCode = CompileShader(L"Frost/shaders/identity_ps.hlsl", nullptr, "PS", "ps_5_0");

		mInputLayout =
		{
			{"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,           0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,           0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,           0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD[0]", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD[1]", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR",	    0, DXGI_FORMAT_R32G32B32A32_FLOAT,        0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		DrawStaticCube();

		CreatePSO();

		mCommandList->Close();
		
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();

		OnResize(mClientWidth, mClientHeight);
	}

	void DX12Renderer::Update(double time)
	{
		UpdateObjectCBs();

	}

	void DX12Renderer::Render(double time)
	{
		Update(time);



		// Reuse the memory associated with command recording
		// We can only reset when the associated command lists have finsihed execution on the GPU
		mDirectCmdListAlloc->Reset();

		// A command list can be reset after it has been added to the command queue via ExecuteCommandList
		// Reusing the command list reuses memory
		mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get());

		// Convert Spherical to Cartesian coordinates.
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float z = mRadius * sinf(mPhi) * sinf(mTheta);
		float y = mRadius * cosf(mPhi);

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		mView = view;

		XMMATRIX world = mWorld;
		XMMATRIX proj = mProj;
		XMMATRIX worldViewProj = world * view * proj;

		// Update the constant buffer with the latest worldViewProj matrix.
		ObjectConstants objConstants;
		objConstants.WorldViewProj = XMMatrixTranspose(worldViewProj);
	
		objConstants.gTime = XMFLOAT4(time, 0, 0, 0);
		objConstants.pulseColor = XMFLOAT4(Colors::Aquamarine);
		mObjectCB->CopyData(0, objConstants);
		
		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);

		// Indicate a state transition on the resource usage
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// Clear the back buffer and depth buffer
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
		//std::cout << mBoxGeo->VertexBufferView().BufferLocation << std::endl;
		mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
		//std::cout << mBoxGeo->IndexBufferView().BufferLocation << std::endl;
		mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

		mCommandList->DrawIndexedInstanced(mBoxGeo->DrawArgs["box"].IndexCount, 1, 0, 0, 0);

		// Indicate a state transition on the resource usage
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// Done recording commands
		mCommandList->Close();

		// Add the command list tot he queue for execution
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Swap the back and front buffers
		mSwapChain->Present(0, 0);
		mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

		// Wait until frame commands are complete (inefficient, for simplicity)
		FlushCommandQueue();
		//mBoxGeo->DisposeUploaders();

	}

	void DX12Renderer::OnResize(float width, float height)
	{
		// std::cout << "--- RENDERER ---  : Window Resized" << std::endl;
		
		// Flush before changing any resources
		FlushCommandQueue();

		mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr);

		mClientWidth = width;
		mClientHeight = height;

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

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, width/height, 1.0f, 1000.0f);
		mProj = P;

		// std::cout << "--- RENDERER ---  : Finished Resizing Window" << std::endl;
	}

	void DX12Renderer::CreateCommandObjects()
	{
		// std::cout << "--- RENDERER ---  : Creating Command Objects" << std::endl;

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
		// std::cout << "--- RENDERER ---  : Creating Swap Chain" << std::endl;

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
		assert(mSwapChain.Get());
	}

	void DX12Renderer::CreateRtvAndDsvDescriptorHeaps()
	{
		// std::cout << "--- RENDERER ---  : Creating RTV and DSV Descriptor Heaps" << std::endl;

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf()));
		assert(mRtvHeap.Get());

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf()));
		assert(mDsvHeap.Get());
	}

	ID3D12Resource* DX12Renderer::CurrentBackBuffer() const
	{
		return mSwapChainBuffer[mCurrBackBuffer].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::CurrentBackBufferView() const
	{
		// std::cout << "--- RENDERER ---  : Getting back buffer view" << std::endl;
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
		// std::cout << "--- RENDERER ---  : Command Queue Flushed" << std::endl;

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
		
		// std::cout << "--- RENDERER ---  : Created Upload Buffer" << std::endl;

		return defaultBuffer;
	}

	void DX12Renderer::CreateCbvDescriptorHeaps()
	{
		// Create Descriptor heaps ----------------------------------------------

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap));
		assert(mCbvHeap.Get());
		
		// std::cout << "--- RENDERER ---  : Created Descriptor Heaps" << std::endl;
	}

	void DX12Renderer::CreateConstantBuffers()
	{
		// Create Constant Buffers ----------------------------------------------

		//ObjectConstants objConstants;

		// Constant buffer to store the constants of n object
		mObjectCB = std::make_unique<DX12UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

		UINT objectCBByteSize = DX12UploadBuffer<ObjectConstants>::CalcConstantBufferByteSize(sizeof(ObjectConstants));

		// Address to start of the buffer (0th constant buffer)
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

		// Offset to the ith object constant buffer in the buffer
		int boxCBufIndex = 0; // i
		cbAddress += boxCBufIndex * objectCBByteSize;

		// Describes subset of the constant buffer resource to bind to the HLSL constant buffer structure
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = objectCBByteSize;

		md3dDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());

		// std::cout << "--- RENDERER ---  : Created CBV" << std::endl;

	}

	void DX12Renderer::CreateRootSignature()
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CBVs.
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		HRESULT hr2 = md3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature));
		HRESULT hr3 = md3dDevice->GetDeviceRemovedReason();
		assert(mRootSignature.Get());
	}

	Microsoft::WRL::ComPtr<ID3DBlob> DX12Renderer::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
	{
		UINT compileFlags = 0;

//#if defined(DEBUG) || defined(_DEBUG)
//		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#endif

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errors;

		hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

		if (FAILED(hr))
		{
			// std::cout << "Failed to compile shader" << std::endl;
		}

		// Output errors to debug window
		if (errors != nullptr)
			OutputDebugStringA((char*)errors->GetBufferPointer());

		assert(byteCode);

		// std::cout << "--- RENDERER ---  : Finished Compiling Shader code" << std::endl;

		return byteCode;
	}


	void DX12Renderer::DrawStaticCube()
	{

		// GEOMETRY

		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		std::array<Vertex, 8> vertices = 
		{ 
			Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(1, 1, 1, 1) }),
			Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(0, 0, 0, 1) }),
			Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(1, 0, 0, 1) }),
			Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(0, 1, 0, 1) }),
			Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(0, 0, 1, 1) }),
			Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(1, 1, 0, 1) }),
			Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(0, 1, 1, 1) }),
			Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(-1.0f, -1.0f), XMFLOAT4(1, 0, 1, 1) }) 
		};

		std::array<std::uint16_t, 36> indices =
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

		const UINT vertexCount = (UINT)vertices.size();
		const UINT vbByteSize = vertexCount * sizeof(Vertex);
		
		const UINT indexCount = (UINT)indices.size();
		const UINT ibByteSize = indexCount * sizeof(std::uint16_t);

		mBoxGeo = std::make_unique<MeshGeometry>();
		mBoxGeo->Name = "boxGeo";

		D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU);
		assert(mBoxGeo->VertexBufferCPU.Get());
		CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
		
		D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU);
		assert(mBoxGeo->IndexBufferCPU.Get());
		CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		mBoxGeo->VertexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);
		mBoxGeo->IndexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

		mBoxGeo->VertexByteStride = sizeof(Vertex);
		mBoxGeo->VertexBufferByteSize = vbByteSize;
		mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
		mBoxGeo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = indexCount;
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		mBoxGeo->DrawArgs["box"] = submesh;

		{
			//Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
			//Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
			//VertexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices, vbByteSize, VertexBufferUploader);

			//D3D12_VERTEX_BUFFER_VIEW vbv;
			//vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
			//vbv.StrideInBytes = sizeof(Vertex);
			//vbv.SizeInBytes = vbByteSize;

			//D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vbv };
			//mCommandList->IASetVertexBuffers(0, 1, vertexBuffers);

			//mCommandList->DrawInstanced(vertexCount, 1, startVertexLocation, 0);


			// INDICES 

			//std::uint16_t indices[] = 
			//{   
			//	// front face   
			//	0, 1, 2,   
			//	0, 2, 3,     
			//	// back face   
			//	4, 6, 5,   
			//	4, 7, 6,     
			//	// left face   
			//	4, 5, 1,   
			//	4, 1, 0,     
			//	// right face   
			//	3, 2, 6,   
			//	3, 6, 7,     
			//	// top face
			//	1, 5, 6,   
			//	1, 6, 2,     
			//	// bottom face   
			//	4, 0, 3,   
			//	4, 3, 7 
			//};

			//const UINT indexCount = 36;
			//const UINT ibByteSize = indexCount * sizeof(std::uint16_t);
			//UINT startIndexLocation = 0;

			//Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
			//Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
			//IndexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices, ibByteSize, IndexBufferUploader);

			//D3D12_INDEX_BUFFER_VIEW ibv;
			//ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
			//ibv.Format = DXGI_FORMAT_R16_UINT;
			//ibv.SizeInBytes = ibByteSize;

			//mCommandList->IASetIndexBuffer(&ibv);

			//mCommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, startVertexLocation, 0);


			// SHADER
		}

		// std::cout << "--- RENDERER ---  : Finished Drawing Cube" << std::endl;

		
	}

	void DX12Renderer::CreatePSO()
	{
		// ---- PSO ----

		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = { reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), mvsByteCode->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), mpsByteCode->GetBufferSize() };
		CD3DX12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
#ifdef WIREFRAME
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
#endif
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = mBackBufferFormat;
		psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = mDepthStencilFormat;

		md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO));

		// std::cout << "--- RENDERER ---  : Finished Creating PSO" << std::endl;

	}




	void DX12Renderer::CreateFrameResources()
	{
		for (int i = 0; i < gNumFrameResources; ++i)
		{
			mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 1, (UINT)mAllRitems.size()));
		}
	}

	void DX12Renderer::FrameResourcesUpdate()
	{
		// Cycle through the circular frame resource array
		mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
		mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
	
		// Has the GPU finished processing the commands of the current frame resource? If not, wait until the GPU has completed commands up to this fence point
		if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

		// Update resources in mCurrFrameResource (like cbuffers)
	}


	void DX12Renderer::UpdateObjectCBs()
	{
		//auto currObjectCB = mCurrFrameResource->ObjectCB.get();
		//for (auto& e : mAllRitems)
		//{
		//	// Only update the cbuffer data if the constants have changed
		//	// Needs to be tracked per frame resource
		//	if (e->GetNumFramesDirty() > 0)
		//	{
		//		XMMATRIX world = e->GetWorldMatrix();

		//		ObjectConstants objConstants;
		//		objConstants.World = world;

		//		currObjectCB->CopyData(e->GetObjCBIndex(), objConstants);

		//		// Next FrameResource needs to be updated too
		//		e->DecrementNumFramesDirty();
		//	}
		//}
	}

	void DX12Renderer::UpdateMainPassCBs()
	{
		XMMATRIX view = mView;
		XMMATRIX proj = mProj;

		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		mMainPassCB.View = view;
		mMainPassCB.InvView = invView;
		mMainPassCB.Proj = proj;
		mMainPassCB.InvProj = invProj;
		mMainPassCB.ViewProj = viewProj;
		mMainPassCB.InvViewProj = invViewProj;
		mMainPassCB.EyePosW = mEyePos;
	}
}

