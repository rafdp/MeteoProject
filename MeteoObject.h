#pragma once

#include "includes.h"

class MeteoObject : NZA_t
{
	MeteoDataLoader dl_;
	Direct3DObject* object_;
	Direct3DObject* front_;
	uint8_t currentHour_;
	ShaderIndex_t vertS_;
	ShaderIndex_t pixS_;
	ShaderIndex_t geoS_;
	LayoutIndex_t layout_;
	WindowClass*  wnd_;

	public:

	MeteoObject (std::string cosmomesh,
				 std::string fronts,
				 std::string height,
				 Direct3DProcessor* proc);

	void ok ();
	~MeteoObject ();
	void LoadShadersAndLayout (Direct3DProcessor* proc);

	void CreateMap (Direct3DProcessor* proc);
	void CreateFronts (Direct3DProcessor* proc);
	void CreateFrontsParticles (Direct3DProcessor* proc);

	void Rotate ();

	void NextHour (Direct3DProcessor * proc);

	//float FrontPower (float x, float y, float z);

	friend void OnPoint (void*, WPARAM, LPARAM);

	void MouseClick (int x, int y);
};