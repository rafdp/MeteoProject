#pragma once
#include "includes.h"

struct FrontInfo_t
{
	std::vector<POINT> points_;
	unsigned int sections_;

	void   clear();
	bool   empty();
	size_t size();
	POINT* data();
	void   AddPoint(int x, int y);
};

class FrontAnalyzer : NZA_t
{
	std::vector<FrontInfo_t> fronts_;
	unsigned char set_[DATA_WIDTH][DATA_HEIGHT];
	MeteoDataLoader* mdl_;
	int slice_;

public:

	void ok ();
	FrontAnalyzer (MeteoDataLoader* mdl, int slice);
	~FrontAnalyzer ();

	void RecursiveFrontFinder (int x, int y, FrontInfo_t& current);
};