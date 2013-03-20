#pragma once

#include "Direct3DBase.h"
#include "CommonStates.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "RenderWinRT.h"

struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

// This class renders a simple spinning cube.
ref class CubeRenderer : public Direct3DBase
{
internal:
	CubeRenderer();
public:

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Render() override;
	
	// Method for updating time-dependent objects.
	void Update(float timeTotal, float timeDelta);

private:
	ModelViewProjectionConstantBuffer m_constantBufferData;
	std::unique_ptr<DirectX::CommonStates> m_commonStates;
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batchDrawer;
	std::unique_ptr<DebugDraw> m_box2dDrawer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};
