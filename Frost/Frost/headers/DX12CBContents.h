#pragma once
#include "FrostIncludes.h"

using namespace DirectX;

namespace Frost
{

	struct PassConstants
	{
		DirectX::XMMATRIX View = XMMatrixIdentity();
		DirectX::XMMATRIX InvView = XMMatrixIdentity();
		DirectX::XMMATRIX Proj = XMMatrixIdentity();
		DirectX::XMMATRIX InvProj = XMMatrixIdentity();
		DirectX::XMMATRIX ViewProj = XMMatrixIdentity();
		DirectX::XMMATRIX InvViewProj = XMMatrixIdentity();
		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};

	struct ObjectConstants
	{
		DirectX::XMMATRIX WorldViewProj = XMMatrixIdentity();

		XMFLOAT4 gTime;
		XMFLOAT4 pulseColor;
	};

}