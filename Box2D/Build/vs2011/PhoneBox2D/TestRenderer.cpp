#include "pch.h"
#include "TestRenderer.h"
#include "arial.inc"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Box2DXaml;

TestRenderer ^TestRenderer::m_instance = nullptr;

TestRenderer::TestRenderer()
{
	m_instance = this;
	m_enableText = true;
	m_beginPrimitive = false;
	m_numTests = 0;
	while (g_testEntries[m_numTests].createFcn != NULL) {
		++m_numTests;
	}

	m_currentTest = nullptr;
	m_viewZoom = 1.0f;

	m_currentTestIndex = 0;
	m_currentTest = std::unique_ptr<Test>(g_testEntries[m_currentTestIndex].createFcn());
}

void TestRenderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources();

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
			&m_inputLayout)
	);
}

void TestRenderer::CreateWindowSizeDependentResources()
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

void TestRenderer::UpdateForWindowSizeChange()
{
	Direct3DBase::UpdateForWindowSizeChange();

	m_basicEffect->SetProjection(XMMatrixOrthographicRH(m_windowBounds.Width, m_windowBounds.Height, -1, 1));

	m_width = ConvertDipsToPixels(m_windowBounds.Width);
	m_height = ConvertDipsToPixels(m_windowBounds.Height);
}

bool TestRenderer::SaveStateKey(IPropertySet^ state, Platform::String^ key, int value)
{
	if (state->HasKey(key))
	{
		state->Remove(key);
		state->Insert(key, PropertyValue::CreateInt32(value));
		return true;
	}
	return false;
}

bool TestRenderer::LoadStateKey(IPropertySet^ state, Platform::String^ key, int& value) 
{
	if (state->HasKey(key))
	{
		value = safe_cast<IPropertyValue^>(state->Lookup(key))->GetInt32();
		return true;
	}
	return false;
}

void TestRenderer::LoadInternalState(IPropertySet^ state)
{
	int value;

	LoadStateKey(state,"VEL_ITERS", m_settings.velocityIterations);
	LoadStateKey(state,"POS_ITERS",m_settings.positionIterations);
	if(LoadStateKey(state,"HERTZ", value)) {
		m_settings.hz = (float) value;
	}
	LoadStateKey(state,"SLEEP", m_settings.enableSleep);
	LoadStateKey(state,"WARM_STARTING", m_settings.enableWarmStarting);
	LoadStateKey(state,"TIME_OF_IMPACT", m_settings.enableContinuous);
	LoadStateKey(state,"SUB_STEPPING", m_settings.enableSubStepping);
	LoadStateKey(state,"SHAPES", m_settings.drawShapes);
	LoadStateKey(state,"JOINTS", m_settings.drawJoints);
	LoadStateKey(state,"AABB", m_settings.drawAABBs);
	LoadStateKey(state,"CONTACT_POINTS", m_settings.drawContactPoints);
	LoadStateKey(state,"CONTACT_NORMALS", m_settings.drawContactNormals);
	LoadStateKey(state,"CONTACT_IMPULSES", m_settings.drawContactImpulse);
	LoadStateKey(state,"FRICTION_IMPULSES", m_settings.drawFrictionImpulse);
	LoadStateKey(state,"CENTER_OF_MASSES", m_settings.drawCOMs);
	LoadStateKey(state,"STATISTICS", m_settings.drawStats);
	LoadStateKey(state,"PROFILE", m_settings.drawProfile);
	LoadStateKey(state,"PAUSED", m_settings.pause);
	LoadStateKey(state,"SINGLE_STEP", m_settings.singleStep);
	LoadStateKey(state,"CURRENT_TEST", m_currentTestIndex);
}

