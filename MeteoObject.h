#pragma once

#include "includes.h"

class MeteoObject : NZA_t
{
	MeteoDataLoader dl_;
	Direct3DObject* object_;
	Direct3DObject* front_;
	Direct3DObject* shuttle_;
	bool shuttleSet_;
	uint8_t currentHour_;
	ShaderIndex_t vertS_;
	ShaderIndex_t pixS_;
	ShaderIndex_t geoS_;
	ShaderIndex_t geoShuttleS_;
	LayoutIndex_t layout_;
	Direct3DProcessor* proc_;
	Direct3DCamera* cam_;
	Direct3DCamera bak_;
	bool drawShuttle_;

	public:

	MeteoObject (std::string cosmomesh,
				 std::string fronts,
				 std::string height,
				 Direct3DProcessor* proc,
				 Direct3DCamera* cam);

	void ok ();
	~MeteoObject ();
	void LoadShadersAndLayout ();

	void CreateMap ();
	void CreateFronts ();
	void CreateFrontsParticles ();

	void Rotate ();

	void NextHour ();

	//float FrontPower (float x, float y, float z);

	friend void OnPoint (void*, WPARAM, LPARAM);
	friend void OnWheel (void*, WPARAM, LPARAM);
	friend void OnChar  (void*, WPARAM, LPARAM);

	void MouseClick (int x, int y);
	void MouseWheel (int d);
	void SwitchCams ();
};