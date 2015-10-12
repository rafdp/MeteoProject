#pragma once

#include <exception>
#include <iostream>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <conio.h>
#include <new>
#include <vector>
#include <map>
#include <time.h>
#include <thread>
#include <string>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
#include <atomic>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

#define IGNORE_VERTEX_NORMAL
#define IGNORE_VERTEX_TEXTURE
//#define ENHANCE_PERFORMANCE

#ifdef GWL_USERDATA
#define GWL_MY_USERDATA GWL_USERDATA
#else
#define GWL_MY_USERDATA GWLP_USERDATA
#endif