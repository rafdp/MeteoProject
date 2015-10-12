#pragma once

#include "includes.h"

class WindowClass : NZA_t
{
private:
	HWND hwnd_;
	SIZE size_;
	UINT style_;
	volatile bool running_;

	void Destroy ();
	void CreateWin32Window ();

public:
	void ok ();
	WindowClass (int width, 
				 int height);
	~WindowClass ();

	static void RegisterApplication ();

	UINT width  ();
	UINT height ();
	HWND hwnd ();

	friend LRESULT CALLBACK WindowCallback (HWND windowHandle,
											UINT msg,
											WPARAM wparam,
											LPARAM lparam);
};

void LinkHWNDToClass (HWND wnd, LPARAM& createStr);

LRESULT CALLBACK WindowCallback (HWND windowHandle, 
								 UINT msg, 
								 WPARAM wparam, 
								 LPARAM lparam);