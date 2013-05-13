//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage.xaml class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include <Box2D/Common/b2Math.h>
#include "Testbed/Framework/Test.h"

using namespace Box2DXaml;
using namespace Box2DSettings;

using namespace Platform;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

DirectXPage::DirectXPage() :
	m_keyHandled(false),
	m_mouseDown(false)
{
	InitializeComponent();

	m_renderer = TestRenderer::GetInstance();

	m_renderer->Initialize(
		Window::Current->CoreWindow,
		SwapChainPanel,
		DisplayProperties::LogicalDpi
		);

	Window::Current->CoreWindow->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXPage::OnWindowSizeChanged);

	Window::Current->CoreWindow->KeyDown += 
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyDown);
	
	Window::Current->CoreWindow->KeyUp += 
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyUp);

	DisplayProperties::LogicalDpiChanged +=
		ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnLogicalDpiChanged);

	DisplayProperties::OrientationChanged +=
        ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnOrientationChanged);

	DisplayProperties::DisplayContentsInvalidated +=
		ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnDisplayContentsInvalidated);
	
	m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));
	UpdateSettings();
	m_timer = ref new BasicTimer();
}

void DirectXPage::UpdateSettings() {
	// update the Tests ComboBox with the names of the tests
	int index = 0;
	while (g_testEntries[index].createFcn != NULL)
	{
		std::string s(g_testEntries[index].name);
		std::wstring w(s.begin(),s.end());
		testsComboBox->Items->Append(ref new String(w.c_str()));
		++index;
	}

	testsComboBox->SelectedIndex = m_renderer->GetCurrentTest();

	SetNumberBox(velItersBox,m_renderer->GetSetting(TestSettings::VEL_ITERS));
	SetNumberBox(posItersBox,m_renderer->GetSetting(TestSettings::POS_ITERS));
	SetNumberBox(hertzBox,m_renderer->GetSetting(TestSettings::HERTZ));

	sleepCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::SLEEP) != 0 ? true : false;
	warmStartingCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::WARM_STARTING) ? true : false;
	timeOfImpactCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::TIME_OF_IMPACT)  ? true : false;
	subSteppingCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::SUB_STEPPING) ? true : false;
	shapesCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::SHAPES) ? true : false;
	jointsCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::JOINTS) ? true : false;
	aabbsCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::AABB) ? true : false;
	contactPointsCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::CONTACT_POINTS) ? true : false;
	contactNormalsCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::CONTACT_NORMALS) ? true : false;
	contactImpulsesCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::CONTACT_IMPULSES) ? true : false;
	frictionImpulsesCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::FRICTION_IMPULSES) ? true : false;
	comCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::CENTER_OF_MASSES) ? true : false;
	statisticsCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::STATISTICS) ? true : false;
	profileCheckBox->IsChecked =  m_renderer->GetSetting(TestSettings::PROFILE) ? true : false;

	//Settings s = m_renderer->GetSettings();

}

void DirectXPage::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
{
	VirtualKey key = args->VirtualKey;

	// check if keypress was already handled
	if(m_keyHandled) {
		m_keyHandled = false;
		return;
	}

	m_renderer->KeyUp(key);
}
void DirectXPage::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
{
	VirtualKey key = args->VirtualKey;
	//TODO: figure out how to get the keyboard modifiers using this framework

	// check if keypress was already handled
	if(m_keyHandled) {
		return;
	}

	switch (key)
	{
	// Press left to pan left.
	case VirtualKey::Left:
		m_renderer->UpdateViewCenter(-0.5f,0.0f);
		break;

	// Press right to pan right.
	case VirtualKey::Right:
		m_renderer->UpdateViewCenter(0.5f,0.0f);
		break;

	// Press down to pan down.
	case VirtualKey::Down:
		m_renderer->UpdateViewCenter(0.0f,-0.5);
		break;

	// Press up to pan up.
	case VirtualKey::Up:
		m_renderer->UpdateViewCenter(0.0f,0.5);
		break;

	// Press Home to reset the view.
	case VirtualKey::Home:
		m_renderer->ResetView();
		break;

	// Press PageDown to go to previous test
	case VirtualKey::PageDown:
		m_renderer->PreviousTest();
		testsComboBox->SelectedIndex = m_renderer->GetCurrentTest();
		break;

	// Press PageUp to go to next test
	case VirtualKey::PageUp:
		m_renderer->NextTest();
		testsComboBox->SelectedIndex = m_renderer->GetCurrentTest();
		break;

	// Press 'z' to zoom out.
	case VirtualKey::Z:
		m_renderer->ZoomIn();
		break;

	// Press 'x' to zoom in.
	case VirtualKey::X:
		m_renderer->ZoomOut();
		break;

	// Press 'r' to reset.
	case VirtualKey::R:
		m_renderer->Restart();
		break;

	// Press 'p' to pause.
	case VirtualKey::P:
		m_renderer->Pause();
		updatePauseButton();
		break;

	default:
		m_renderer->KeyDown(key);
		break;
	}
}  


