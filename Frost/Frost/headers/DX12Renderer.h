#pragma once
#include "Renderer.h"
#include "FrostIncludes.h"
#include "DX12UploadBuffer.h"
#include "DX12CBContents.h"
#include "DX12FrameResource.h"

using namespace DirectX;

#define _DEBUG

//#define WIREFRAME

namespace Frost
{
	struct Vertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Tangent;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex0;
		XMFLOAT2 Tex1;
		XMFLOAT4 Color;
	};
 
	struct SubmeshGeometry
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;

		// Bounding box of the geometry defined by this submesh
		DirectX::BoundingBox Bounds;
	};

	struct MeshGeometry
	{
		std::string Name;

		Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

		// Data about the buffers
		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		UINT IndexBufferByteSize = 0;

		// MeshGeometry may store multiple geometries in one vertex/index buffer
		// Use this container to define the Submesh geometries so we can draw them individually
		std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = VertexByteStride;
			vbv.SizeInBytes = VertexBufferByteSize;

			return vbv;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
			ibv.Format = IndexFormat;
			ibv.SizeInBytes = IndexBufferByteSize;

			return ibv;
		}

		void DisposeUploaders()
		{
			VertexBufferUploader = nullptr;
			IndexBufferUploader = nullptr;
		}
	};

	class DX12Renderer : public Renderer
	{
	public:
		DX12Renderer(void* hwnd);
		void Init() override;
		void Render(double time) override;
		void OnResize(float width, float height) override;
		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateRtvAndDsvDescriptorHeaps();
		void FlushCommandQueue();
		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
																	const void* initData, UINT64 byteSize, 
																	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
		
		void CreateCbvDescriptorHeaps();
		void CreateConstantBuffers();
		void CreateRootSignature();
		Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);
		void DrawStaticCube();
		void CreatePSO();

	protected:


	private:
		HWND mhMainWnd = nullptr;										// Main Window handle

		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
		
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
		UINT64 mCurrentFence = 0;

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
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;

		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
		int mClientWidth, mClientHeight;

		static const int SwapChainBufferCount = 2;
		int mCurrBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;			// Render Target heap
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;			// Depth/Stencil heap
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap;			// Constant buffer heap

		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
		DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
		
		D3D12_VIEWPORT mScreenViewport;
		D3D12_RECT mScissorRect;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
		Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode;
		Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO;

		std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;
		std::unique_ptr<DX12UploadBuffer<ObjectConstants>> mObjectCB = nullptr;		   // n
	
		XMFLOAT4X4 mWorld = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		XMFLOAT4X4 mView = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		XMFLOAT4X4 mProj = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		float mTheta = 1.5f * XM_PI;
		float mPhi = XM_PIDIV4;
		float mRadius = 5.0f;


		// Frame resources --------------------
		
		
		//static const int NumFrameResources = 3;
		//std::vector<std::unique_ptr<FrameResource>> mFrameResources;
		//FrameResource* mCurrFrameResource = nullptr;
		//int mCurrFrameResourceIndex = 0;

		//void CreateFrameResources();
	};

	
}