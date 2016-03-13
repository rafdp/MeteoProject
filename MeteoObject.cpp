#include "Builder.h"
#include <xnamath.h>

//#define SPHERE_STRETCH

MeteoObject::MeteoObject (std::string cosmomesh,
					 	  std::string fronts,
				 		  std::string height,
						  float* stepPtr,
						  float std_step,
			 			  Direct3DProcessor* proc,
						  Direct3DCamera* cam)
	try :
	dl_ (cosmomesh, fronts, height, proc),
	object_      (),
	map_         (),
	shuttle_     (),
	shuttleSet_  (),
	currentHour_ (),
	vertSRM_     (),
	vertS_       (),
	pixSRM_      (),
	pixS_        (),
	geoShuttleS_ (),
	texture_     (),
	texture2_    (),
	layout_      (),
	layoutRM_    (),
	sampler_     (),
	sampler2_    (),
	cb_          (),
	meteoData_	 (reinterpret_cast<MeteoObjectShaderData_t*>(_aligned_malloc(sizeof(MeteoObjectShaderData_t), 16))),
	proc_        (proc),
	cam_         (cam),
	bak_         (*cam),
	drawShuttle_ (false),
	screenNoise_ (),
	stepPtr_     (stepPtr),
	std_step_    (std_step)
{
	meteoData_->shuttle = -1.0f;
	ok ();
	CreateMap();

	meteoData_->size_ = XMVectorSet(REGION_X, REGION_Y, REGION_Z, 0.0f);

	proc_->GetWindowPtr ()->SetCallbackPtr (this);
	proc_->GetWindowPtr ()->AddCallback (WM_LBUTTONDOWN, OnPoint);
	proc_->GetWindowPtr ()->AddCallback (WM_MOUSEWHEEL,  OnWheel);
	proc_->GetWindowPtr ()->AddCallback (WM_KEYDOWN,     OnChar);
	sampler_ = proc_->AddSamplerState (D3D11_TEXTURE_ADDRESS_BORDER, {0.0f, 0.0f, 0.0f, 0.0f});
	sampler2_ = proc_->AddSamplerState (D3D11_TEXTURE_ADDRESS_WRAP);
	dl_.Float2Color();
	//Create3dTexture();
	//InitRayMarching ();
	cb_ = proc_->RegisterConstantBuffer (meteoData_, sizeof(MeteoObjectShaderData_t), 2);
	
	proc_->UpdateConstantBuffer(cb_);
}
_END_EXCEPTION_HANDLING (CTOR)

void MeteoObject::ok ()
{
	DEFAULT_OK_BLOCK

	if (proc_ == nullptr)
		_EXC_N (NULL_PROC_PTR, "Null Direct3DProcessor pointer")
}


MeteoObject::~MeteoObject ()
{
	object_ = nullptr;
	map_ = nullptr;
	shuttle_ = nullptr;
	currentHour_ = 0;
	vertS_ = -1;
	pixS_ = -1;
	layout_ = -1;
	_aligned_free(meteoData_);
}

void MeteoObject::LoadShadersAndLayout ()
{
	static bool once = false;
	if (!once) once = true;
	else return;

	vertS_ = proc_->LoadShader ("shaders.hlsl",
							   "VShader",
							   SHADER_VERTEX);
	vertSRM_ = proc_->LoadShader("shaders.hlsl",
			   				     "VShaderRM",
							     SHADER_VERTEX);
	pixS_ = proc_->LoadShader ("shaders.hlsl",
							  "PShaderRM_Map",
							  SHADER_PIXEL); 
	pixSRM_ = proc_->LoadShader("shaders.hlsl",
								"PShaderRM",
								SHADER_PIXEL);

	geoShuttleS_ = proc_->LoadShader ("shaders.hlsl",
									  "GShaderShuttle",
									  SHADER_GEOMETRY);

	layout_ = proc_->AddLayout (vertS_, true, false, false, true);
	layoutRM_ = proc_->AddLayout(vertSRM_, true, false, false, true);
}