void DirectXPage::OnPointerPressed(Platform::Object^ sender, PointerRoutedEventArgs^ args)
{
	// Use the mouse to move things around.
	auto currentPoint = args->GetCurrentPoint(nullptr);

	VirtualKeyModifiers mod = args->KeyModifiers;
	if (mod == VirtualKeyModifiers::Shift) {
		m_renderer->ShiftMouseDown(currentPoint->Position);
	}
	else {
		m_renderer->MouseDown(currentPoint->Position);
	}
	m_mouseDown = true;
}

void DirectXPage::OnPointerReleased(Platform::Object^ sender, PointerRoutedEventArgs^ args)
{
	// Use the mouse to move things around.
	auto currentPoint = args->GetCurrentPoint(nullptr);
	m_renderer->MouseUp(currentPoint->Position);
	m_mouseDown = false;
}

void DirectXPage::OnPointerMoved(Platform::Object^ sender, PointerRoutedEventArgs^ args)
{
	if(m_mouseDown) {
		auto currentPoint = args->GetCurrentPoint(nullptr);
		m_renderer->MouseMove(currentPoint->Position);
	}
}

void DirectXPage::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();
}

void DirectXPage::OnLogicalDpiChanged(Object^ sender)
{
	m_renderer->SetDpi(DisplayProperties::LogicalDpi);
}

void DirectXPage::OnOrientationChanged(Object^ sender)
{
	m_renderer->UpdateForWindowSizeChange();
}

void DirectXPage::OnDisplayContentsInvalidated(Object^ sender)
{
	m_renderer->ValidateDevice();
}

void DirectXPage::OnRendering(Object^ sender, Object^ args)
{
	m_timer->Update();
	m_renderer->Update(m_timer->Total, m_timer->Delta);
	m_renderer->Render();
	m_renderer->Present();
}

void DirectXPage::OnPreviousTestPressed(Object^ sender, RoutedEventArgs^ args)
{
	m_renderer->PreviousTest();
	testsComboBox->SelectedIndex = m_renderer->GetCurrentTest();
}

void DirectXPage::OnNextTestPressed(Object^ sender, RoutedEventArgs^ args)
{
	m_renderer->NextTest();
	testsComboBox->SelectedIndex = m_renderer->GetCurrentTest();
}

void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	m_renderer->SaveInternalState(state);
}

void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	m_renderer->LoadInternalState(state);
}

