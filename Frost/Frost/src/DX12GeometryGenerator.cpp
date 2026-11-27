#include "DX12GeometryGenerator.h"

namespace Frost
{

	DX12GeometryGenerator::MeshData DX12GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
	{
		MeshData meshData;

		//
		// Build stacks
		//

		float stackHeight = height / stackCount;

		// Amount to increment radius as we move up each stack level from bottom to top
		float radiusStep = (topRadius - bottomRadius) / stackCount;
	}



}