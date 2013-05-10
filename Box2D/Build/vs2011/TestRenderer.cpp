#include "pch.h"
#include "TestRenderer.h"
#include "arial.inc"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Box2DSettings;

TestRenderer ^TestRenderer::m_instance = nullptr;

TestRenderer::TestRenderer()
{
	m_instance = this;
	m_enableText = true;
	m_beginPrimitive = false;
	m_numTests = 0;
	while (g_testEntries[m_numTests].createFcn != NULL)
	{
		++m_numTests;
	}

	m_currentTest = nullptr;
	m_currentTestIndex = 0;
	m_viewZoom = 1.0f;
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

void TestRenderer::SaveInternalState(IPropertySet^ state)
{
#if 0
	if (state->HasKey("m_backgroundColorIndex"))
	{
		state->Remove("m_backgroundColorIndex");
	}
	if (state->HasKey("m_textPosition"))
	{
		state->Remove("m_textPosition");
	}
	state->Insert("m_backgroundColorIndex", PropertyValue::CreateInt32(m_backgroundColorIndex));
	state->Insert("m_textPosition", PropertyValue::CreatePoint(m_textPosition));
#endif
}

void TestRenderer::LoadInternalState(IPropertySet^ state)
{
#if 0
	if (state->HasKey("m_backgroundColorIndex") && state->HasKey("m_textPosition"))
	{
		m_backgroundColorIndex = safe_cast<IPropertyValue^>(state->Lookup("m_backgroundColorIndex"))->GetInt32();
		m_textPosition = safe_cast<IPropertyValue^>(state->Lookup("m_textPosition"))->GetPoint();
	}  
#endif // 0

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

	BasicEffect *basicEffect = GetBasicEffect();
	XMMATRIX identity = XMMatrixIdentity();
	basicEffect->SetView(identity);
	basicEffect->SetWorld(XMMatrixIdentity());

	if(!m_currentTest) {
		m_currentTest = g_testEntries[m_currentTestIndex].createFcn();
	}

	Resize();
	BeginPrimitive();
	m_currentTest->Step(&m_settings);
	EndPrimitive();
	m_currentTest->DrawTitle(g_testEntries[m_currentTestIndex].name);
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

void TestRenderer::Resize()
{

	float32 ratio = float32(m_windowBounds.Width) / float32(m_windowBounds.Width);

	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_viewZoom;

	b2Vec2 lower = m_settings.viewCenter - extents;
	b2Vec2 upper = m_settings.viewCenter + extents;

	// L/R/B/T
	BasicEffect *effect = TestRenderer::GetInstance()->GetBasicEffect();
	effect->SetProjection(XMMatrixOrthographicOffCenterRH(lower.x, upper.x, lower.y, upper.y, -1, 1));
	effect->Apply(TestRenderer::GetInstance()->GetDeviceContext());
}

void TestRenderer::Restart() 
{
	SetTest(m_currentTestIndex);
	m_settings.pause = 0;
	m_settings.singleStep = 0;
}

void TestRenderer::Pause() 
{
	m_settings.pause = !m_settings.pause;
}

void TestRenderer::SingleStep() 
{
	m_settings.pause = 1;
	m_settings.singleStep = 1;
}

void TestRenderer::SetTest(int index) 
{
	if(m_currentTest) 
	{
		delete m_currentTest;
		m_currentTest = nullptr;
	}

	m_currentTestIndex = b2Clamp(index, 0, m_numTests-1);
	m_currentTest = g_testEntries[m_currentTestIndex].createFcn();
}

void TestRenderer::PreviousTest() 
{
	m_currentTestIndex++;
	if(m_currentTestIndex >= m_numTests) {
		m_currentTestIndex = 0;
	}
	SetTest(m_currentTestIndex);
}

void TestRenderer::NextTest() 
{
	m_currentTestIndex--;
	if(m_currentTestIndex < 0) {
		m_currentTestIndex = m_numTests - 1;
	}
	SetTest(m_currentTestIndex);
}


int TestRenderer::GetSetting(TestSettings s) 
{
	switch(s) {

	case TestSettings::VEL_ITERS:
		return m_settings.velocityIterations;
		break;

	case TestSettings::POS_ITERS:
		return m_settings.positionIterations;
		break;

	case TestSettings::HERTZ:
		return (int)m_settings.hz;
		break;

	case TestSettings::SLEEP:
		return m_settings.enableSleep;
		break;

	case TestSettings::WARM_STARTING:
		return m_settings.enableWarmStarting;
		break;

	case TestSettings::TIME_OF_IMPACT:
		return m_settings.enableContinuous;
		break;

	case TestSettings::SUB_STEPPING:
		return m_settings.enableSubStepping;
		break;

	case TestSettings::SHAPES:
		return m_settings.drawShapes;
		break;

	case TestSettings::JOINTS:
		return m_settings.drawJoints;
		break;	

	case TestSettings::AABB:
		return m_settings.drawAABBs;
		break;			

	case TestSettings::CONTACT_POINTS:
		return m_settings.drawContactPoints;
		break;	

	case TestSettings::CONTACT_NORMALS:
		return m_settings.drawContactNormals;
		break;	

	case TestSettings::CONTACT_IMPULSES:
		return m_settings.drawContactImpulse;
		break;	

	case TestSettings::FRICTION_IMPULSES:
		return m_settings.drawFrictionImpulse;
		break;	

	case TestSettings::CENTER_OF_MASSES:
		return m_settings.drawCOMs;
		break;		

	case TestSettings::STATISTICS:
		return m_settings.drawStats;
		break;	

	case TestSettings::PROFILE:
		return m_settings.drawProfile;
		break;	

	case TestSettings::PAUSED:
		return m_settings.pause;
		break;	
	default:
		throw new std::exception("Invalid Box2D setting");
		break;
	}
};

void TestRenderer::SetSetting(TestSettings s, int value) 
{
	switch(s) {
	case TestSettings::VEL_ITERS:
		m_settings.velocityIterations = b2Clamp(value,VEL_ITERS_MIN, VEL_ITERS_MAX);
		break;

	case TestSettings::POS_ITERS:
		m_settings.positionIterations = b2Clamp(value,POS_ITERS_MIN, POS_ITERS_MAX);
		break;

	case TestSettings::HERTZ:
		m_settings.hz = (float)b2Clamp(value,HERTZ_MIN, HERTZ_MAX);
		break;

	case TestSettings::SLEEP:
		m_settings.enableSleep = value != 0 ? 1 : 0;
		break;

	case TestSettings::WARM_STARTING:
		m_settings.enableWarmStarting = value != 0 ? 1 : 0;
		break;

	case TestSettings::TIME_OF_IMPACT:
		m_settings.enableContinuous = value != 0 ? 1 : 0;
		break;

	case TestSettings::SUB_STEPPING:
		m_settings.enableSubStepping = value != 0 ? 1 : 0;
		break;

	case TestSettings::SHAPES:
		m_settings.drawShapes = value != 0 ? 1 : 0;
		break;

	case TestSettings::JOINTS:
		m_settings.drawJoints = value != 0 ? 1 : 0;
		break;	

	case TestSettings::AABB:
		m_settings.drawAABBs = value != 0 ? 1 : 0;
		break;			

	case TestSettings::CONTACT_POINTS:
		m_settings.drawContactPoints = value != 0 ? 1 : 0;
		break;	

	case TestSettings::CONTACT_NORMALS:
		m_settings.drawContactNormals = value != 0 ? 1 : 0;
		break;	

	case TestSettings::CONTACT_IMPULSES:
		m_settings.drawContactImpulse = value != 0 ? 1 : 0;
		break;	

	case TestSettings::FRICTION_IMPULSES:
		m_settings.drawFrictionImpulse = value != 0 ? 1 : 0;
		break;	

	case TestSettings::CENTER_OF_MASSES:
		m_settings.drawCOMs = value != 0 ? 1 : 0;
		break;		

	case TestSettings::STATISTICS:
		m_settings.drawStats = value != 0 ? 1 : 0;
		break;	

	case TestSettings::PROFILE:
		m_settings.drawProfile = value != 0 ? 1 : 0;
		break;	

	case TestSettings::PAUSED:
		m_settings.pause = value != 0 ? 1 : 0;
		break;	
	default:
		throw new std::exception("Invalid Box2D setting");
		break;
	}
};

