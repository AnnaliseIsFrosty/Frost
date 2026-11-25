#pragma once
#include "FrostIncludes.h"
using namespace DirectX;

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






	class RenderItem
	{
	public:
		RenderItem(int numFrameResources) : gNumFrameResources(numFrameResources) {};

		int GetNumFramesDirty() { return NumFramesDirty; }
		void DecrementNumFramesDirty() { NumFramesDirty--; }
		XMMATRIX GetWorldMatrix() { return WorldMatrix; }
		UINT GetObjCBIndex() { return ObjCBIndex; }

	private:
		// World matrix of the shape, describes the object's local space relative to the world space, which defines position, orientation and scale of the object in the world
		XMMATRIX WorldMatrix = XMMatrixIdentity();

		int gNumFrameResources;

		// Dirty flag indicating the object data has changed and we need to update the constant buffer. Since we have an object cbuffer for each FrameResource we have to apply 
		// the update to each FrameResource, so we set NumFramesDirty = gNumFrameResources
		int NumFramesDirty = gNumFrameResources;

		// Index into GPU constant buffer corresponding to the ObjectCB for this render item
		UINT ObjCBIndex = -1;

		// Gemoetry associated with this render-item (multiple render-items may share the same geometry)
		MeshGeometry* Geo = nullptr;

		// Primitive topology
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstance paramters
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		int BaseVertexLocation = 0;
	};


}