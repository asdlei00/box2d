#pragma once

#include "DirectXBase.h"
//#include "CommonStates.h"
#include "Effects.h"
//#include "PrimitiveBatch.h"
//#include "VertexTypes.h"
#include "RenderWinRT.h"
//#include "SpriteFont.h"
#include "Testbed/Framework/Test.h"

struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

namespace Box2DXaml {
	const int VEL_ITERS_MIN = 1;
	const int VEL_ITERS_MAX = 500;
	const int POS_ITERS_MIN = 0;
	const int POS_ITERS_MAX = 100;
	const int HERTZ_MIN = 5;
	const int HERTZ_MAX = 100;

	public enum class TestSettings {
		VEL_ITERS = 0,
		POS_ITERS,
		HERTZ,
		SLEEP,
		WARM_STARTING,
		TIME_OF_IMPACT,
		SUB_STEPPING,
		SHAPES,
		JOINTS,
		AABB,
		CONTACT_POINTS,
		CONTACT_NORMALS,
		CONTACT_IMPULSES,
		FRICTION_IMPULSES,
		CENTER_OF_MASSES,
		STATISTICS,
		PROFILE,
		PAUSED,
		NUM_SETTINGS
	};
}


// This class renders a simple spinning cube.
ref class TestRenderer sealed : public DirectXBase
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

	// Methods to save and load state in response to suspend.
	void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
	void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

	void PreviousTest();
	void NextTest();

	bool GetTextEnable() { return m_enableText; }
	int GetCurrentTest() { return m_currentTestIndex; }
	void SetTextEnable(bool enable) { m_enableText = enable; }
	void BeginPrimitive();
	void EndPrimitive();
	bool IsPrimitiveBeginning() { return m_beginPrimitive; }

	int GetSetting(Box2DXaml::TestSettings s);
	void SetSetting(Box2DXaml::TestSettings s, int value);
	void SetTest(int index);
	void Restart();
	void Pause();
	void SingleStep();

	void ShiftMouseDown(Windows::Foundation::Point position);
	void MouseDown(Windows::Foundation::Point position);
	void MouseMove(Windows::Foundation::Point position);
	void MouseUp(Windows::Foundation::Point position);
	
	void KeyDown(Windows::System::VirtualKey key);
	void KeyUp(Windows::System::VirtualKey key);
	void UpdateViewCenter(float deltaX, float deltaY);
	void ResetView();
	void ZoomIn();
	void ZoomOut();

internal:
	DirectX::XMMATRIX* GetProjectionMatrix(){return &projectionMatrix;}
private:
	TestRenderer();
	void Resize();
	bool SaveStateKey(Windows::Foundation::Collections::IPropertySet^ state, Platform::String^ key, int value);
	bool LoadStateKey(Windows::Foundation::Collections::IPropertySet^ state, Platform::String^ key, int& value);

	int m_numTests;
	int m_currentTestIndex;
	std::unique_ptr<Test> m_currentTest;
	Settings m_settings;
	float m_viewZoom;
	static TestRenderer ^m_instance;
	b2Vec2 ConvertScreenToWorld(int32 x, int32 y);
	ModelViewProjectionConstantBuffer m_constantBufferData;
	bool m_enableText;
	bool m_beginPrimitive;
	float m_width;
	float m_height;
	DirectX::XMMATRIX projectionMatrix;
};
