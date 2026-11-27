#pragma once
#include "FrostIncludes.h"

namespace Frost
{

	class DX12GeometryGenerator
	{
	public:
		using uint16 = std::uint16_t;
		using uint32 = std::uint32_t;

		struct Vertex
		{
			Vertex() {}
			Vertex(
				const DirectX::XMFLOAT3& p,
				const DirectX::XMFLOAT3& n,
				const DirectX::XMFLOAT3& t,
				const DirectX::XMFLOAT2& uv) :
				Position(p),
				Normal(n),
				Tangent(t),
				TexC(uv) {}

			Vertex(
				float px, py, pz,
				nx, ny, nz,
				tx, ty, tz,
				u, v) :
				Position(px, py, pz),
				Normal(nx, ny, nz),
				Tangent(tx, ty, tz),
				TexC(u, v) {}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT2 TexC;
		};

		struct MeshData
		{
			std::vector<Vertex> Vertices;
			std::vector<uint32> Indices32;

			std::vector<uint16>& GetIndices16()
			{
				if (mIndices16.empty())
				{
					mIndices16.resize(Indices32.size());
					for (size_t i = 0; i < Indices32.size(); i++)
					{
						mIndices16[i] = static_cast<uint16>(Indices32[i]);
					}
				}

				return mIndices16;
			}
		};

		MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	private:
		std::vector<uint16> mIndices16;

	};



}