#pragma once
#include "includes.h"

class Incrementator_t
{
	int& data;
public:
	Incrementator_t(int& data_);
	~Incrementator_t();
};

struct FrontInfo_t;

struct EquivalentPointData_t
{
	uint32_t point;
	double d;
};

EquivalentPointData_t FindEquivalentPoint (FrontInfo_t* front,
										   FrontInfo_t* frontTarget,
										   uint32_t point);

class FrontVisualizer : NZA_t
{
	XMMATRIX world_;
	FrontAnalyzer* level0_;
	FrontAnalyzer* level1_;
	int8_t zeroLevel_;
	Direct3DObject* object_;
	Direct3DProcessor* proc_;

	ShaderIndex_t vertS_;
	ShaderIndex_t pixS_;
	LayoutIndex_t layout_;

	void ConnectSlices(MeteoDataLoader* mdl,
					   std::vector<Vertex_t>& vertices,
					   std::vector<UINT>& indices,
					   FrontInfo_t& front,
					   FrontInfo_t& frontTarget,
					   int8_t currentSlice,
					   int8_t d);

public:
	FrontVisualizer (FrontAnalyzer* level0,
				 	 FrontAnalyzer* level1,
			  		 uint8_t zeroLevel,
					 Direct3DProcessor* proc);
	~FrontVisualizer();

	void ok ();

	void BuildPolygons (MeteoDataLoader* mdl);
};
