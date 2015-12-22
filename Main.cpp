
#include "Builder.h"

ExceptionData_t* __EXPN__ = nullptr;


struct CamInfo_t
{
	XMFLOAT4 pos;
	XMFLOAT4 dir;
};

void ProcessCam (Direct3DCamera* cam);

int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE legacy,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	__EXPN__ = new ExceptionData_t (20, "ExceptionErrors.txt");
	srand (static_cast<UINT>(time (nullptr)));

	

	try
	{
		WindowClass window (int (SCREEN_WIDTH * 0.98f), int (SCREEN_HEIGHT * 0.9f));
		AllocConsole ();
		FILE* file = nullptr;
		freopen_s (&file, "CONOUT$", "w", stdout);

		Direct3DProcessor d3dProc (&window);
		d3dProc.ApplyBlendState (d3dProc.AddBlendState (true));

		//d3dProc.ApplyRasterizerState (d3dProc.AddRasterizerState (false, false, true));

		//XMMATRIX world = XMMatrixTranslation (0.0f, 0.0f, 0.0f);
		CamInfo_t camInfo = { { BASE_X, BASE_Y, BASE_Z, 1.0f }};
		Direct3DCamera cam (&window,
							camInfo.pos.x, camInfo.pos.y, camInfo.pos.z,
							0.0f, -1.0f, 1.0f,
							0.0, 1.0f, 0.0f,
							FOV, 0.1f);

		XMStoreFloat4(&camInfo.dir, cam.GetDir());

		ConstantBufferIndex_t camBuf = d3dProc.RegisterConstantBuffer (&camInfo,
																	   sizeof (camInfo),
																	   1);
		d3dProc.UpdateConstantBuffer (camBuf);

		MeteoObject meteo ("Data/COSMOMESH", "Data/Fronts", "Data/H", &d3dProc, &cam);
		cam.Update ();
		//SetForegroundWindow (window.hwnd ());


		d3dProc.ProcessObjects ();


		MSG msg = {};
		bool rotate = true;
		bool wasPressedSpace = false;
		uint64_t ticksOld = 0;
		uint64_t ticksNew = GetTickCount64 ();
		char ticksN = 0;
		
		int hour = 0;
		while (true)
		{
			if (PeekMessage (&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);

				if (msg.message == WM_QUIT) break;
			}
			// SCENE PROCESSING

			if (rotate) meteo.Rotate (0.01f);
			if (GetAsyncKeyState (VK_SPACE))
			{
				if (!wasPressedSpace)
				{
					rotate = !rotate;
					wasPressedSpace = true;
				}
			}
			else wasPressedSpace = false;

			ProcessCam (&cam);

			meteo.PreDraw ();

			cam.Update ();
			cam.StorePos (camInfo.pos);
			d3dProc.UpdateConstantBuffer (camBuf);

			d3dProc.SendCBToGS(camBuf);
			d3dProc.SendCBToPS (camBuf);
			d3dProc.ProcessDrawing (&cam, true);
			d3dProc.Present ();
			if (ticksN >= 10)
			{
				ticksN = 0;
				ticksOld = ticksNew;
				ticksNew = GetTickCount64();

				printf ("%.2f fps            \r", 10000.0f/(ticksNew - ticksOld));
			}
			ticksN++;
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
	if (GetAsyncKeyState (VK_LSHIFT)) k *= 5;

	if (GetAsyncKeyState ('E'))
	{
		XMFLOAT3 d = { 0.0f, k*0.005f, 0.0f };
		cam->TranslatePos (d);
	}
	if (GetAsyncKeyState ('Q'))
	{
		XMFLOAT3 d = { 0.0f, -k*0.005f, 0.0f };
		cam->TranslatePos (d);
	}
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
	if (GetAsyncKeyState ('N'))
	{
		cam->SetPos ({ BASE_X,  BASE_Y,  BASE_Z, 1.0f});
		cam->SetDir ({-BASE_X, -BASE_Y, -BASE_Z, 1.0f});
	}
	if (GetAsyncKeyState ('M'))
	{
		if (cam->GetFOV () < 0.9)
			cam->GetFOV () += 0.01;
	}
	if (GetAsyncKeyState ('L'))
	{
		if (cam->GetFOV () > 0.025)
			cam->GetFOV () -= 0.01;
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