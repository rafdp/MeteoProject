#pragma once
#include "includes.h"

struct FloatPOINT
{
	float x, y;

	void Normalize();

	FloatPOINT& operator = (const POINT& that);
	FloatPOINT& operator += (const FloatPOINT& that);
};

double Dist(const POINT& x, const POINT& y);
POINT operator + (const POINT& x, const POINT& y);
POINT operator / (const POINT& x, int y);

struct FrontInfo_t
{
	std::vector<POINT> points_;
	std::vector<uint32_t> skeleton0_;
	SectionsType_t sections_;

	std::vector<uint32_t> near_;

	int32_t equivalentFront_;

	FrontInfo_t();

	void   clear();
	bool   empty();
	size_t size();
	POINT* data();
	void   AddPoint(int x, int y);

	void Process (MeteoDataLoader* mdl, int slice);

	void FillSkeleton0 (MeteoDataLoader* mdl, int slice);

	void CalculateNear(const std::vector <FrontInfo_t>& data);

	void FindEquivalentFront (const std::vector <FrontInfo_t>& data);

};

class FrontAnalyzer : NZA_t
{
	std::vector<FrontInfo_t> fronts_;
	unsigned char* set_;
	MeteoDataLoader* mdl_;
	int slice_;

	friend class FrontVisualizer;

public:

	void ok ();
	FrontAnalyzer (MeteoDataLoader* mdl, int slice);
	~FrontAnalyzer ();

	std::vector<FrontInfo_t>& GetFront();

	void RecursiveFrontFinder (int x, int y, FrontInfo_t& current);

};