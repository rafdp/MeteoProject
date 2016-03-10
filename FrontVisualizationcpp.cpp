
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

void FrontVisualizer::BuildPolygons()
{
	for (auto front : level0_->fronts_)
	{
		front.FindEquivalentFront(level1_->fronts_);
		if (front.equivalentFront_ == NO_EQUIVALENT) continue;

		int pointsLeft = // Need way to create the polygon wall
	}
}