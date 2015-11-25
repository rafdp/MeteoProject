#pragma once

#include "includes.h"

class WindowClass : NZA_t
{
private:
	HWND hwnd_;
	SIZE size_;
	UINT style_;
	volatile bool running_;

	void* callbackPtr_;

	std::map <UINT, void (*) (void*, WPARAM, LPARAM)> callbacks_;

	void Destroy ();
	void CreateWin32Window ();
	void ProcessMessageCallback (UINT msg,
								 WPARAM wparam,
								 LPARAM lparam);

public:
	void ok () override;
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

	void AddCallback (UINT msg, void (*f) (void*, WPARAM, LPARAM));
	void SetCallbackPtr (void* ptr);

	void This ();

};

void LinkHWNDToClass (HWND wnd, LPARAM& createStr);

LRESULT CALLBACK WindowCallback (HWND windowHandle, 
								 UINT msg, 
								 WPARAM wparam, 
								 LPARAM lparam);