#include "pch.h"
#include "TestbedWinRT.h"
#include "BasicTimer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Input;
using namespace concurrency;
using namespace DirectX;

namespace
{
	int32 testIndex = 0;
	int32 testSelection = 0;
	int32 testCount = 0;
	TestEntry* entry;
	Test* test;
	Settings settings;
	float width = 640;
	float height = 480;
	float settingsHz = 30.0f;
	float framePeriod = 1000.0f/settingsHz;
	float32 viewZoom = 1.0f;
	float tw, th;
	bool rMouseDown;
	b2Vec2 lastp;
	std::vector<Test*> tests;
}

static b2Vec2 ConvertScreenToWorld(int32 x, int32 y)
{
	float32 u = x / float32(tw);
	float32 v = (th - y) / float32(th);

	float32 ratio = float32(tw) / float32(th);
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= viewZoom;

	b2Vec2 lower = settings.viewCenter - extents;
	b2Vec2 upper = settings.viewCenter + extents;

	b2Vec2 p;
	p.x = (1.0f - u) * lower.x + u * upper.x;
	p.y = (1.0f - v) * lower.y + v * upper.y;
	return p;
}

TestbedWinRT::TestbedWinRT() :
	m_currentHighlightedX(0),
	m_currentHighlightedY(0),
	m_appState(AppState::GRID),
	m_windowClosed(false),
	m_windowVisible(true)
{
}

void TestbedWinRT::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &TestbedWinRT::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &TestbedWinRT::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &TestbedWinRT::OnResuming);

	m_renderer = TestRenderer::GetInstance();
}

void TestbedWinRT::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &TestbedWinRT::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &TestbedWinRT::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &TestbedWinRT::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &TestbedWinRT::OnPointerPressed);

	window->PointerReleased +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &TestbedWinRT::OnPointerReleased);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &TestbedWinRT::OnPointerMoved);

	window->PointerWheelChanged +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &TestbedWinRT::OnPointerWheelChanged);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &TestbedWinRT::OnKeyDown);
		
	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &TestbedWinRT::OnKeyUp);

	m_renderer->Initialize(CoreWindow::GetForCurrentThread());

	Windows::Foundation::Rect windowBounds = window->Bounds;
	float windowWidth = m_renderer->ConvertDipsToPixels(windowBounds.Width);
	float windowHeight = m_renderer->ConvertDipsToPixels(windowBounds.Height);
	width = windowWidth;
	height = windowHeight;
	tw = width;
	th = height;
	Resize(width, height);
	
	testCount = 0;
	while (g_testEntries[testCount].createFcn != NULL)
	{
		++testCount;
	}

	testIndex = b2Clamp(testIndex, 0, testCount-1);
	testSelection = testIndex;

	entry = g_testEntries + testIndex;
	//test = entry->createFcn();
	for(int i = 0; i < testCount; ++i)
		tests.push_back(g_testEntries[i].createFcn());
}

void TestbedWinRT::Load(Platform::String^ entryPoint)
{
}



