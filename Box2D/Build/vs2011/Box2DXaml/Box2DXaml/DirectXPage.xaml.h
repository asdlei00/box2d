//
// BlankPage.xaml.h
// Declaration of the BlankPage.xaml class.
//

#pragma once

#include "DirectXPage.g.h"
#include "TestRenderer.h"
#include "BasicTimer.h"

namespace Box2DXaml
{
	/// <summary>
	/// A DirectX page that can be used on its own.  Note that it may not be used within a Frame.
	/// </summary>
    [Windows::Foundation::Metadata::WebHostHidden]
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();

		void OnPreviousTestPressed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnNextTestPressed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

	private:
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args);
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args);
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnLogicalDpiChanged(Platform::Object^ sender);
		void OnOrientationChanged(Platform::Object^ sender);
		void OnDisplayContentsInvalidated(Platform::Object^ sender);
		void OnRendering(Object^ sender, Object^ args);
		void OnChecked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnTestsComboBoxChanged(Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void OnPause(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void updatePauseButton();
		void OnSingleStep(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnRestart(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnTextChanged(Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e); 
		void UpdateSettings();

		int ValidateNumber(Windows::UI::Xaml::Controls::TextBox^ box, int min, int max);
		void SetNumberBox(Windows::UI::Xaml::Controls::TextBox^ box, int value);

		Windows::Foundation::EventRegistrationToken m_eventToken;

		TestRenderer^ m_renderer;

		Windows::Foundation::Point m_lastPoint;
		bool m_lastPointValid;
		
		BasicTimer^ m_timer;
		bool m_mouseDown;
	};
}
