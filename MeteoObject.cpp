
#include "Builder.h"

MeteoObject::MeteoObject (std::string cosmomesh,
					 	  std::string fronts,
				 		  std::string height,
			 			  Direct3DProcessor* proc)
	try :
	dl_ (cosmomesh, fronts, height, proc),
	object_ (),
	front_ (),
	currentHour_ (),
	vertS_ (),
	pixS_ (),
	geoS_ (),
	layout_ ()
{
	CreateMap (proc);
	CreateFrontsParticles (proc);
}
_END_EXCEPTION_HANDLING (CTOR)


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

void MeteoObject::LoadShadersAndLayout (Direct3DProcessor* proc)
{
	static bool once = false;
	if (!once) once = true;
	else return;

	vertS_ = proc->LoadShader ("shaders.hlsl",
							   "VShader",
							   SHADER_VERTEX);
	pixS_ = proc->LoadShader ("shaders.hlsl",
							  "PShader",
							  SHADER_PIXEL);
	geoS_ = proc->LoadShader ("shaders.hlsl",
							  "GShader",
							  SHADER_GEOMETRY);
	layout_ = proc->AddLayout (vertS_, true, false, false, true);
}

void MeteoObject::CreateMap (Direct3DProcessor* proc)
{
	BEGIN_EXCEPTION_HANDLING

		XMMATRIX world = XMMatrixTranslation (0.0f, 0.0f, 0.0f);

	Vertex_t currentVertex = {};
	LoadShadersAndLayout (proc);

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

	//float xMIN = data_.latitude (0, 0), yMIN = data_.longitude (0, 0);
	float xMIN = 0, yMIN = 0;
	//float xMAX = data_.latitude (0, 0), yMAX = data_.longitude (0, 0);
	float xMAX = DATA_WIDTH, yMAX = DATA_HEIGHT;
	/*
	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			if (data_.latitude (x, y) < xMIN) xMIN = data_.latitude (x, y);
			if (data_.latitude (x, y) > xMAX) xMAX = data_.latitude (x, y);
			if (data_.longitude (x, y) < yMIN) yMIN = data_.longitude (x, y);
			if (data_.longitude (x, y) > yMAX) yMAX = data_.longitude (x, y);
		}
	}*/

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
			//currX = (data_.latitude (x, y) - xMIN);
			currX = x;
			//currY = (data_.longitude (x, y) - yMIN);
			currY = y;
			currentVertex = {};
			int16_t h = data_.ground (x, y);
			currentVertex.SetPos (-REGION_X / 2.0f + REGION_X / (xD)* currX,
								  -(REGION_Z / 2.0f) + REGION_Z / hD * h,
								  -REGION_Y / 2.0f + REGION_Y / (yD)* currY);
			float k = 0.5f * data_.ground (x, y) / hD + 0.5f;
			//currentVertex.SetColor (k, k, data_.ground (x, y) < 10 ? 1.0f : k, k);
			if (h < 10) currentVertex.SetColor (0.0f, 0.5f, 1.0f, 0.5f);
			else
				if (h < 500) currentVertex.SetColor (0.0f, 0.5f, 0.0f, 0.3f);
				else
					currentVertex.SetColor (k, k, k, k);

			if (y == DATA_HEIGHT - 1) currentVertex.SetColor (1.0f, 0.0f, 0.0f, 1.0f);

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
		//printf ("indices %d\n", indices);
		//Sleep (100);
	}

	object_->AddIndexArray (mapping, mapSize);
	proc->AttachShaderToObject (object_, vertS_);
	proc->AttachShaderToObject (object_, pixS_);
	proc->SetLayout (object_, layout_);
	proc->RegisterObject (object_);

	delete[] mapping;

	END_EXCEPTION_HANDLING (CREATE_OBJECTS)
}

void MeteoObject::CreateFronts (Direct3DProcessor* proc)
{
	BEGIN_EXCEPTION_HANDLING

		if (front_)
		{
			//front_->ClearBuffers ();
			proc->RemoveObject (front_);
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
	LoadShadersAndLayout (proc);

	MeteoData_t& data_ = dl_.data_;


	float hMIN = data_.ground (0, 0);
	float hMAX = data_.height (0, 0, 0, currentHour_);

	float xMIN = data_.latitude (0, 0), yMIN = data_.longitude (0, 0);
	float xMAX = data_.latitude (0, 0), yMAX = data_.longitude (0, 0);

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
			currX = (data_.latitude (x, y) - xMIN);
			currY = (data_.longitude (x, y) - yMIN);
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
	proc->AttachShaderToObject (front_, vertS_);
	proc->AttachShaderToObject (front_, pixS_);
	proc->AttachShaderToObject (front_, geoS_);
	proc->SetLayout (front_, layout_);
	proc->RegisterObject (front_);

	END_EXCEPTION_HANDLING (BUILD_FRONT)
}


void MeteoObject::CreateFrontsParticles (Direct3DProcessor* proc)
{
	BEGIN_EXCEPTION_HANDLING

	if (front_)
	{
		//front_->ClearBuffers ();
		proc->RemoveObject (front_);
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
	LoadShadersAndLayout (proc);

	MeteoData_t& data_ = dl_.data_;


	float hMIN = data_.ground (0, 0);
	float hMAX = data_.height (0, 0, 0, currentHour_);

	//float xMIN = data_.latitude (0, 0), yMIN = data_.longitude (0, 0);
	float xMIN = 0, yMIN = 0;
	//float xMAX = data_.latitude (0, 0), yMAX = data_.longitude (0, 0);
	float xMAX = DATA_WIDTH, yMAX = DATA_HEIGHT;
	float midSliceHeight[SLICES] = {};

	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			//if (data_.latitude (x, y) < xMIN) xMIN = data_.latitude (x, y);
			//if (data_.latitude (x, y) > xMAX) xMAX = data_.latitude (x, y);
			//if (data_.longitude (x, y) < yMIN) yMIN = data_.longitude (x, y);
			//if (data_.longitude (x, y) > yMAX) yMAX = data_.longitude (x, y);
			float midH = 0.0f;
			for (int i = 0; i < SLICES; i++)
				midSliceHeight[i] += data_.height (x, y, i, currentHour_) / (1.0f * DATA_WIDTH * DATA_HEIGHT);
		}
	}
	float xD = xMAX - xMIN;
	float yD = yMAX - yMIN;
	//float midSliceHeight[SLICES] = {};

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
		//printf ("%f %%\n", x / (0.01f * DATA_WIDTH));
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			//currX = (data_.latitude (x, y) - xMIN);
			currX = x;
			//currY = (data_.longitude (x, y) - yMIN);
			currY = y;
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
	proc->AttachShaderToObject (front_, vertS_);
	proc->AttachShaderToObject (front_, pixS_);
	proc->AttachShaderToObject (front_, geoS_);
	proc->SetLayout (front_, layout_);
	proc->RegisterObject (front_);

	END_EXCEPTION_HANDLING (BUILD_FRONT)
}

void MeteoObject::Rotate ()
{
	if (object_)
		object_->GetWorld () *= XMMatrixRotationY (0.01f);
	if (front_)
		front_->GetWorld () *= XMMatrixRotationY (0.01f);
}

void MeteoObject::NextHour (Direct3DProcessor * proc)
{
	currentHour_++;
	if (currentHour_ == 25) currentHour_ = 0;
	CreateFronts (proc);
	front_->SetupBuffers (proc->GetDevice ());
}