void TestbedWinRT::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			timer->Update();
			m_renderer->Update(timer->Total, timer->Delta);

			//this doesn't actually do anything other than clear the screen to blue
			m_renderer->Render();

			BasicEffect *basicEffect = m_renderer->GetBasicEffect();
			XMMATRIX identity = XMMatrixIdentity();
			basicEffect->SetView(identity);
			basicEffect->SetWorld(XMMatrixIdentity());

			auto context = m_renderer->GetDeviceContext();
			auto drawer = m_renderer->GetBatchDrawer();

			settings.hz = settingsHz;
			//test->Step(&settings);
			std::vector<Test*>::iterator testIt = tests.begin();
			//float aspect = (float)tw / th;

			//handling the app logic in grid mode
			if(m_appState == AppState::GRID)
			{
				//don't draw any text in grid mode
				m_renderer->SetTextEnable(false);

				//save the current width and height to set the viewport back to it later
				float oldWidth = width;
				float oldHeight = height;

				float viewportHeight = height/5.0f;
				float viewportWidth = width/10.0f;
				float aspect = viewportWidth / viewportHeight;

				//zoom in on the gridded demos so they're easier to see
				height = height * 0.05f;
				width = height * aspect;
				m_renderer->GetBasicEffect()->SetProjection(XMMatrixOrthographicRH(width, height, -1, 1));
				m_renderer->GetBasicEffect()->Apply(context);

				//draw all the demos
				for(unsigned i = 0; i < 5; ++i)
				{
					for(unsigned j = 0; j < 10; ++j)
					{
						//sets the drawing area for each demo to its own spot
						D3D11_VIEWPORT viewport;
						viewport.Width = viewportWidth;
						viewport.Height = viewportHeight;
						viewport.MinDepth = 0;
						viewport.MaxDepth = 1;
						viewport.TopLeftX = j * viewportWidth;
						viewport.TopLeftY = i * viewportHeight;
						context->RSSetViewports(1, &viewport);

						//start a new batch of stuff to render
						m_renderer->BeginPrimitive();
						b2Vec2 oldCenter = settings.viewCenter;
						(*testIt)->Step(&settings);

#if 0
						//the step can potentially changed the camera and zoom so resize if it does
						if (oldCenter.x != settings.viewCenter.x || oldCenter.y != settings.viewCenter.y)
						{
							Resize(width, height);
						}

#endif // 0
						++testIt;
						m_renderer->EndPrimitive();

						//the final demo is invalid so it needs to break when that happens
						if(testIt == tests.end())
							break;
					}

					if(testIt == tests.end())
						break;
				}

				width = oldWidth;
				height = oldHeight;
				//test->DrawTitle(entry->name);

				//if (testSelection != testIndex)
				//{
				//	testIndex = testSelection;
				//	delete test;
				//	entry = g_testEntries + testIndex;
				//	test = entry->createFcn();
				//	viewZoom = 1.0f;
				//	settings.viewCenter.Set(0.0f, 20.0f);
				//	Resize(width, height);
				//}

#if 1
				//sets the viewport back to cover the whole screen with the origin in the upper left and positive y going down
				D3D11_VIEWPORT viewport;
				viewport.Width = (float)width;
				viewport.Height = (float)height;
				viewport.MinDepth = 0;
				viewport.MaxDepth = 1;
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
				context->RSSetViewports(1, &viewport);
				m_renderer->GetBasicEffect()->SetProjection(XMMatrixOrthographicOffCenterRH(0, (float)width, (float)height, 0, -1, 1));
				m_renderer->GetBasicEffect()->Apply(context);

				//draw the grid
				drawer->Begin();
				for(unsigned i = 0; i <= 5; ++i)
				{
					for(unsigned j = 0; j <= 10; ++j)
					{
						drawer->DrawLine(VertexPositionColor(XMFLOAT3(j * viewportWidth, 0, 0), XMFLOAT4(1, 1, 1, 1)), VertexPositionColor(XMFLOAT3(j * viewportWidth, height, 0), XMFLOAT4(1, 1, 1, 1)));
					}
					drawer->DrawLine(VertexPositionColor(XMFLOAT3(0, i *viewportHeight, 0), XMFLOAT4(1, 1, 1, 1)), VertexPositionColor(XMFLOAT3(width, i *viewportHeight, 0), XMFLOAT4(1, 1, 1, 1)));
				}

				//draw the highlighted square
				drawer->DrawLine(
					VertexPositionColor(XMFLOAT3(m_currentHighlightedX * viewportWidth, m_currentHighlightedY * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1)),
					VertexPositionColor(XMFLOAT3(m_currentHighlightedX * viewportWidth, (m_currentHighlightedY + 1) * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1))
					);
				drawer->DrawLine(
					VertexPositionColor(XMFLOAT3((m_currentHighlightedX + 1) * viewportWidth, m_currentHighlightedY * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1)),
					VertexPositionColor(XMFLOAT3((m_currentHighlightedX + 1) * viewportWidth, (m_currentHighlightedY + 1) * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1))
					);
				drawer->DrawLine(
					VertexPositionColor(XMFLOAT3(m_currentHighlightedX * viewportWidth, m_currentHighlightedY * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1)),
					VertexPositionColor(XMFLOAT3((m_currentHighlightedX + 1) * viewportWidth, m_currentHighlightedY * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1))
					);
				drawer->DrawLine(
					VertexPositionColor(XMFLOAT3(m_currentHighlightedX * viewportWidth, (m_currentHighlightedY + 1) * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1)),
					VertexPositionColor(XMFLOAT3((m_currentHighlightedX + 1) * viewportWidth, (m_currentHighlightedY + 1) * viewportHeight, 0), XMFLOAT4(1, 0, 0, 1))
					);

				drawer->End();
#endif // 0

				//Resize(width, height);  

			}
			//handling app logic when zoomed in on a demo
			else
			{
				m_renderer->SetTextEnable(true);
				Resize(width, height);
				m_renderer->BeginPrimitive();
				test->Step(&settings);
				m_renderer->EndPrimitive();
				test->DrawTitle(entry->name);
			}

			m_renderer->Present(); // This call is synchronized to the display frame rate.
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void TestbedWinRT::Uninitialize()
{
}

