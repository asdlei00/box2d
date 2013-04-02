#pragma once

#include "Direct3DBase.h"
#include "CommonStates.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "RenderWinRT.h"
#include "SpriteFont.h"

struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

// This class renders a simple spinning cube.
ref class TestRenderer : public Direct3DBase
{
public:

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void UpdateForWindowSizeChange() override;
	virtual void Render() override;
	
	// Method for updating time-dependent objects.
	void Update(float timeTotal, float timeDelta);

	// Get TestRenderer singleton
	static TestRenderer ^GetInstance();

	bool GetTextEnable() { return m_enableText; }
	void SetTextEnable(bool enable) { m_enableText = enable; }
	void BeginPrimitive();
	void EndPrimitive();
	bool IsPrimitiveBeginning() { return m_beginPrimitive; }

internal:
	//getters for the DirectXTK helpers
	DirectX::CommonStates *GetCommonStates() { return m_commonStates.get(); }
	DirectX::BasicEffect *GetBasicEffect() { return m_basicEffect.get(); }
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor> *GetBatchDrawer() { return m_batchDrawer.get(); }
	DirectX::SpriteBatch *GetSpriteBatch() { return m_spriteBatch.get(); }
	DirectX::SpriteFont *GetSpriteFont() { return m_spriteFont.get(); }
	ID3D11DeviceContext *GetDeviceContext() { return m_d3dContext.Get(); }
	ID3D11InputLayout *GetInputLayout() { return m_inputLayout.Get(); }

private:
	TestRenderer();

	static TestRenderer ^m_instance;

	ModelViewProjectionConstantBuffer m_constantBufferData;
	std::unique_ptr<DirectX::CommonStates> m_commonStates;
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batchDrawer;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	bool m_enableText;
	bool m_beginPrimitive;
};
