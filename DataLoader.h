#pragma once

#include "includes.h"

class MeteoData_t
{
	float* latitude_; // [DATA_WIDTH][DATA_HEIGHT];
	float* longitude_;// [DATA_WIDTH][DATA_HEIGHT];
	float* ground_;   // [DATA_WIDTH][DATA_HEIGHT];
	float* fronts_;   // [DATA_WIDTH][DATA_HEIGHT][SLICES][HOURS];
	float* height_;   // [DATA_WIDTH][DATA_HEIGHT][SLICES][HOURS];

public:

	MeteoData_t ();
	~MeteoData_t ();

	float& latitude (short x, short y);
	float& longitude (short x, short y);
	float& ground (short x, short y);
	float& front (short x, short y, short slice, char hour = 0);
	float& height (short x, short y, short slice, char hour = 0);
};

class MeteoDataLoader : NZA_t
{
	MeteoData_t data_;
	Direct3DObject* object_;
	Direct3DObject* front_;
	uint8_t currentHour_;
	ShaderIndex_t vertS_;
	ShaderIndex_t pixS_;
	ShaderIndex_t geoS_;
	LayoutIndex_t layout_;

public:

	void ok ();

	MeteoDataLoader (std::string cosmomesh, 
					 std::string fronts,
					 std::string height,
					 Direct3DProcessor* proc);

	void CreateFronts (Direct3DProcessor* proc);
	void CreateCoords (Direct3DProcessor* proc);
	void CreateGround (Direct3DProcessor* proc);
	void CreateAll    (Direct3DProcessor* proc);
	void BuildFront   (Direct3DProcessor* proc);

	~MeteoDataLoader ();

	void Rotate ();

	void NextHour (Direct3DProcessor* proc);

	void LoadShadersAndLayout (Direct3DProcessor* proc);

};