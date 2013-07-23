#include "pch.h"
#include "PhoneBox2D.h"
#include "BasicTimer.h"
#include "TestRenderer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;

PhoneBox2D::PhoneBox2D() :
	m_windowClosed(false),
	m_windowVisible(true)
{
	
}

void PhoneBox2D::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &PhoneBox2D::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &PhoneBox2D::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &PhoneBox2D::OnResuming);

	m_renderer = TestRenderer::GetInstance();
}

void PhoneBox2D::SetWindow(CoreWindow^ window)
{
	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &PhoneBox2D::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &PhoneBox2D::OnWindowClosed);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PhoneBox2D::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PhoneBox2D::OnPointerMoved);

	window->PointerReleased +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PhoneBox2D::OnPointerReleased);	

	m_renderer->Initialize(CoreWindow::GetForCurrentThread());
	m_renderer->SetSetting(Box2DXaml::TestSettings::PROFILE,true);
	m_renderer->SetSetting(Box2DXaml::TestSettings::STATISTICS,true);

}

void PhoneBox2D::Load(Platform::String^ entryPoint)
{
}

void PhoneBox2D::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			m_renderer->Update(timer->Total, timer->Delta);
			m_renderer->Render();
			m_renderer->Present(); // This call is synchronized to the display frame rate.
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void PhoneBox2D::Uninitialize()
{
}

void PhoneBox2D::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void PhoneBox2D::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void PhoneBox2D::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	Windows::Foundation::Rect windowRect = m_renderer->GetRect();
	
	int buttonWidth = (int)(windowRect.Width / 3);
	int buttonHeight = 60;
	
	int touchX = (int)args->CurrentPoint->Position.X;
	int touchY = (int)args->CurrentPoint->Position.Y;

	if((touchX < 10 + buttonWidth)&&(touchX > 10)&&
		(touchY < windowRect.Height)&&(touchY > windowRect.Height - buttonHeight))
	{
		m_renderer->PreviousTest();
	}
	else
	{
		if((touchX < windowRect.Width)&&(touchX > windowRect.Width - buttonWidth)&&
			(touchY < windowRect.Height)&&(touchY > windowRect.Height - buttonHeight))
		{
			m_renderer->NextTest();
		}
		else
		{
			if((touchX < 2 * buttonWidth)&&(touchX > buttonWidth)&&
				(touchY < windowRect.Height)&&(touchY > windowRect.Height - buttonHeight))
			{
				m_renderer->Restart();
			}
			else
			{
				m_renderer->GetCurrentTestObject()->LaunchBomb();
			}
		}
	}
}

void PhoneBox2D::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void PhoneBox2D::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void PhoneBox2D::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void PhoneBox2D::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
	m_renderer->ReleaseResourcesForSuspending();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void PhoneBox2D::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
	 m_renderer->CreateWindowSizeDependentResources();
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new PhoneBox2D();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}