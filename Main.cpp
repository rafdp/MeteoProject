
#include "Builder.h"

ExceptionData_t* __EXPN__ = nullptr;

void ProcessCam (Direct3DCamera* cam);

int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE legacy,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	__EXPN__ = new ExceptionData_t (20, "ExceptionErrors.txt");
	srand (static_cast<UINT>(time (NULL)));

	try
	{
		WindowClass window (int (SCREEN_WIDTH * 0.98f), int (SCREEN_HEIGHT * 0.9f));
		AllocConsole ();
		FILE* file = nullptr;
		freopen_s (&file, "CONOUT$", "w", stdout);

		Direct3DProcessor d3dProc (&window);
		d3dProc.ApplyBlendState (d3dProc.AddBlendState (true));

		d3dProc.ApplyRasterizerState (d3dProc.AddRasterizerState (false, false, true));

		
		//XMMATRIX world = XMMatrixTranslation (0.0f, 0.0f, 0.0f);
		XMFLOAT4 camPos = { 0.0f, 4.0f, -4.0f, 1.0f };
		Direct3DCamera cam (&window,
							camPos.x, camPos.y, camPos.z,
							0.0f, -1.0f, 1.0f,
							0.0, 1.0f, 0.0f,
							0.1f, 0.1f);

		ConstantBufferIndex_t camBuf = d3dProc.RegisterConstantBuffer (&camPos,
																	   sizeof (camPos),
																	   1);
		d3dProc.UpdateConstantBuffer (camBuf);

		MeteoObject meteo ("Data/COSMOMESH", "Data/Fronts", "Data/H", &d3dProc);
		cam.Update ();
		
		SetForegroundWindow (window.hwnd ());


		d3dProc.ProcessObjects ();


		MSG msg = {};
		bool rotate = true;
		bool wasPressedSpace = false;
		bool wasPressedN = false;
		
		printf ("18.9.15 00:00\r");

		int hour = 0;
		while (true)
		{
			if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);

				if (msg.message == WM_QUIT) break;
			}
			// SCENE PROCESSING

			if (rotate) meteo.Rotate ();
			if (GetAsyncKeyState (VK_SPACE))
			{
				if (!wasPressedSpace)
				{
					rotate = !rotate;
					wasPressedSpace = true;
				}
			}
			else wasPressedSpace = false;

			if (GetAsyncKeyState ('N'))
			{
				if (!wasPressedN)
				{
					meteo.NextHour (&d3dProc);
					hour++;
					if (hour == 25) hour = 0;
					printf ("18.9.15 %02d:00\r", hour);
					wasPressedN = true;
				}
			}
			else wasPressedN = false;

			ProcessCam (&cam);

			//cam.RotateHorizontal (0.1f);
			cam.Update ();
			cam.StorePos (camPos);
			d3dProc.UpdateConstantBuffer (camBuf);

			d3dProc.SendCBToGS (camBuf);
			d3dProc.ProcessDrawing (&cam, true);
			d3dProc.Present ();

			//Sleep (10);
		}
		FreeConsole ();
	}
	catch (ExceptionHandler_t& ex)
	{
		_MessageBox ("Exception occurred\nCheck \"ExceptionErrors.txt\"");
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

	delete __EXPN__;
	return 0;
}


void ProcessCam (Direct3DCamera* cam)
{
	float k = 0.5f;
	if (GetAsyncKeyState (VK_LSHIFT)) k *= 2;

	if (GetAsyncKeyState ('W'))
	{
		cam->MoveForward (k*0.02f);
	}
	if (GetAsyncKeyState ('S'))
	{
		cam->MoveBackward (k*0.02f);
	}
	if (GetAsyncKeyState ('A'))
	{
		cam->MoveLeft (k*0.005f);
	}
	if (GetAsyncKeyState ('D'))
	{
		cam->MoveRight (k*0.005f);
	}
	if (GetAsyncKeyState (VK_UP))
	{
		cam->RotateVertical (k*0.005f);
	}
	if (GetAsyncKeyState (VK_DOWN))
	{
		cam->RotateVertical (-k*0.005f);
	}
	if (GetAsyncKeyState (VK_RIGHT))
	{
		cam->RotateHorizontal (k*0.01f);
	}
	if (GetAsyncKeyState (VK_LEFT))
	{
		cam->RotateHorizontal (-k*0.01f);
	}
}