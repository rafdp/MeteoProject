#pragma once
#include "includes.h"

class FrontVisualizer : NZA_t
{
	XMMATRIX world_;
	FrontAnalyzer* level0_;
	FrontAnalyzer* level1_;
	int8_t zeroLevel_;
	Direct3DObject object_;
	Direct3DProcessor* proc_;

public:
	FrontVisualizer (FrontAnalyzer* level0,
				 	 FrontAnalyzer* level1,
			  		 uint8_t zeroLevel,
					 Direct3DProcessor* proc);
	~FrontVisualizer();

	void ok ();

	void BuildPolygons ();
};