void TestRenderer::SaveInternalState(IPropertySet^ state)
{
	SaveStateKey(state,"VEL_ITERS", m_settings.velocityIterations);
	SaveStateKey(state,"POS_ITERS",m_settings.positionIterations);
	SaveStateKey(state,"HERTZ", (int)m_settings.hz);
	SaveStateKey(state,"SLEEP", m_settings.enableSleep);
	SaveStateKey(state,"WARM_STARTING", m_settings.enableWarmStarting);
	SaveStateKey(state,"TIME_OF_IMPACT", m_settings.enableContinuous);
	SaveStateKey(state,"SUB_STEPPING", m_settings.enableSubStepping);
	SaveStateKey(state,"SHAPES", m_settings.drawShapes);
	SaveStateKey(state,"JOINTS", m_settings.drawJoints);
	SaveStateKey(state,"AABB", m_settings.drawAABBs);
	SaveStateKey(state,"CONTACT_POINTS", m_settings.drawContactPoints);
	SaveStateKey(state,"CONTACT_NORMALS", m_settings.drawContactNormals);
	SaveStateKey(state,"CONTACT_IMPULSES", m_settings.drawContactImpulse);
	SaveStateKey(state,"FRICTION_IMPULSES", m_settings.drawFrictionImpulse);
	SaveStateKey(state,"CENTER_OF_MASSES", m_settings.drawCOMs);
	SaveStateKey(state,"STATISTICS", m_settings.drawStats);
	SaveStateKey(state,"PROFILE", m_settings.drawProfile);
	SaveStateKey(state,"PAUSED", m_settings.pause);
	SaveStateKey(state,"SINGLE_STEP", m_settings.singleStep);
	SaveStateKey(state,"CURRENT_TEST", m_currentTestIndex);
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
		
	m_spriteBatch->Begin();
	m_spriteFont->DrawString(m_spriteBatch.get(), L"Previous", XMFLOAT2(10 , m_renderTargetSize.Height - 50));
	m_spriteFont->DrawString(m_spriteBatch.get(), L"Restart", XMFLOAT2(m_renderTargetSize.Width / 2 - 30, m_renderTargetSize.Height - 50));
	m_spriteFont->DrawString(m_spriteBatch.get(), L"Next", XMFLOAT2(m_renderTargetSize.Width - 65, m_renderTargetSize.Height - 50));
	m_spriteBatch->End();

	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	BasicEffect *basicEffect = GetBasicEffect();
	XMMATRIX identity = XMMatrixIdentity();
	basicEffect->SetView(identity);
	basicEffect->SetWorld(XMMatrixIdentity());


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

void TestRenderer::KeyUp(Windows::System::VirtualKey key) {
	unsigned char c = (unsigned char)key;
	if(c >= 'A' && c <= 'Z') {
		c += 'a' - 'A';		
	}
	m_currentTest->KeyboardUp(c);
}

void TestRenderer::KeyDown(Windows::System::VirtualKey key) {

	switch (key)
	{
		// Press space to launch a bomb.
	case VirtualKey::Space:
		{
			m_currentTest->LaunchBomb();
		}
		break;

	default:
		unsigned char c = (unsigned char)key;
		if(c >= 'A' && c <= 'Z') {
			c += 'a' - 'A';		
		}
		m_currentTest->Keyboard(c);
		break;
	}
}


void TestRenderer::UpdateViewCenter(float deltaX, float deltaY){
	m_settings.viewCenter.x += deltaX;
	m_settings.viewCenter.y += deltaY;
	Resize();
}

void TestRenderer::ResetView(){
	m_settings.viewCenter.Set(0.0f, 20.0f);
	m_viewZoom = 1.0f;
}

void TestRenderer::ZoomIn(){
	m_viewZoom = b2Min(1.1f * m_viewZoom, 20.0f);
	Resize();
}

void TestRenderer::ZoomOut(){
	m_viewZoom = b2Min(0.9f * m_viewZoom, 20.0f);
	Resize();
}






void TestRenderer::Resize()
{

	float32 ratio = float32(m_windowBounds.Width) / float32(m_windowBounds.Height);

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
	m_currentTestIndex = b2Clamp(index, 0, m_numTests-1);
	m_currentTest = std::unique_ptr<Test>(g_testEntries[m_currentTestIndex].createFcn());
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

void TestRenderer::ShiftMouseDown(Point position) {
	b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
	m_currentTest->ShiftMouseDown(p);
}

void TestRenderer::MouseDown(Point position) {
	b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
	m_currentTest->MouseDown(p);
}

void TestRenderer::MouseMove(Point position) {
	b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
	m_currentTest->MouseMove(p);
}

void TestRenderer::MouseUp(Point position) {
	b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
	m_currentTest->MouseUp(p);
}

b2Vec2 TestRenderer::ConvertScreenToWorld(int32 x, int32 y)
{
	float32 u = x / float32(m_width);
	float32 v = (m_height - y) / float32(m_height);

	float32 ratio = float32(m_width) / float32(m_height);
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_viewZoom;

	b2Vec2 lower = m_settings.viewCenter - extents;
	b2Vec2 upper = m_settings.viewCenter + extents;

	b2Vec2 p;
	p.x = (1.0f - u) * lower.x + u * upper.x;
	p.y = (1.0f - v) * lower.y + v * upper.y;
	return p;
}

