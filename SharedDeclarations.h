#pragma once

#define E(num, name) ERROR_##name = num,

enum ERRORS
{
	E ( 14, DTOR)
	E ( 15, CTOR)
	E ( 16, NULL_THIS)
	E ( 17, BAD_ALLOC)
	E ( 18, UNKNOWN)
	E ( 19, REGISTER_APP)
	E ( 20, REGISTER_CLASS)
	E ( 21, FATAL_NO_EXCEPTION_DATA)
	E ( 22, DESTROYED)
	E ( 23, WIN_CALLBACK)
	E ( 24, DESTROY)
	E ( 25, NOT_RUNNING)
	E ( 26, CREATE_WINDOW)
	E ( 27, NULL_HANDLE)
	E ( 28, NULL_WND)
	E ( 29, DEVICE_SWAP_CHAIN)
	E ( 30, NULL_SWAP_CHAIN)
	E ( 31, NULL_DEVICE)
	E ( 32, NULL_DEVICE_CONTEXT)
	E ( 33, PROCESS_DRAWING)
	E ( 34, DEVICE_SWAP_CHAIN_GET_BUFFER)
	E ( 35, CREATE_RENDER_TARGET)
	E ( 36, PRESENT)
	E ( 37, SWAP_PRESENT)
	E ( 38, CREATE_DEPTH_STENCIL_STATE)
	E ( 39, ADD_DEPTH_STENCIL_STATE)
	E ( 40, APPLY_DEPTH_STENCIL_STATE)
	E ( 41, OUT_OF_RANGE)
	E ( 42, CREATE_TEXTURE)
	E ( 43, CREATE_DEPTH_STENCIL_VIEW)
	E ( 44, INIT_DEVICE_AND_SWAPCHAIN)
	E ( 45, INIT_VIEWPORT)
	E ( 46, INIT_DEPTH_STENCIL_VIEW)
	E ( 47, ADD_RASTERIZER_STATE)
	E ( 48, CREATE_RASTERIZER_STATE)
	E ( 49, REGISTER_OBJECT)
	E ( 50, SET_ID)
	E ( 51, ADD_VERTEX_ARRAY)
	E ( 52, ADD_INDEX_ARRAY)
	E ( 53, BUFFERS_SET)
	E ( 54, EMPTY_INDICES)
	E ( 55, PROCESS_OBJECTS)
	E ( 56, SETUP_BUFFERS)
	E ( 57, EMPTY_VERTICES)
	E ( 58, INDEX_BUFFER)
	E ( 59, VERTEX_BUFFER)
	E ( 60, OBJECT_BUFFER)
	E ( 61, UPDATE_CAMERA)
	E ( 62, DRAW_OBJECT)
	E ( 63, BUFFERS_NOT_SET)
	E ( 64, LOAD_SHADER)
	E ( 65, LOAD_SHADER_BLOB)
	E ( 66, GET_VERTEX_SHADER)
	E ( 67, OBJECT_BUFFER_SET)
	E ( 68, SHADER_TYPE)
	E ( 68, VERTEX_SHADER_INDEX)
	E ( 68, PIXEL_SHADER_INDEX)
	E ( 69, ATTACH_VERTEX_SHADER)
	E ( 70, ATTACH_PIXEL_SHADER)
	E ( 71, ENABLE_SHADER)
	E ( 72, ADD_LAYOUT)
	E ( 73, TEXTURE_AND_COLOR)
	E ( 74, GET_BLOB)
	E ( 75, ENABLE_LAYOUT)
	E ( 76, LAYOUT_INDEX)
	E ( 77, SET_LAYOUT)
	E ( 78, NULL_OBJECT)
	E ( 79, CREATE_LAYOUT)
	E ( 80, CREATE_BLEND_STATE)
	E ( 81, ADD_BLEND_STATE)
	E ( 82, CLEAR_BUFFERS)
	E ( 83, SET_VERTEX_BUFFER)
	E ( 84, SET_INDEX_BUFFER)
	E ( 85, SET_OBJECT_BUFFER)
	E ( 86, BIND)
	E ( 87, CREATE_CONSTANT_BUFFER)
	E ( 88, REGISTER_CONSTANT_BUFFER)
	E ( 89, NULL_CB_MANAGER)
	E ( 90, NULL_BLOB)
	E ( 91, ATTACH_SHADER_TO_OBJECT)
	E ( 92, CREATE_SHADER)
	E ( 93, LOAD_TEXTURE)
	E ( 94, LOAD_TEXTURE_FILE)
	E ( 95, GET_TEXTURE)
	E ( 96, SEND_TEXTURE_TO_PS)
	E ( 97, NULL_RESOURCE_VIEW)
	E ( 98, ADD_SAMPLER_STATE)
	E ( 99, CREATE_SAMPLER_STATE)
	E (100, APPLY_RASTERIZER_STATE)
	E (101, APPLY_BLEND_STATE)
	E (102, SEND_SAMPLER_STATE_TO_PS)
	E (103, PREPARE_TO_DRAW0)
	E (104, PREPARE_TO_DRAW1)
	E (105, ENABLE_OBJECT_SETTINGS)
	E (106, LAYOUT_NOT_SET)
	E (107, NULL_PROC_PTR)
	E (108, NULL_PARTICLE_SYSTEM_PTR)
	E (109, PREPARE_TO_DRAW)
	E (110, TRACE_RAY)
	E (111, DUMP_VERTICES_TO_OBJECT)
	E (112, PARALLEL_PROCESSING)
	E (113, NULL_RAYTRACER_PTR)
	E (114, STOP)
	E (115, RUN)
	E (116, CREATE_OBJECTS)
	E (117, NULL_FRONT_OBJECT)
	E (118, BUILD_FRONT)
	E (119, NULL_WND_PTR)
};

const wchar_t  APPLICATION_TITLE_W[] = L"MeteoProject";
const char     APPLICATION_TITLE_A[] = "MeteoProject";
const uint16_t MAX_STRING_LENGTH = 512;
const UINT     CONST_WINDOW_STYLE = WS_MINIMIZEBOX | WS_BORDER | WS_CAPTION | WS_SYSMENU;

std::string& CreateStringOnFail (const char* text, ...);
void _MessageBox (const char* text, ...);
void PrintfProgressBar (uint64_t progress, uint64_t full);

const UINT SCREEN_WIDTH  = GetSystemMetrics (SM_CXSCREEN);
const UINT SCREEN_HEIGHT = GetSystemMetrics (SM_CYSCREEN);


class ExceptionData_t;
extern ExceptionData_t* __EXPN__;

const DXGI_SAMPLE_DESC SAMPLE_DESC = { 4, 0 };

typedef uint64_t DepthStencilIndex_t;
typedef uint64_t RasterizerIndex_t;
typedef uint64_t BlendIndex_t;
typedef uint64_t SamplerIndex_t;
typedef uint64_t LayoutIndex_t;
typedef uint64_t TextureIndex_t;
typedef uint64_t ShaderIndex_t;
typedef uint64_t ConstantBufferIndex_t;

const size_t DATA_WIDTH = 1000;
const size_t DATA_HEIGHT = 500;

const UINT SLICES = 14;
const UINT HOURS = 1;

const float REGION_X = 2.0f;
const float REGION_Y = 1.0f;
const float REGION_Z = 0.5f;

#undef E