void TestbedWinRT::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();

	Windows::Foundation::Size windowSize = args->Size;
	float windowWidth = m_renderer->ConvertDipsToPixels(windowSize.Width);
	float windowHeight = m_renderer->ConvertDipsToPixels(windowSize.Height);
	width = windowWidth;
	height = windowHeight;
	tw = width;
	th = height;
	Resize(width, height);
}

void TestbedWinRT::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void TestbedWinRT::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void TestbedWinRT::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Use the mouse to move things around.
	PointerPointProperties ^mouseProperties = args->CurrentPoint->Properties;
	Point position = args->CurrentPoint->Position;
	if(m_appState == ZOOMED_IN)
	{
		if (mouseProperties->IsLeftButtonPressed)
		{
			VirtualKeyModifiers mod = args->KeyModifiers;
			b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
			if (mod == VirtualKeyModifiers::Shift)
			{
				if(test)
					test->ShiftMouseDown(p);
			}
			else
			{
				if(test)
					test->MouseDown(p);
			}
		}
		else if (mouseProperties->IsRightButtonPressed)
		{
			lastp = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
			rMouseDown = true;
		}
	}
	else
	{
		//zoom in on the demo selected
		if(mouseProperties->IsLeftButtonPressed)
		{
			m_currentHighlightedX = (int)(position.X / width * 10);
			m_currentHighlightedY = (int)(position.Y / height * 5);

			testIndex = m_currentHighlightedY * 10 + m_currentHighlightedX;//testSelection;

			entry = g_testEntries + testIndex;
			if(entry->createFcn == NULL)
			{
				test = NULL;
				return;
			}
			test = tests[testIndex];
			m_appState = AppState::ZOOMED_IN;
			//delete test;
			//test = entry->createFcn();
			viewZoom = 1.0f;
			settings.viewCenter.Set(0.0f, 20.0f);
			Resize(width, height);
		}
	}
}

void TestbedWinRT::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Use the mouse to move things around.
	PointerUpdateKind button = args->CurrentPoint->Properties->PointerUpdateKind;
	Point position = args->CurrentPoint->Position;
	if (button == PointerUpdateKind::LeftButtonReleased)
	{
		VirtualKeyModifiers mod = args->KeyModifiers;
		b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
		if(test)
			test->MouseUp(p);
	}
	else if (button == PointerUpdateKind::RightButtonReleased)
	{
		rMouseDown = false;
	}
}

void TestbedWinRT::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	Point position = args->CurrentPoint->Position;

	//determine the correct grid square to highlight
	m_currentHighlightedX = (int)(position.X / width * 10);
	m_currentHighlightedY = (int)(position.Y / height * 5);

	b2Vec2 p = ConvertScreenToWorld((int32)position.X, (int32)position.Y);

	if(test)
		test->MouseMove(p);
	
	if (rMouseDown)
	{
		b2Vec2 diff = p - lastp;
		settings.viewCenter.x -= diff.x;
		settings.viewCenter.y -= diff.y;
		Resize(width, height);
		lastp = ConvertScreenToWorld((int32)position.X, (int32)position.Y);
	}
}

void TestbedWinRT::OnPointerWheelChanged(CoreWindow^ sender, PointerEventArgs^ args)
{
	int direction = args->CurrentPoint->Properties->MouseWheelDelta;
	if (direction > 0)
	{
		viewZoom /= 1.1f;
	}
	else
	{
		viewZoom *= 1.1f;
	}
	Resize(width, height);
}