void MeteoObject::CreateMap ()
{
	BEGIN_EXCEPTION_HANDLING

		XMMATRIX world = XMMatrixTranslation (0.0f, 0.0f, 0.0f);

	Vertex_t currentVertex = {};
	LoadShadersAndLayout ();

	map_ = new (GetValidObjectPtr ())
		Direct3DObject (world, true, true, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	std::vector<Vertex_t>& vertices = map_->GetVertices ();
	vertices.reserve (DATA_WIDTH * DATA_HEIGHT);

	MeteoData_t& data_ = dl_.data_;


	float hMIN = data_.ground (0, 0);
	float hMAX = data_.height (0, 0, 0, currentHour_);
	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			if (data_.ground (x, y) < hMIN) hMIN = data_.ground (x, y);
			for (int i = 0; i < SLICES; i++)
				if (data_.height (x, y, i, 0) > hMAX)
					hMAX = data_.height (x, y, i, currentHour_);
			if (data_.ground (x, y) > hMAX)
				hMAX = data_.ground (x, y);
		}
	}

	#ifdef SPHERE_STRETCH
	float xMIN = data_.latitude (0, 0), yMIN = data_.longitude (0, 0);
	float xMAX = data_.latitude (0, 0), yMAX = data_.longitude (0, 0);
	
	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			if (data_.latitude (x, y) < xMIN) xMIN = data_.latitude (x, y);
			if (data_.latitude (x, y) > xMAX) xMAX = data_.latitude (x, y);
			if (data_.longitude (x, y) < yMIN) yMIN = data_.longitude (x, y);
			if (data_.longitude (x, y) > yMAX) yMAX = data_.longitude (x, y);
		}
	}

	#else
	float xMIN = 0, yMIN = 0;
	float xMAX = DATA_WIDTH, yMAX = DATA_HEIGHT;

	#endif

	float xD = xMAX - xMIN;
	float yD = yMAX - yMIN;

	float hD = (hMAX - hMIN);
	float currX = 0.0f, currY = 0.0f;

	const size_t mapSize = (DATA_WIDTH - 1) * (2 * DATA_HEIGHT - 1);

	UINT* mapping = new UINT[mapSize + 1];
	UINT indices = 0;
	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
#ifdef SPHERE_STRETCH
			currX = (data_.latitude (x, y) - xMIN);
			currY = (data_.longitude (x, y) - yMIN);
#else
			currX = x;
			currY = y;
#endif
			currentVertex = {};
			float h = data_.ground (x, y);
			currentVertex.SetPos (-REGION_X / 2.0f + REGION_X / (xD)* currX,
								  -(REGION_Z / 2.0f)/* + REGION_Z / hD * h*/,
								  -REGION_Y / 2.0f + REGION_Y / (yD)* currY);
			float k = 0.5f * h / hD + 0.5f;
		
			if (h < 10.0f) currentVertex.SetColor (0.0f, 0.25f, 0.5f, 1.0f);
			else
			if (h < 3000.0f) currentVertex.SetColor (0.0f, 0.25f, 0.0f, 1.0f);
			else
				currentVertex.SetColor (k/2.0f, k / 2.0f, k / 2.0f, 1.0f);

			
			vertices.push_back (currentVertex);
		}
	}

	for (int x = 0; x < DATA_WIDTH; x += 2)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			mapping[indices] = x * DATA_HEIGHT + y;
			mapping[indices + 1] = (x + 1) * DATA_HEIGHT + y;
			if (x < DATA_WIDTH - 2)
			{
				mapping[indices + 2 * DATA_HEIGHT] = (x + 3) * DATA_HEIGHT - y - 1;
				mapping[indices + 2 * DATA_HEIGHT + 1] = (x + 2) * DATA_HEIGHT - y - 2;
			}

			indices += 2;
		}
		indices += DATA_HEIGHT * 2 - 2;
	}

	map_->AddIndexArray (mapping, mapSize);
	proc_->AttachShaderToObject (map_, vertS_);
	proc_->AttachShaderToObject (map_, pixS_);
	proc_->SetLayout (map_, layout_);
	proc_->RegisterObject (map_);

	delete[] mapping;

	END_EXCEPTION_HANDLING (CREATE_OBJECTS)
}

void MeteoObject::Rotate (float d)
{
	if (map_)
		map_->GetWorld () *= XMMatrixRotationY (d);
	if (object_)
		object_->GetWorld () *= XMMatrixRotationY (d);
}
/*
void MeteoObject::NextHour ()
{
	currentHour_++;
	if (currentHour_ == 25) currentHour_ = 0;
	CreateFronts ();
	front_->SetupBuffers (proc_->GetDevice ());
}*/

