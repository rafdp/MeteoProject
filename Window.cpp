#include "Builder.h"

void WindowClass::ok ()
{
	DEFAULT_OK_BLOCK
}

WindowClass::WindowClass (int width, 
						  int height)
	try :  
	hwnd_    (nullptr),
	size_    ({ width, height }),
	style_ (CONST_WINDOW_STYLE),
	running_ (false),
	callbacks_ (),
	callbackPtr_ (nullptr)
{
	printf ("%X\n", this);
	RegisterApplication ();
	CreateWin32Window ();
}
_END_EXCEPTION_HANDLING (CTOR)

WindowClass::~WindowClass ()
{
	if (hwnd_ && running_)
	{
		Destroy ();
		SendNotifyMessage (hwnd_, WM_DESTROY, 0, 100500);
	}
}

void WindowClass::RegisterApplication ()
{
	try
	{
		static bool done = false;
		if (done) return;

		WNDCLASSEX wnd = {};

		wnd.cbSize = sizeof (wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc = (WNDPROC)WindowCallback;
		wnd.hCursor = LoadCursor (NULL, IDC_ARROW);
		wnd.hbrBackground = (HBRUSH)GetStockObject (HOLLOW_BRUSH);
		wnd.lpszClassName = APPLICATION_TITLE_W;

		ATOM wndclass = RegisterClassEx (&wnd);
		if (!wndclass)
			_EXC_N (REGISTER_CLASS,
					"Failed to register Win32 class")
			done = true;
	}
	_END_EXCEPTION_HANDLING (REGISTER_APP)
}

HWND WindowClass::hwnd ()
{
	return hwnd_;
}

void WindowClass::Destroy ()
{
	BEGIN_EXCEPTION_HANDLING
		if (!running_)
			_EXC_N (NOT_RUNNING,
					"Trying to destory already destroyed window")
			running_ = false;
	hwnd_ = NULL;
	size_ = {};

	END_EXCEPTION_HANDLING (DESTROY)
}

void WindowClass::CreateWin32Window ()
{
	BEGIN_EXCEPTION_HANDLING

		RECT windowRect = { (LONG)(SCREEN_WIDTH - size_.cx) / 2,
							(LONG)(SCREEN_HEIGHT - size_.cy) / 2,
							(LONG)(SCREEN_WIDTH + size_.cx) / 2,
							(LONG)(SCREEN_HEIGHT + size_.cy) / 2 };
	AdjustWindowRect (&windowRect, style_, false);

	HWND handle = CreateWindowExA (0,
								   APPLICATION_TITLE_A,
								   APPLICATION_TITLE_A,
								   style_,
								   windowRect.left,
								   windowRect.top,
								   windowRect.right - windowRect.left,
								   windowRect.bottom - windowRect.top,
								   NULL,
								   NULL,
								   NULL,
								   this);
	if (!handle)
		_EXC_N (NULL_HANDLE, "Win32 CreateWindow failed")
		hwnd_ = handle;
	ShowWindow (handle, SW_SHOWNORMAL);
	SetForegroundWindow (handle);
	UpdateWindow (handle);
	running_ = true;
	END_EXCEPTION_HANDLING (CREATE_WINDOW)
}

void WindowClass::ProcessMessageCallback (UINT msg, WPARAM wparam, LPARAM lparam)
{
	auto found = callbacks_.find (msg);
	if (found != callbacks_.end ())
		(*found->second) (callbackPtr_, wparam, lparam);

}

UINT WindowClass::width ()
{
	return size_.cx;
}

UINT WindowClass::height ()
{
	return size_.cy;
}


LRESULT CALLBACK WindowCallback (HWND windowHandle,
								 UINT msg,
								 WPARAM wparam,
								 LPARAM lparam)
{
	try
	{
		WindowClass* windowPtr = reinterpret_cast <WindowClass*> (GetWindowLongPtr (windowHandle, GWL_MY_USERDATA));
		if (windowPtr)
			windowPtr->ProcessMessageCallback (msg, wparam, lparam);
		switch (msg)
		{
			case WM_CREATE:
			{
				WindowClass* windowPtr = *reinterpret_cast<WindowClass**> (lparam + offsetof (CREATESTRUCT, lpCreateParams));
				SetWindowLongPtr (windowHandle,
								  GWL_MY_USERDATA,
							      reinterpret_cast<LONG_PTR> (windowPtr));

				break;
			}
			case WM_DESTROY:
			{
				PostQuitMessage (0);
				return 0;
			}
			case WM_KEYDOWN:
			{
				if (wparam == VK_ESCAPE)
				{
					SendNotifyMessage (windowHandle, WM_CLOSE, 0, 0);
					return 0;
				}
				break;
			}

			case WM_SIZE:
			{
				if (wparam == SIZE_MINIMIZED) break;
				RECT r = {};
				GetClientRect (windowHandle, &r);
				windowPtr->size_ = { r.right - r.left, r.bottom - r.top };
				UpdateWindow (windowHandle);
				break;
			}
			/*
			case WM_CLOSE:
				return on::Close (wnd);
				break;

			case WM_DESTROY:
				on::Destroy (wnd, lparam);
				break;

			case WM_CHAR:
				on::Char (wparam, lparam);
				break;

			case WM_SIZE:
				on::Size (wnd, wparam);
				break;
				*/
			default:
				break;

		}

		
		return DefWindowProc (windowHandle, msg, wparam, lparam);
	}
	catch (ExceptionHandler_t& ex)
	{
		_MessageBox ("Exception occurred (WINDOW_CALLBACK)\nCheck \"ExceptionErrors.txt\"");
		ex.WriteLog (__EXPN__);
		system ("start ExceptionErrors.txt");
	}
	catch (std::exception err)
	{
		_MessageBox ("Exception occurred: %s\n", err.what ());
	}
	catch (...)
	{
		_MessageBox ("Exception occurred\n");
	}
}


void WindowClass::AddCallback (UINT msg, void (*f) (void*, WPARAM, LPARAM))
{
	callbacks_[msg] = f;
}

void WindowClass::SetCallbackPtr (void* ptr)
{
	callbackPtr_ = ptr;
}

void WindowClass::This ()
{
	printf ("_____%X\n", this);
}
