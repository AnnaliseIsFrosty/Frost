#pragma once
#include "Renderer.h"
#include "FrostIncludes.h"

namespace Frost
{

	class DX12Renderer : public Renderer
	{
	public:
		DX12Renderer(void* hwnd);
		void Init() override;
		void Render() override;
		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateRtvAndDsvDescriptorHeaps();
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	protected:


	private:
		HWND mhMainWnd = nullptr;										// Main Window handle

		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

		UINT mRtvDescriptorSize = 0;									// Render Target resources
		UINT mDsvDescriptorSize = 0;									// Depth/Stencil resources
		UINT mCbvSrvUavDescriptorSize = 0;								// Constant buffers, Shader resources and Unordered Acces View resources

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;		// Each element has four 8-bit unsigned integer components mapped to the [0, 255] range
		bool m4xMsaaState = false;										// 4X MSAA enabled
		UINT m4xMsaaQuality = 0;										// Quality Level of 4x MSAA

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
		int mClientWidth, mClientHeight;

		static const int SwapChainBufferCount = 2;
		int mCurrBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;			// Render Target heap
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;			// Depth/Stencil heap

		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	};

}