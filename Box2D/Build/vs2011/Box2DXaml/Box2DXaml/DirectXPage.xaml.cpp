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
	m_lastPointValid(false)
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

void DirectXPage::OnPointerMoved(Object^ sender, PointerRoutedEventArgs^ args)
{
	auto currentPoint = args->GetCurrentPoint(nullptr);
	if (currentPoint->IsInContact)
	{
		if (m_lastPointValid)
		{
			Windows::Foundation::Point delta(
				currentPoint->Position.X - m_lastPoint.X,
				currentPoint->Position.Y - m_lastPoint.Y
				);
			//m_renderer->UpdateTextPosition(delta);
		}
		m_lastPoint = currentPoint->Position;
		m_lastPointValid = true;
	}
	else
	{
		m_lastPointValid = false;
	}
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerRoutedEventArgs^ args)
{
	m_lastPointValid = false;
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

void DirectXPage::OnPreviousColorPressed(Object^ sender, RoutedEventArgs^ args)
{
	//m_renderer->BackgroundColorPrevious();
}

void DirectXPage::OnNextColorPressed(Object^ sender, RoutedEventArgs^ args)
{
	//m_renderer->BackgroundColorNext();
}

void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	//m_renderer->SaveInternalState(state);
}

void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	m_renderer->LoadInternalState(state);
}

void DirectXPage::OnChecked(Object^ sender, RoutedEventArgs^ e)
{
	if(sender->Equals(sleepCheckBox)) {
		bool checked = sleepCheckBox->IsChecked->Value;
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

void DirectXPage::SetNumberBox(TextBox^ box, int value) {
	box->Text = value.ToString();
	box->Select(box->Text->Length(), 0);
}

int DirectXPage::ValidateNumber(TextBox^ box, int min, int max) {

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

void DirectXPage::OnTextChanged(Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	int value = 0;

	if(sender->Equals(velItersBox)) 
	{
		value = ValidateNumber(velItersBox, VEL_ITERS_MIN, VEL_ITERS_MAX);
	}
	else if(sender->Equals(posItersBox)) 
	{
		value = ValidateNumber(posItersBox, POS_ITERS_MIN, POS_ITERS_MAX);
	}
	else if(sender->Equals(hertzBox)) 
	{
		value = ValidateNumber(hertzBox, HERTZ_MIN, HERTZ_MAX);
	}
}

void DirectXPage::OnTestsComboBoxChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
	if(sender->Equals(testsComboBox)) {
		m_renderer->SetTest(testsComboBox->SelectedIndex);
	}
}


