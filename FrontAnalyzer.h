#pragma once
#include "includes.h"

struct IntPair_t
{
	int x, y;
};

class FrontAnalyzer : NZA_t
{
	std::vector<std::vector<IntPair_t>> fronts_;
	unsigned char set_[DATA_WIDTH][DATA_HEIGHT];
	MeteoDataLoader* mdl_;
	int slice_;

public:

	void ok ();
	FrontAnalyzer (MeteoDataLoader* mdl, int slice);
	~FrontAnalyzer ();

	void RecursiveFrontFinder (int x, int y, std::vector<IntPair_t>& current);
};