
#include "Builder.h"

//#define SPHERE_STRETCH

MeteoObject::MeteoObject (std::string cosmomesh,
					 	  std::string fronts,
				 		  std::string height,
			 			  Direct3DProcessor* proc,
						  Direct3DCamera* cam)
	try :
	dl_ (cosmomesh, fronts, height, proc),
	object_      (),
	front_       (),
	shuttle_     (),
	shuttleSet_  (),
	currentHour_ (),
	vertS_       (),
	pixS_        (),
	geoS_        (),
	layout_      (),
	proc_        (proc),
	cam_         (cam),
	bak_         (*cam),
	drawShuttle_ (false)
{
	ok ();

	proc_->GetWindowPtr ()->SetCallbackPtr (this);
	proc_->GetWindowPtr ()->AddCallback (WM_LBUTTONDOWN, OnPoint);
	proc_->GetWindowPtr ()->AddCallback (WM_MOUSEWHEEL,  OnWheel);
	proc_->GetWindowPtr ()->AddCallback (WM_KEYDOWN,     OnChar);
	CreateMap ();
	CreateFrontsParticles ();
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
	front_ = nullptr;
	currentHour_ = 0;
	vertS_ = 0;
	pixS_ = 0;
	geoS_ = -1;
	layout_ = -1;
}

void MeteoObject::LoadShadersAndLayout ()
{
	static bool once = false;
	if (!once) once = true;
	else return;

	vertS_ = proc_->LoadShader ("shaders.hlsl",
							   "VShader",
							   SHADER_VERTEX);
	pixS_ = proc_->LoadShader ("shaders.hlsl",
							  "PShader",
							  SHADER_PIXEL);
	geoS_ = proc_->LoadShader ("shaders.hlsl",
							  "GShader",
							  SHADER_GEOMETRY);
	layout_ = proc_->AddLayout (vertS_, true, false, false, true);
}