void OnPoint (void* meteoObjectPtr, WPARAM wparam, LPARAM lparam)
{
	int x = LOWORD (lparam);
	int y = HIWORD (lparam);

	(reinterpret_cast <MeteoObject*> (meteoObjectPtr))->MouseClick (x, y);
}

void OnWheel (void* meteoObjectPtr, WPARAM wparam, LPARAM lparam)
{
	int d = GET_WHEEL_DELTA_WPARAM (wparam);

	(reinterpret_cast <MeteoObject*> (meteoObjectPtr))->MouseWheel (-d);
}

void OnChar (void* meteoObjectPtr, WPARAM wparam, LPARAM lparam)
{
	if (wparam != VK_TAB) return;

	(reinterpret_cast <MeteoObject*> (meteoObjectPtr))->SwitchCams ();
}

void MeteoObject::SwitchCams ()
{
	BEGIN_EXCEPTION_HANDLING

		//printf ("CAPS %d\n", (GetKeyState (VK_CAPITAL) & 0x1));

		*stepPtr_ = std_step_;

	if (!drawShuttle_)
	{
		if (!shuttleSet_) return;
		drawShuttle_ = true;
		meteoData_->shuttle = 1.0f;

		bak_ = *cam_;
		Vertex_t& tempV = shuttle_->GetVertices ()[0];
		XMFLOAT3 tempFloat = { tempV.x, tempV.y, tempV.z };
		XMVECTOR vec = XMLoadFloat3 (&tempFloat);

		cam_->GetPos () = vec;

		proc_->RemoveObject (shuttle_);

		return;
	}

	Vertex_t& tempV = shuttle_->GetVertices ()[0];
	XMFLOAT3 tempFloat = {};
	XMStoreFloat3 (&tempFloat, cam_->GetPos ());

	tempV.SetPos (tempFloat.x, tempFloat.y, tempFloat.z);

	*cam_ = bak_;
	proc_->RegisterObject (shuttle_);
	drawShuttle_ = false;
	meteoData_->shuttle = -1.0f;

	END_EXCEPTION_HANDLING (SWITCH_CAMS)
}

void MeteoObject::Create3dTexture()
{
	BEGIN_EXCEPTION_HANDLING
	{
		dl_.Float2Color();

		D3D11_TEXTURE3D_DESC desc = {};
		desc.Width = DATA_WIDTH;
		desc.Height = DATA_HEIGHT;
		desc.MipLevels = 1;
		desc.Depth = SLICES;
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;


		D3D11_SUBRESOURCE_DATA initData = { dl_.Offset(0, 0, 0, currentHour_), DATA_WIDTH * sizeof(float), DATA_HEIGHT * DATA_WIDTH * sizeof(float) };

		HRESULT result = S_OK;

		ID3D11Texture3D* tex = nullptr;
		result = proc_->GetDevice()->CreateTexture3D(&desc, &initData, &tex);

		if (result != S_OK || !tex)
			_EXC_N(CREATE_3D_TEXTURE_D3D, "Meteo object: Failed to create texture3D (0x%x)" _ result)


			ID3D11ShaderResourceView* srv = nullptr;

		result = proc_->GetDevice()->CreateShaderResourceView(tex, nullptr, &srv);
		if (result != S_OK)
			_EXC_N(CREATE_SHADER_RESOURCE_VIEW, "Meteo object: Failed to create shader resource view (0x%x)" _ result)
			//proc_->GetDeviceContext()->GenerateMips(srv);
		texture_ = proc_->GetTextureManager().RegisterTexture(srv);

		proc_->AddToDelete(tex);
		proc_->AddToDelete(srv);

		
	}

	HRESULT result = S_OK;

	srand(time(nullptr));

	for (int x = 0; x < SCREEN_NOISE_SIZE; x++)
		for (int y = 0; y < SCREEN_NOISE_SIZE; y++)
		{
			screenNoise_[x][y] = (rand() * 1.0f) / (1.0f * RAND_MAX);
		}
	
	D3D11_TEXTURE2D_DESC desc2 = {};
	desc2.ArraySize = 1;
	desc2.SampleDesc.Count = 1;
	desc2.SampleDesc.Quality = 0;
	desc2.Width = SCREEN_NOISE_SIZE;
	desc2.Height = SCREEN_NOISE_SIZE;
	desc2.MipLevels = 1;
	desc2.Format = DXGI_FORMAT_R32_FLOAT;
	desc2.Usage  = D3D11_USAGE_DEFAULT;
	desc2.BindFlags = D3D11_BIND_SHADER_RESOURCE; 

	ID3D11Texture2D* tex2 = nullptr;
	result = proc_->GetDevice()->CreateTexture2D(&desc2, nullptr, &tex2);

	if (result != S_OK || !tex2)
		_EXC_N(CREATE_3D_TEXTURE_D3D, "Meteo object: Failed to create texture2D (0x%x)" _ result)

	proc_->GetDeviceContext()->UpdateSubresource(tex2, 
												 D3D11CalcSubresource(0, 0, 0), 
												 nullptr, 
												 screenNoise_, 
												 SCREEN_NOISE_SIZE * sizeof (float),
												 SCREEN_NOISE_SIZE * SCREEN_NOISE_SIZE * sizeof(float));


	ID3D11ShaderResourceView* srv2 = nullptr;

	result = proc_->GetDevice()->CreateShaderResourceView(tex2, nullptr, &srv2);
	if (result != S_OK)
		_EXC_N(CREATE_SHADER_RESOURCE_VIEW, "Meteo object: Failed to create shader resource view (0x%x)" _ result)
	texture2_ = proc_->GetTextureManager().RegisterTexture (srv2);

	proc_->AddToDelete(tex2);
	proc_->AddToDelete(srv2);


	END_EXCEPTION_HANDLING (CREATE_3D_TEXTURE)
}