void DirectXPage::OnChecked(Object^ sender, RoutedEventArgs^ e)
{
	CheckBox ^cb = dynamic_cast<CheckBox ^>(sender);
	int checked = cb->IsChecked->Value ? 1 : 0;

	if(sender->Equals(sleepCheckBox)) {
		m_renderer->SetSetting(TestSettings::SLEEP,checked);
	} else if(sender->Equals(warmStartingCheckBox)) {
		m_renderer->SetSetting(TestSettings::WARM_STARTING,checked);
	} else if(sender->Equals(timeOfImpactCheckBox)) {
		m_renderer->SetSetting(TestSettings::TIME_OF_IMPACT,checked);
	} else if(sender->Equals(subSteppingCheckBox)) {
		m_renderer->SetSetting(TestSettings::SUB_STEPPING,checked);
	} else if(sender->Equals(shapesCheckBox)) {
		m_renderer->SetSetting(TestSettings::SHAPES,checked);
	} else if(sender->Equals(jointsCheckBox)) {
		m_renderer->SetSetting(TestSettings::JOINTS,checked);
	} else if(sender->Equals(aabbsCheckBox)) {
		m_renderer->SetSetting(TestSettings::AABB,checked);
	} else if(sender->Equals(contactPointsCheckBox)) {
		m_renderer->SetSetting(TestSettings::CONTACT_POINTS,checked);
	} else if(sender->Equals(contactNormalsCheckBox)) {
		m_renderer->SetSetting(TestSettings::CONTACT_NORMALS,checked);
	} else if(sender->Equals(contactImpulsesCheckBox)) {
		m_renderer->SetSetting(TestSettings::CONTACT_IMPULSES,checked);
	} else if(sender->Equals(frictionImpulsesCheckBox)) {
		m_renderer->SetSetting(TestSettings::FRICTION_IMPULSES,checked);
	} else if(sender->Equals(comCheckBox)) {
		m_renderer->SetSetting(TestSettings::CENTER_OF_MASSES,checked);
	} else if(sender->Equals(statisticsCheckBox)) {
		m_renderer->SetSetting(TestSettings::STATISTICS,checked);
	} else if(sender->Equals(profileCheckBox)) {
		m_renderer->SetSetting(TestSettings::PROFILE,checked);
	}
}

void DirectXPage::updatePauseButton()
{
	if(m_renderer->GetSetting(TestSettings::PAUSED)) 
	{
		pauseButton->Content=(L"Resume");
	}
	else
	{
		pauseButton->Content=(L"Pause");
	}
}

void DirectXPage::OnPause(Object^ sender, RoutedEventArgs^ args)
{
	m_renderer->Pause();
	updatePauseButton();
}

void DirectXPage::OnRestart(Object^ sender, RoutedEventArgs^ args)
{
	m_renderer->Restart();
	updatePauseButton();
}

void DirectXPage::OnSingleStep(Object^ sender, RoutedEventArgs^ args)
{
	m_renderer->SingleStep();
	updatePauseButton();
}

void DirectXPage::SetNumberBox(TextBox^ box, int value) 
{
	box->Text = value.ToString();
	box->Select(box->Text->Length(), 0);
}

int DirectXPage::ValidateNumber(TextBox^ box, int min, int max) 
{

	int value = min;
	if(box->Text->Length() > 0) 
	{
		value = _wtoi(box->Text->Data());
		int newValue = b2Clamp(value, min, max);
		if(newValue != value) 
		{
			SetNumberBox(box,newValue);
			value = newValue;
		}
	}
	return value;
}

void DirectXPage::OnTextKeyDown(Object^ sender, KeyRoutedEventArgs^ e)
{
	int value = 0;
	m_keyHandled = true;

	if(e->Key < VirtualKey::Number0 || e->Key > VirtualKey::Number9) {
		if(e->Key < VirtualKey::NumberPad0 || e->Key > VirtualKey::NumberPad9) {
			if(e->Key != VirtualKey::Back && e->Key != VirtualKey::Tab) {
				e->Handled = true;
				return;
			}
		}
	}	
}

void DirectXPage::OnTextLostFocus(Object^ sender, RoutedEventArgs ^ e)
{
	if(sender->Equals(velItersBox)) {
		ValidateNumber(velItersBox, VEL_ITERS_MIN, VEL_ITERS_MAX);
	} else if(sender->Equals(posItersBox)) {
		ValidateNumber(posItersBox, POS_ITERS_MIN, POS_ITERS_MAX);
	} else if(sender->Equals(hertzBox)) {
		ValidateNumber(hertzBox, HERTZ_MIN, HERTZ_MAX);
	}
}

void DirectXPage::OnTestsComboBoxChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
	if(sender->Equals(testsComboBox)) {
		m_renderer->SetTest(testsComboBox->SelectedIndex);
	}
}


