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
public:

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void UpdateForWindowSizeChange() override;
	virtual void Render() override;
	
	// Method for updating time-dependent objects.
	void Update(float timeTotal, float timeDelta);

	// Get CubeRenderer singleton
	static CubeRenderer ^GetInstance();

internal:
	//getters for the DirectXTK helpers
	DirectX::CommonStates *GetCommonStates() { return m_commonStates.get(); }
	DirectX::BasicEffect *GetBasicEffect() { return m_basicEffect.get(); }
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor> *GetBatchDrawer() { return m_batchDrawer.get(); }
	ID3D11DeviceContext *GetDeviceContext() { return m_d3dContext.Get(); }

private:
	CubeRenderer();

	static CubeRenderer ^m_instance;

	ModelViewProjectionConstantBuffer m_constantBufferData;
	std::unique_ptr<DirectX::CommonStates> m_commonStates;
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batchDrawer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};
