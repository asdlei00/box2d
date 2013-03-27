#include "pch.h"
#include "CubeRenderer.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

CubeRenderer ^CubeRenderer::m_instance = nullptr;

CubeRenderer::CubeRenderer()
{
	m_instance = this;
}

void CubeRenderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources();

	m_commonStates.reset(new DirectX::CommonStates(m_d3dDevice.Get()));
	m_basicEffect.reset(new DirectX::BasicEffect(m_d3dDevice.Get()));
	m_batchDrawer.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(m_d3dContext.Get()));
	
	m_d3dContext->RSSetState(m_commonStates->CullNone());
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

void CubeRenderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();

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

void CubeRenderer::UpdateForWindowSizeChange()
{
	Direct3DBase::UpdateForWindowSizeChange();

	m_basicEffect->SetProjection(XMMatrixOrthographicRH(m_windowBounds.Width, m_windowBounds.Height, -1, 1));
}

void CubeRenderer::Update(float timeTotal, float timeDelta)
{
	(void) timeDelta; // Unused parameter.

	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_constantBufferData.view, (XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());//(XMMatrixRotationY(timeTotal * XM_PIDIV4)));
}

void CubeRenderer::Render()
{
	const float midnightBlue[] = { 0.098f, 0.098f, 0.439f, 1.000f };
	m_d3dContext->ClearRenderTargetView(
		m_renderTargetView.Get(),
		midnightBlue
		);

	m_d3dContext->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);

	m_d3dContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
		);
		
	//m_basicEffect->SetView(XMLoadFloat4x4(&m_constantBufferData.view));
	//m_basicEffect->SetWorld(XMLoadFloat4x4(&m_constantBufferData.model));
	//m_basicEffect->Apply(m_d3dContext.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	//m_batchDrawer->Begin();
	//m_batchDrawer->DrawLine(
	//	VertexPositionColor(XMFLOAT3(0, 0, 0), XMFLOAT4(1, 0, 0, 1)),
	//	VertexPositionColor(XMFLOAT3(100, 100, 0), XMFLOAT4(0, 1, 0, 1))
	//	);
	//m_batchDrawer->End();

	//box2D renderer test cases
	//DebugDraw m_box2dDrawer;
	//m_box2dDrawer.DrawSegment(b2Vec2(10, 100), b2Vec2(30, 100), b2Color(1, 0.5f, 0));
	//b2AABB aabb;
	//aabb.lowerBound = b2Vec2(100, 200);
	//aabb.upperBound = b2Vec2(120, 230);
	//m_box2dDrawer.DrawAABB(&aabb, b2Color(0.2f, 0.2f, 1));
	//m_box2dDrawer.DrawCircle(b2Vec2(200, 250), 20, b2Color(1, 0, 0));
	//m_box2dDrawer.DrawPoint(b2Vec2(300, 400), 100, b2Color(0, 1, 0));
	//m_box2dDrawer.DrawSolidCircle(b2Vec2(500, 100), 20, b2Vec2(2, 0), b2Color(1, 1, 0));
	//m_box2dDrawer.DrawTransform(b2Transform(b2Vec2(100, 500), b2Rot(0)));
	//b2Vec2 polygon[4] = { b2Vec2(50, 250), b2Vec2(70, 250), b2Vec2(70, 200), b2Vec2(40, 190) };
	//m_box2dDrawer.DrawSolidPolygon(polygon, 4, b2Color(0.5f, 1, 1));
}

CubeRenderer ^CubeRenderer::GetInstance()
{
	if(m_instance == nullptr)
		m_instance = ref new CubeRenderer();
	return m_instance;
}
