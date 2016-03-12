
#include "Builder.h"


FrontVisualizer::FrontVisualizer(FrontAnalyzer* level0,
								 FrontAnalyzer* level1,
								 uint8_t zeroLevel, 
								 Direct3DProcessor* proc)
try:
	world_     (XMMatrixIdentity ()),
	level0_    (level0),
	level1_    (level1),
	zeroLevel_ (zeroLevel),
	object_    (world_, true, true),
	proc_      (proc)
{}
_END_EXCEPTION_HANDLING(CTOR)

void FrontVisualizer::ok ()
{
	DEFAULT_OK_BLOCK
	if (zeroLevel_ < 0)
		_EXC_N (BAD_ZERO_LEVEL, "Bad zero level (%d)" _ zeroLevel_)
}


FrontVisualizer::~FrontVisualizer ()
{
	level0_ = nullptr;
	level1_ = nullptr;
	zeroLevel_ = -1;
	proc_ = nullptr;
}

void FrontVisualizer::BuildSingleTriangle (MeteoDataLoader* mdl,
										   std::vector<Vertex_t>& vertices,
										   std::vector<UINT>& indices,
										   FrontInfo_t& front,
										   FrontInfo_t& frontTarget, 
										   char d)
{
	uint32_t size0 = front.skeleton0_.size();
	int current = 0;

#define FP0(x) front.points_[front.skeleton0_[x]]
#define FP1(x) frontTarget.points_[front.skeleton0_[x]]

	double nearD = Dist(FP0(0), FP0(1));
	uint32_t nearPointIndex0 = 0;
	uint32_t nearPointIndex1 = 0;

	while (size0 - current > 0)
	{
		nearD = 10000000000.0;
		nearPointIndex0 = 0;
		for (uint32_t i = current + 1; i < size0; i++)
		{
			double newD = Dist(FP0(current), FP0(i));
			if (nearD > newD)
			{
				nearD = newD;
				nearPointIndex0 = i;
			}
		}

		POINT finalP = (FP0(current) + FP0(nearPointIndex0)) / 2;

		uint32_t size1 = frontTarget.skeleton0_.size();

		nearD = 10000000000.0;
		nearPointIndex1 = 0;
		for (uint32_t i = 0; i < size1; i++)
		{
			double newD = Dist(FP0(current), FP1(i));
			if (nearD > newD)
			{
				nearD = newD;
				nearPointIndex1 = i;
			}
		}
		if (nearD > FRONT_SHIFT*sqrt(2.0)) continue;

		float intensity0 = *mdl->Offset(FP0(current).x, FP0(current).y, zeroLevel_);
		float intensity1 = *mdl->Offset(FP0(nearPointIndex0).x, FP0(nearPointIndex0).y, zeroLevel_);
		float intensity2 = *mdl->Offset(FP1(nearPointIndex1).x, FP0(nearPointIndex1).y, zeroLevel_);

		indices.push_back(vertices.size());
		vertices.push_back(Vertex_t{ -REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(current).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* zeroLevel_,
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(current).y,
			102 + 154 * intensity0,
			102 - 66 * intensity0,
			102 * (1 - intensity0) });

		indices.push_back(vertices.size());
		vertices.push_back(Vertex_t{ -REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(nearPointIndex0).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* zeroLevel_,
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(nearPointIndex0).y,
			102 + 154 * intensity1,
			102 - 66 * intensity1,
			102 * (1 - intensity1) });

		indices.push_back(vertices.size());
		vertices.push_back(Vertex_t{ -REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(nearPointIndex1).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* zeroLevel_ + d,
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(nearPointIndex1).y,
			102 + 154 * intensity2,
			102 - 66 * intensity2,
			102 * (1 - intensity2) });

		current++;
	}
#undef FP
}


void FrontVisualizer::BuildPolygons(MeteoDataLoader* mdl)
{
	for (auto front : level0_->fronts_)
	{
		front.FindEquivalentFront(level1_->fronts_);
		if (front.equivalentFront_ == NO_EQUIVALENT) continue;

		std::vector<Vertex_t> vertices;
		std::vector<UINT> indices;

		BuildSingleTriangle(mdl,
							vertices,
							indices,
							front,
							level1_->fronts_[front.equivalentFront_],
							+1);

		BuildSingleTriangle(mdl,
							vertices,
							indices,
							level1_->fronts_[front.equivalentFront_],
							front,
							-1);

	}
}