void MeteoObject::PreDraw()
{
	BEGIN_EXCEPTION_HANDLING

	XMVECTOR temp;

	//meteoData_->inverseWorld_ = object_->GetWorld ();

	proc_->UpdateConstantBuffer(cb_);
	proc_->SendCBToPS(cb_);
	/*proc_->SendSamplerStateToPS(sampler_, 2);
	proc_->SendTextureToPS     (texture_, 2);
	proc_->SendSamplerStateToPS(sampler2_, 1);
	proc_->SendTextureToPS     (texture2_, 1);*/

	END_EXCEPTION_HANDLING(PREDRAW)
}

void MeteoObject::InitRayMarching ()
{
	BEGIN_EXCEPTION_HANDLING
	XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	object_ = new (GetValidObjectPtr())
		Direct3DObject(world, false, true, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	std::vector<Vertex_t>& vec = object_->GetVertices();

	Vertex_t current = {};

	for (int i = 0; i < 8; i++)
	{
		current.SetPos((i & 0b100 ? (-1.0f) : (1.0f)) * (REGION_X / 2.0 - 0.005f), 
					   (i & 0b010 ? (-1.0f) : (1.0f)) * (REGION_Z / 2.0 - 0.005f), 
					   (i & 0b001 ? (-1.0f) : (1.0f)) * (REGION_Y / 2.0 - 0.005f));
		vec.push_back(current);
	}

	UINT indices[36] = { 0, 4, 5, 0, 5, 1,
						 0, 1, 2, 2, 1, 3,
						 6, 5, 4, 6, 7, 5,
						 6, 2, 3, 6, 3, 7,
						 6, 4, 0, 6, 0, 2,
						 1, 7, 3, 1, 5, 7 };

	object_->AddIndexArray(indices, 36);



	LoadShadersAndLayout();

	proc_->AttachShaderToObject(object_, vertSRM_);
	proc_->AttachShaderToObject(object_, pixSRM_);
	proc_->SetLayout(object_, layoutRM_);
	proc_->RegisterObject(object_);

	END_EXCEPTION_HANDLING (INIT_RAY_MARCHING)
}

void MeteoObject::RunPolygonalBuilding()
{
	BEGIN_EXCEPTION_HANDLING

	FrontAnalyzer* frontsAnalyzed[SLICES] = {};
	FrontVisualizer* fv[SLICES - 1] = {};

	frontsAnalyzed[SLICES - 1] = new (_aligned_malloc(sizeof(FrontAnalyzer), 16)) FrontAnalyzer(&dl_, SLICES - 1);

	for (int8_t currentSlice = SLICES - 2; currentSlice >= 0; currentSlice--)
	{
		printf("Processing slice %d\n", currentSlice);
		frontsAnalyzed[currentSlice] = new (_aligned_malloc(sizeof(FrontAnalyzer), 16)) FrontAnalyzer (&dl_, currentSlice);
		fv[currentSlice] = new FrontVisualizer (frontsAnalyzed[currentSlice],
								  frontsAnalyzed[currentSlice + 1],
								  currentSlice,
								  proc_);
		fv[currentSlice]->BuildPolygons(&dl_);
	}

	for (int8_t currentSlice = 0; currentSlice < SLICES; currentSlice++)
	{
		_aligned_free(frontsAnalyzed[currentSlice]);
		printf("Hello %d", currentSlice);
		if (currentSlice == SLICES - 1) break;
		delete fv[currentSlice];
	}

	END_EXCEPTION_HANDLING (RUN_POLYGONAL_BUILDING)
}

void MeteoObject::MouseClick (int x, int y)
{
	BEGIN_EXCEPTION_HANDLING
	if (drawShuttle_) return;

	XMVECTOR view = XMVectorSet ((2 * x / ((float)proc_->GetWindowPtr ()->width ()) - 1) / cam_->GetProjection () (0, 0), 
								 (1 - 2 * y / ((float)proc_->GetWindowPtr ()->height ())) / cam_->GetProjection () (1, 1),
								 1.0f,
								 0.0f);

	XMVECTOR vec = XMVectorSet (0.0f, 0.0f, 0.0f, 0.0f);
	if (!cam_) return;
		view = XMVector3TransformCoord (view, XMMatrixInverse (&vec, cam_->GetView ()));


	XMFLOAT3 pos = {};
	XMStoreFloat3 (&pos, view);

	Vertex_t vert = {};
	vert.SetPos (pos.x, pos.y, pos.z);
	vert.SetColor (1.0f, 1.0f, 0.0f, 1.0f);

	if (!shuttleSet_)
	{
		if (shuttle_)
			_EXC_N (SHUTTLE_SET, "Shuttle not null")

		XMMATRIX world = XMMatrixIdentity (); 

		shuttle_ = new (GetValidObjectPtr ())
			Direct3DObject (world, false, false, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		shuttle_->GetVertices ().push_back (vert);
		proc_->AttachShaderToObject (shuttle_, vertS_);
		proc_->AttachShaderToObject (shuttle_, pixS_);
		proc_->AttachShaderToObject (shuttle_, geoShuttleS_);
		proc_->SetLayout (shuttle_, layout_);
		proc_->RegisterObject (shuttle_);
		shuttle_->SetupBuffers (proc_->GetDevice ());
		shuttleSet_ = true;
		//printf("shuttle ok!\n");
	}
	else
	{
		if (!shuttle_)
			_EXC_N (SHUTTLE_SET, "Shuttle null")
		shuttle_->GetVertices ()[0] = vert;
		shuttle_->SetupBuffers (proc_->GetDevice ());
	}

	END_EXCEPTION_HANDLING (MOUSE_CLICK)
}


void MeteoObject::MouseWheel (int d)
{
	BEGIN_EXCEPTION_HANDLING
	if (!shuttleSet_ || drawShuttle_) return;
	if (!shuttle_)
		_EXC_N (SHUTTLE_SET, "Shuttle null")
	Vertex_t& currentVertex = shuttle_->GetVertices ()[0];
	XMVECTOR vec = XMVectorSet (currentVertex.x, currentVertex.y, currentVertex.z, 1.0f);
	
	XMVECTOR dir = vec - cam_->GetPos ();

	vec += XMVector4Normalize (dir) * d / 1000.0f;
	XMFLOAT3 back = {};
	XMStoreFloat3 (&back, vec);

	currentVertex.x = back.x;
	currentVertex.y = back.y;
	currentVertex.z = back.z;

	shuttle_->SetupBuffers (proc_->GetDevice ());

	END_EXCEPTION_HANDLING (MOUSE_WHEEL)
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                