void MeteoObject::CreateMap ()
{
	BEGIN_EXCEPTION_HANDLING

		XMMATRIX world = XMMatrixTranslation (0.0f, 0.0f, 0.0f);

	Vertex_t currentVertex = {};
	LoadShadersAndLayout ();

	object_ = new (GetValidObjectPtr ())
		Direct3DObject (world, false, true, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	std::vector<Vertex_t>& vertices = object_->GetVertices ();
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
								  -(REGION_Z / 2.0f) + REGION_Z / hD * h,
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

	object_->AddIndexArray (mapping, mapSize);
	proc_->AttachShaderToObject (object_, vertS_);
	proc_->AttachShaderToObject (object_, pixS_);
	proc_->SetLayout (object_, layout_);
	proc_->RegisterObject (object_);

	delete[] mapping;

	END_EXCEPTION_HANDLING (CREATE_OBJECTS)
}

void MeteoObject::CreateFronts ()
{
	BEGIN_EXCEPTION_HANDLING

		if (front_)
		{
			//front_->ClearBuffers ();
			proc_->RemoveObject (front_);
			_aligned_free (front_);
		}
	XMMATRIX world = object_->GetWorld ();
	//if (!front_)
	front_ = new (GetValidObjectPtr ())
		Direct3DObject (world, false, false, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	const UINT interpolationN = 100;


	Vertex_t currentVertex = {};

	std::vector<Vertex_t>& vertices = front_->GetVertices ();
	vertices.clear ();
	vertices.reserve (DATA_WIDTH * DATA_HEIGHT);
	LoadShadersAndLayout ();

	MeteoData_t& data_ = dl_.data_;


	float hMIN = data_.ground (0, 0);
	float hMAX = data_.height (0, 0, 0, currentHour_);

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

	float midSliceHeight[SLICES] = {};

	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			if (data_.latitude (x, y) < xMIN) xMIN = data_.latitude (x, y);
			if (data_.latitude (x, y) > xMAX) xMAX = data_.latitude (x, y);
			if (data_.longitude (x, y) < yMIN) yMIN = data_.longitude (x, y);
			if (data_.longitude (x, y) > yMAX) yMAX = data_.longitude (x, y);
			float midH = 0.0f;
			for (int i = 0; i < SLICES; i++)
				midSliceHeight[i] += data_.height (x, y, i, currentHour_) / (1.0f * DATA_WIDTH * DATA_HEIGHT);
		}
	}

	float xD = xMAX - xMIN;
	float yD = yMAX - yMIN;

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
	float clipColor = 0.3f;
	float hD = (hMAX - hMIN);
	float currX = 0.0f, currY = 0.0f;
	for (int x = 0; x < DATA_WIDTH; x++)
	{
		printf ("%f %%\n", x / (0.01f * DATA_WIDTH));
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
#ifdef SPHERE_STRETCH
			currX = (data_.latitude (x, y) - xMIN);
			currY = (data_.longitude (x, y) - yMIN);
#else
			currX = x;
			currY = y;
#endif
			for (uint8_t i = 0; i < SLICES; i++)
			{
				{
					float color = data_.front (x, y, i, currentHour_) / 12.0f;
					if (!(color > 1.2f || color < 0.01f))
					{
						currentVertex.SetColor (1.0f, color, color, sin (color * XM_PI / 2.0f));

						currentVertex.SetPos (-REGION_X / 2.0f + REGION_X / (xD)* currX,
											  -REGION_Z / 2.0f + REGION_Z / hD * data_.height (x, y, i, currentHour_),
											  -REGION_Y / 2.0f + REGION_Y / (yD)* currY);
						vertices.push_back (currentVertex);
					}

				}
				if (i == (SLICES - 1)) break;
				const int interN = fabs (midSliceHeight[i + 1] - midSliceHeight[i]) / (1.0f * interpolationN);
				for (uint16_t interp = 0; interp < interN; interp++)
				{
					float kLow = (interN - interp) / (interN + 1.0f);
					float kHigh = (interp + 1) / (interN + 1.0f);
					float frontLow = data_.front (x, y, i, currentHour_) > 13.0f ? 0.0f : data_.front (x, y, i, currentHour_);
					float frontHigh = data_.front (x, y, i + 1, currentHour_) > 13.0f ? 0.0f : data_.front (x, y, i + 1, currentHour_);
					float color = (kLow * frontLow + kHigh * frontHigh) / 12.0f;
					if (color > 1.2f || color < 0.01f) continue;
					if (color < 0.1f) continue;
					currentVertex.SetColor (0.0f, color, color, sin (color * XM_PI / 2.0f));

					float Y = (kLow * data_.height (x, y, i, currentHour_) + kHigh * data_.height (x, y, i + 1, currentHour_));

					currentVertex.y = -REGION_Z / 2.0f + REGION_Z / hD * Y;
					vertices.push_back (currentVertex);
				}
			}

		}
	}
	proc_->AttachShaderToObject (front_, vertS_);
	proc_->AttachShaderToObject (front_, pixS_);
	proc_->AttachShaderToObject (front_, geoS_);
	proc_->SetLayout (front_, layout_);
	proc_->RegisterObject (front_);

	END_EXCEPTION_HANDLING (BUILD_FRONT)
}


void MeteoObject::CreateFrontsParticles ()
{
	BEGIN_EXCEPTION_HANDLING

	if (front_)
	{
		//front_->ClearBuffers ();
		proc_->RemoveObject (front_);
		_aligned_free (front_);
	}
	XMMATRIX world = object_->GetWorld ();
	//if (!front_)
	front_ = new (GetValidObjectPtr ())
		Direct3DObject (world, false, false, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	const UINT interpolationN = 500;
	Vertex_t currentVertex = {};

	std::vector<Vertex_t>& vertices = front_->GetVertices ();
	vertices.clear ();
	LoadShadersAndLayout ();

	MeteoData_t& data_ = dl_.data_;


	float hMIN = data_.ground (0, 0);
	float hMAX = data_.height (0, 0, 0, currentHour_);
	float midSliceHeight[SLICES] = {};
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
			float midH = 0.0f;
			for (int i = 0; i < SLICES; i++)
				midSliceHeight[i] += data_.height (x, y, i, currentHour_) / (1.0f * DATA_WIDTH * DATA_HEIGHT);
		}
	}
#else
	float xMIN = 0, yMIN = 0;
	float xMAX = DATA_WIDTH, yMAX = DATA_HEIGHT;
	
	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			float midH = 0.0f;
			for (int i = 0; i < SLICES; i++)
				midSliceHeight[i] += data_.height (x, y, i, currentHour_) / (1.0f * DATA_WIDTH * DATA_HEIGHT);
		}
	}

