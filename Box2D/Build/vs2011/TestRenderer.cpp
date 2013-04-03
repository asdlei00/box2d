#include "pch.h"
#include "TestRenderer.h"
#include "arial.inc"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

TestRenderer ^TestRenderer::m_instance = nullptr;

TestRenderer::TestRenderer()
{
	m_instance = this;
	m_enableText = true;
	m_beginPrimitive = false;
}

void TestRenderer::CreateDeviceResources()
{
	DirectXBase::CreateDeviceResources();

	m_commonStates.reset(new DirectX::CommonStates(m_d3dDevice.Get()));
	m_basicEffect.reset(new DirectX::BasicEffect(m_d3dDevice.Get()));
	size_t defaultBatchSize  = 1 << 16;
	m_batchDrawer.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(m_d3dContext.Get(), defaultBatchSize * 3, defaultBatchSize));
	m_spriteBatch.reset(new DirectX::SpriteBatch(m_d3dContext.Get()));
	m_spriteFont.reset(new DirectX::SpriteFont(m_d3dDevice.Get(), arial, sizeof(arial)));
	
	m_d3dContext->RSSetState(m_commonStates->CullClockwise());
	m_d3dContext->OMSetDepthStencilState(m_commonStates->DepthNone(), 0xffffffff);
	m_d3dContext->OMSetBlendState(m_commonStates->AlphaBlend(), NULL, 0xffffffff);
	m_basicEffect->SetLightingEnabled(false);
	m_basicEffect->SetFogEnabled(false);
	m_basicEffect->SetTextureEnabled(false);
	m_basicEffect->SetVertexColorEnabled(true);
	const void *vertexShaderByteCode;
	size_t vertexShaderSize;
	m_basicEffect->GetVertexShaderBytecode(&vertexShaderByteCode, &vertexShaderSize);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateInputLayout(
			DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			vertexShaderByteCode,
			vertexShaderSize,
			&m_inputLayout
			)
		);
}

void TestRenderer::CreateWindowSizeDependentResources()
{
	DirectXBase::CreateWindowSizeDependentResources();

	//float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;
	//float fovAngleY = 70.0f * XM_PI / 180.0f;

	// Note that the m_orientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.
	// that whole preceding paragraph is a lie!
	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixOrthographicRH(
			m_windowBounds.Width,
			m_windowBounds.Height,
			-1,
			1
			)
		);
	m_basicEffect->SetProjection(XMLoadFloat4x4(&m_constantBufferData.projection));
}

void TestRenderer::UpdateForWindowSizeChange()
{
	DirectXBase::UpdateForWindowSizeChange();

	m_basicEffect->SetProjection(XMMatrixOrthographicRH(m_windowBounds.Width, m_windowBounds.Height, -1, 1));
}

void TestRenderer::Update(float timeTotal, float timeDelta)
{
	(void) timeDelta; // Unused parameter.

	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_constantBufferData.view, (XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());//(XMMatrixRotationY(timeTotal * XM_PIDIV4)));
}

void TestRenderer::Render()
{
	const float midnightBlue[] = { 0.098f, 0.098f, 0.439f, 1.000f };
	m_d3dContext->ClearRenderTargetView(
		m_d3dRenderTargetView.Get(),
		midnightBlue
		);

	m_d3dContext->ClearDepthStencilView(
		m_d3dDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);

	m_d3dContext->OMSetRenderTargets(
		1,
		m_d3dRenderTargetView.GetAddressOf(),
		m_d3dDepthStencilView.Get()
		);
		
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	//D3D11_VIEWPORT viewport;
	//viewport.TopLeftX = 10;
	//viewport.TopLeftY = 100;
	//viewport.Width = 200;
	//viewport.Height = 300;
	//viewport.MinDepth = 0;
	//viewport.MaxDepth = 1;
	//m_d3dContext->RSSetViewports(1, &viewport);
}

TestRenderer ^TestRenderer::GetInstance()
{
	if(m_instance == nullptr)
		m_instance = ref new TestRenderer();
	return m_instance;
}

void TestRenderer::BeginPrimitive()
{
	m_beginPrimitive = true;
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_batchDrawer->Begin();
}

void TestRenderer::EndPrimitive()
{
	m_beginPrimitive = false;
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_batchDrawer->End();
}
