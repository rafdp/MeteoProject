#pragma once

#include "includes.h"

struct MeteoObjectShaderData_t
{
	XMMATRIX inverseWorld_;
	XMVECTOR size_;
	float shuttle;
	float bounding[3];
};

class MeteoObject : NZA_t
{
	MeteoDataLoader dl_;
	Direct3DObject* object_;
	Direct3DObject* map_;
	Direct3DObject* shuttle_;
	bool shuttleSet_;
	uint8_t currentHour_;
	ShaderIndex_t vertS_;
	ShaderIndex_t vertSRM_;
	ShaderIndex_t pixS_;
	ShaderIndex_t pixSRM_;
	ShaderIndex_t geoShuttleS_;
	LayoutIndex_t layout_;
	LayoutIndex_t layoutRM_;
    TextureIndex_t texture_;
	TextureIndex_t texture2_;
	SamplerIndex_t sampler_;
	SamplerIndex_t sampler2_;
	ConstantBufferIndex_t cb_;
	MeteoObjectShaderData_t* meteoData_;
	Direct3DProcessor* proc_;
	Direct3DCamera* cam_;
	Direct3DCamera bak_;
	bool drawShuttle_;
	float screenNoise_[SCREEN_NOISE_SIZE][SCREEN_NOISE_SIZE];
	float* stepPtr_;
	const float std_step_;

	public:

	MeteoObject (std::string cosmomesh,
				 std::string fronts,
				 std::string height,
				 float* stepPtr,
				 float std_step,
				 Direct3DProcessor* proc,
				 Direct3DCamera* cam);

	void ok () override;
	~MeteoObject ();
	void LoadShadersAndLayout ();

	void CreateMap ();
	//void CreateFronts ();
	//void CreateFrontsParticles ();

	void Rotate (float d = 0.01f);

	//void NextHour ();

	//float FrontPower (float x, float y, float z);

	friend void OnPoint (void*, WPARAM, LPARAM);
	friend void OnWheel (void*, WPARAM, LPARAM);
	friend void OnChar  (void*, WPARAM, LPARAM);

	void MouseClick (int x, int y);
	void MouseWheel (int d);
	void SwitchCams ();

	void Create3dTexture ();

	void PreDraw ();

	void InitRayMarching ();
};