#pragma once
#include "FrostIncludes.h"

using namespace DirectX;

namespace Frost
{

	// This buffer stores constant data that is fixed over a given rendering pas
	struct PassConstants
	{
		XMMATRIX View = XMMatrixIdentity();
		XMMATRIX InvView = XMMatrixIdentity();
		XMMATRIX Proj = XMMatrixIdentity();
		XMMATRIX InvProj = XMMatrixIdentity();
		XMMATRIX ViewProj = XMMatrixIdentity();
		XMMATRIX InvViewProj = XMMatrixIdentity();
		XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};

	struct ObjectConstants
	{
		XMMATRIX WorldViewProj = XMMatrixIdentity();

		XMFLOAT4 gTime;
		XMFLOAT4 pulseColor;
	};

}