#endif
	float xD = xMAX - xMIN;
	float yD = yMAX - yMIN;

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
	float clipColor = 0.3f;
	float hD = (hMAX - hMIN);
	float currX = 0.0f, currY = 0.0f;
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
			for (uint8_t i = 0; i < SLICES; i++)
			{
				{
					float color = data_.front (x, y, i, currentHour_) / 12.0f;
					if (!(color > 1.0f || color < 0.01f))
					{
						currentVertex.SetColor (1.0f, color, color, sin (color * XM_PI / 2.0f));

						currentVertex.SetPos (-REGION_X / 2.0f + REGION_X / (xD)* currX,
											  -REGION_Z / 2.0f + REGION_Z / hD * data_.height (x, y, i, currentHour_),
											  -REGION_Y / 2.0f + REGION_Y / (yD)* currY);
						vertices.push_back (currentVertex);
					}

				}
				if (i == (SLICES - 1)) break;
				//continue;
				const int interN = fabs (midSliceHeight[i + 1] - midSliceHeight[i]) / (1.0f * interpolationN);
				for (uint16_t interp = 0; interp < interN; interp++)
				{
					float kLow = (interN - interp) / (interN + 1.0f);
					//float kHigh = (interp + 1) / (interN + 1.0f);
					float frontLow = data_.front (x, y, i, currentHour_) > 13.0f ? 0.0f : data_.front (x, y, i, currentHour_);
					float frontHigh = data_.front (x, y, i + 1, currentHour_) > 13.0f ? 0.0f : data_.front (x, y, i + 1, currentHour_);
					float color = (kLow * frontLow + (1.0f-kLow) * frontHigh) / 12.0f;

					if (color > 1.0f || color < 0.01f) continue;
					if (color < 0.1f) continue;
					currentVertex.SetColor (0.0f, color, color, color *2.0f >= 1.0f ? 1.0f : color * 2.0f);
					//if (interp / (1.0f * interN) > 0.5f && color > 0.5f && data_.front (x, y, i, currentHour_) <= 0.1f)
						//printf ("low %f high %f frontLow %f frontHigh %f interp %f %d %d\n", kLow, 1.0f-kLow, frontLow, frontHigh, interp / (1.0f * interN), x, y);


					float Y = (kLow * data_.height (x, y, i, currentHour_) + (1.0f - kLow) * data_.height (x, y, i + 1, currentHour_));

					currentVertex.y = -REGION_Z / 2.0f + REGION_Z / hD * Y;
					vertices.push_back (currentVertex);
				}
			}

		}
	}
	proc_->AttachShaderToObject (front_, vertS_);
	proc_->AttachShaderToObject (front_, pixS_);
	proc_->AttachShaderToObject (front_, geoS_);
	proc_->SetLayout (front_, layout_);
	proc_->RegisterObject (front_);

	END_EXCEPTION_HANDLING (BUILD_FRONT)
}

void MeteoObject::Rotate ()
{
	if (object_)
		object_->GetWorld () *= XMMatrixRotationY (0.01f);
	if (front_)
		front_->GetWorld () *= XMMatrixRotationY (0.01f);
}

void MeteoObject::NextHour ()
{
	currentHour_++;
	if (currentHour_ == 25) currentHour_ = 0;
	CreateFronts ();
	front_->SetupBuffers (proc_->GetDevice ());
}

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

	if (!drawShuttle_)
	{
		if (!shuttleSet_) return;
		drawShuttle_ = true;

		bak_ = *cam_;
		Vertex_t& tempV = shuttle_->GetVertices ()[0];
		XMFLOAT3 tempFloat = { tempV.x, tempV.y, tempV.z };
		XMVECTOR vec = XMLoadFloat3 (&tempFloat);

		cam_->GetPos () = vec;
		cam_->GetFOV () *= 2;

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

	END_EXCEPTION_HANDLING (SWITCH_CAMS)
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

		//obj->AddVertexArray (&vert, 1);
		shuttle_->GetVertices ().push_back (vert);
		proc_->AttachShaderToObject (shuttle_, vertS_);
		proc_->AttachShaderToObject (shuttle_, pixS_);
		proc_->AttachShaderToObject (shuttle_, geoS_);
		proc_->SetLayout (shuttle_, layout_);
		proc_->RegisterObject (shuttle_);
		shuttle_->SetupBuffers (proc_->GetDevice ());
		shuttleSet_ = true;
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