#pragma once
#include "includes.h"


struct FloatPOINT
{
	float x, y;

	void Normalize();

	FloatPOINT& operator = (const POINT& that);
	FloatPOINT& operator += (const FloatPOINT& that);
};

struct SPOINT_t
{
public:
	int x, y;

	SPOINT_t ();
	SPOINT_t (int x, int y);

};

struct NeighbourN_t
{
	uint8_t neighbours;
	uint8_t n;

	void clear ();
};

double Dist(const SPOINT_t& x, const SPOINT_t& y);
SPOINT_t operator + (const SPOINT_t& x, const SPOINT_t& y);
SPOINT_t operator / (const SPOINT_t& x, int y);

struct FrontInfo_t
{
	std::vector<SPOINT_t> points_;
	std::vector<uint32_t> skeleton0_;
	std::vector<POINT> skeleton1_;
	SectionsType_t sections_;

	std::vector<uint32_t> near_;

	int32_t equivalentFront_;

	FrontInfo_t();

	void   clear();
	bool   empty();
	size_t size();
	SPOINT_t* data();
	void   AddPoint(int x, int y);

	void Process(MeteoDataLoader* mdl, int slice);

	void FillSkeleton0(MeteoDataLoader* mdl, int slice);

	void CalculateNear(const std::vector <FrontInfo_t>& data);

	void FindEquivalentFront(const std::vector <FrontInfo_t>& data);

};

class FrontAnalyzer : NZA_t
{
	std::vector<FrontInfo_t> fronts_;
	NeighbourN_t* set_;
	MeteoDataLoader* mdl_;
	int slice_;
	std::vector<uint32_t> toFlush_;

	friend class FrontVisualizer;

	int32_t NeighbourShift (uint8_t neighbour);

	uint8_t InverseNeighbour (uint8_t neighbour);

public:

	void ok ();
	FrontAnalyzer (MeteoDataLoader* mdl, int slice);
	~FrontAnalyzer ();

	std::vector<FrontInfo_t>& GetFront();

	void AnalyzeCell (int x, int y);

	void FlushBadCells ();

	void FillFronts (MeteoDataLoader* mdl, int slice);

	//void SortFronts ();

};