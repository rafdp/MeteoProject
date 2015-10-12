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

	public:

	MeteoObject (std::string cosmomesh,
				 std::string fronts,
				 std::string height,
				 Direct3DProcessor* proc);
	~MeteoObject ();
	void LoadShadersAndLayout (Direct3DProcessor* proc);

	void CreateMap (Direct3DProcessor* proc);
	void CreateFronts (Direct3DProcessor* proc);

	void Rotate ();

	void NextHour (Direct3DProcessor * proc);
};