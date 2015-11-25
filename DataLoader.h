#pragma once

#include "includes.h"
#include <xnamath.h>

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

	float& latitude (short x, short y)  const;
	float& longitude (short x, short y)  const;
	float& ground (short x, short y)  const;
	float& front (short x, short y, short slice, char hour = 0)  const;
	float& height (short x, short y, short slice, char hour = 0)  const;
};

class MeteoDataLoader : NZA_t
{
	friend class MeteoObject;
	MeteoData_t data_;
	XMFLOAT4* frontColors_;

public:

	void ok () override;

	MeteoDataLoader (std::string cosmomesh, 
					 std::string fronts,
					 std::string height,
					 Direct3DProcessor* proc);

	void Float2Color () const;

	XMFLOAT4* Offset (short x, short y, short slice, char hour = 0) const;

	~MeteoDataLoader ();
};