void TestbedWinRT::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
{
	if (test)
	{
		test->KeyboardUp((int)args->VirtualKey);
	}
}

void TestbedWinRT::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
{
	VirtualKey key = args->VirtualKey;
	//TODO: figure out how to get the keyboard modifiers using this framework

	if(m_appState == AppState::ZOOMED_IN)
	{
		switch (key)
		{
			// Press left to pan left.
		case VirtualKey::Left:
			//if (mod == GLUT_ACTIVE_CTRL)
			//{
			//	b2Vec2 newOrigin(2.0f, 0.0f);
			//	test->ShiftOrigin(newOrigin);
			//}
			//else
			{
				settings.viewCenter.x -= 0.5f;
				Resize(width, height);
			}
			break;

			// Press right to pan right.
		case VirtualKey::Right:
			//if (mod == GLUT_ACTIVE_CTRL)
			//{
			//	b2Vec2 newOrigin(-2.0f, 0.0f);
			//	test->ShiftOrigin(newOrigin);
			//}
			//else
			{
				settings.viewCenter.x += 0.5f;
				Resize(width, height);
			}
			break;

			// Press down to pan down.
		case VirtualKey::Down:
			//if (mod == GLUT_ACTIVE_CTRL)
			//{
			//	b2Vec2 newOrigin(0.0f, 2.0f);
			//	test->ShiftOrigin(newOrigin);
			//}
			//else
			{
				settings.viewCenter.y -= 0.5f;
				Resize(width, height);
			}
			break;

			// Press up to pan up.
		case VirtualKey::Up:
			//if (mod == GLUT_ACTIVE_CTRL)
			//{
			//	b2Vec2 newOrigin(0.0f, -2.0f);
			//	test->ShiftOrigin(newOrigin);
			//}
			//else
			{
				settings.viewCenter.y += 0.5f;
				Resize(width, height);
			}
			break;

			// Press home to reset the view.
		case VirtualKey::Home:
			viewZoom = 1.0f;
			settings.viewCenter.Set(0.0f, 20.0f);
			Resize(width, height);
			break;

		case VirtualKey::Escape:
			m_appState = AppState::GRID;
			test = 0;
			break;

			// Press 'z' to zoom out.
		case VirtualKey::Z:
			viewZoom = b2Min(1.1f * viewZoom, 20.0f);
			Resize(width, height);
			break;

			// Press 'x' to zoom in.
		case VirtualKey::X:
			viewZoom = b2Max(0.9f * viewZoom, 0.02f);
			Resize(width, height);
			break;

			// Press 'r' to reset.
		case VirtualKey::R:
			delete test;
			test = entry->createFcn();
			break;

			// Press space to launch a bomb.
		case VirtualKey::Space:
			if (test)
			{
				test->LaunchBomb();
			}
			break;
 
		case VirtualKey::P:
			settings.pause = !settings.pause;
			break;

			// Press [ to prev test.
		//case 0xDB:
		//	--testSelection;
		//	if (testSelection < 0)
		//	{
		//		testSelection = testCount - 1;
		//	}
		//	break;

			// Press ] to next test.
		//case 0xDD:
		//	++testSelection;
		//	if (testSelection == testCount)
		//	{
		//		testSelection = 0;
		//	}
		//	break;
		
		default:
			if (test)
			{
				test->Keyboard((int)key);
			}
		}
	}
	else
	{
		if(key == VirtualKey::Escape)
			m_windowClosed = true;
	}
}

void TestbedWinRT::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void TestbedWinRT::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void TestbedWinRT::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

void TestbedWinRT::Resize(float w, float h)
{
	if(w && h)
	{
		width = w;
		height = h;
	}

	float32 ratio = float32(tw) / float32(th);

	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= viewZoom;

	b2Vec2 lower = settings.viewCenter - extents;
	b2Vec2 upper = settings.viewCenter + extents;

	// L/R/B/T
	BasicEffect *effect = TestRenderer::GetInstance()->GetBasicEffect();
	effect->SetProjection(XMMatrixOrthographicOffCenterRH(lower.x, upper.x, lower.y, upper.y, -1, 1));
	effect->Apply(TestRenderer::GetInstance()->GetDeviceContext());
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new TestbedWinRT();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
