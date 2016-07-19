
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
	object_    (new (GetValidObjectPtr()) Direct3DObject(world_, true, true)),
	proc_      (proc),
	vertS_     (),
	pixS_      (),
	layout_    ()
{
	if (!proc)
		_EXC_N(NULL_PROC, "Got null D3DProcessor*")
	vertS_ = proc_->LoadShader("shaders.hlsl", "VShader", SHADER_VERTEX);
	pixS_ = proc_->LoadShader("shaders.hlsl", "PShader", SHADER_PIXEL);
	layout_ = proc_->AddLayout(vertS_, true, false, false, true);
}
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

Incrementator_t::Incrementator_t(int& data_) : 
	data (data_)
{
}

Incrementator_t::~Incrementator_t()
{
	data++;
}
/*
void FrontVisualizer::BuildSingleTriangle (MeteoDataLoader* mdl,
										   std::vector<Vertex_t>& vertices,
										   std::vector<UINT>& indices,
										   FrontInfo_t& front,
										   FrontInfo_t& frontTarget,
										   int8_t currentSlice,
										   int8_t d)
{
	uint32_t size0 = front.skeleton0_.size();
	int current = 0;

#define FP0(x) front.points_[front.skeleton0_[x]]
#define FP1(x) frontTarget.points_[frontTarget.skeleton0_[x]]

	double nearD = 10000000000.0;
	uint32_t nearPointIndex1 = 0;

	while (size0 - current > 1)
	{
		Incrementator_t inc_ (current);
		//printf("loop %d\n", current);
		nearD = Dist(FP0(current), FP0(current + 1));

		//SPOINT_t finalP = (FP0(current) + FP0(current + 1)) / 2;

		uint32_t size1 = frontTarget.skeleton0_.size();

		nearD = 10000000000.0;
		nearPointIndex1 = 0;
		for (uint32_t i = 0; i < size1 - 1; i++)
		{
			double newD = Dist(FP0(current), FP1(i));
			if (nearD > newD)
			{
				nearD = newD;
				nearPointIndex1 = i;
			}
		}
		if (nearD > FRONT_SHIFT*2.0) continue;
		float intensity0 = *mdl->Offset(FP0(current).x, FP0(current).y, currentSlice);
		float intensity1 = *mdl->Offset(FP0(current + 1).x, FP0(current + 1).y, currentSlice);
		float intensity2 = *mdl->Offset(FP1(nearPointIndex1).x, FP1(nearPointIndex1).y, currentSlice + d);
		float intensity3 = *mdl->Offset(FP1(nearPointIndex1 + 1).x, FP1(nearPointIndex1 + 1).y, currentSlice + d);

		Vertex_t currentVertex = {};

		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(current).x,
							 -REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* currentSlice,
							 -REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(current).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity0,
							   0.85f - 0.85f * intensity0,
							   0.85f - 0.139f * intensity0,
							   sin(intensity0 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);

		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(current + 1).x,
						 	 -REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* currentSlice,
						 	 -REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(current + 1).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity1,
							   0.85f - 0.85f * intensity1,
							   0.85f - 0.139f * intensity1,
						       sin(intensity1 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);

		indices.push_back(vertices.size());
		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP1(nearPointIndex1).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* (currentSlice + d),
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP1(nearPointIndex1).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity2,
			0.85f - 0.85f * intensity2,
			0.85f - 0.139f * intensity2,
			sin(intensity2 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);


		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP1(nearPointIndex1 + 1).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* (currentSlice + d),
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP1(nearPointIndex1 + 1).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity3,
			0.85f - 0.85f * intensity3,
			0.85f - 0.139f * intensity3,
			sin(intensity3 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);

		if (Dist(FP0(current), FP1(nearPointIndex1 + 1)) > nearD)
			indices.push_back(vertices.size() - 3);
		else
			indices.push_back(vertices.size() - 2);
		//current++;
	}
#undef FP
}*/

void FrontVisualizer::ConnectSlices(MeteoDataLoader* mdl,
									std::vector<Vertex_t>& vertices,
									std::vector<UINT>& indices,
									FrontInfo_t& front,
									FrontInfo_t& frontTarget,
									int8_t currentSlice,
									int8_t d)
{
	BEGIN_EXCEPTION_HANDLING
	uint32_t size0 = front.skeleton0_.size();
	uint32_t size1 = frontTarget.skeleton0_.size();

	if (size0 <= 5 || size1 <= 5) return;

#define FP0(x) front.points_[front.skeleton0_[x]]
#define FP1(x) frontTarget.points_[frontTarget.skeleton0_[x]]

	/*int8_t dIndex = 2;
	uint32_t nearPointIndex = FindEquivalentPoint (&front, &frontTarget, size0/2).point;
	if (nearPointIndex >= size1 - 2) dIndex = -1;
	SPOINT_t D0 = FP0(size0 / 2 + dIndex) - FP0(size0 / 2);
	SPOINT_t D1 = FP1(nearPointIndex + dIndex) - FP1(nearPointIndex);
	bool sameDirection = (D0.x * D1.x > 0) && (D0.y * D1.y > 0);*/
	//bool sameDirection = Dist (FP0(0), FP1(0)) < Dist(FP0(0), FP1(size1/2));
	bool sameDirection = true;
#define IND1(x) (sameDirection ? (x) : (size1 - 1 - (x)))
#undef FP1
#define FP1(x) frontTarget.points_[frontTarget.skeleton0_[IND1(x)]]

	bool fetchBeginFromFirst = true;
	bool fetchEndFromFirst = true;

	EquivalentPointData_t eqBegin0to1 = FindEquivalentPoint(&front, &frontTarget, 0);
	EquivalentPointData_t eqBegin1to0 = FindEquivalentPoint(&frontTarget, &front, IND1 (0));

	EquivalentPointData_t eqEnd0to1 = FindEquivalentPoint(&front, &frontTarget, size0 - 1);
	EquivalentPointData_t eqEnd1to0 = FindEquivalentPoint(&frontTarget, &front, IND1(size1 - 1));

	uint32_t eqBegin = eqBegin1to0.point;
	uint32_t eqEnd = eqEnd1to0.point;

	size_t sizeEnd = size1;

	if (eqBegin0to1.d < eqBegin1to0.d)
	{
		fetchBeginFromFirst = false;
		eqBegin = eqBegin0to1.point;
	}

	if (eqEnd0to1.d < eqEnd1to0.d)
	{
		fetchEndFromFirst = false;
		eqEnd = eqEnd0to1.point;
		sizeEnd = size0;
	}

	printf ("SameDirection %d\nFetch from low %d\n", sameDirection, fetchBeginFromFirst);


	Vertex_t currentVertex = {};

#define SET_POS(x, y, zShift) \
currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* x, \
					 -REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* (currentSlice + zShift), \
					 -REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* y);

#define SET_COLOR(intensity) \
currentVertex.SetColor(0.85f + 0.107f * intensity, \
					   0.85f - 0.85f * intensity, \
					   0.85f - 0.139f * intensity, \
					   sin(intensity * 3.141593f / 2.0f));

#define PUSH vertices.push_back(currentVertex);

#define INDEX(shift) indices.push_back(vertices.size() - shift);

#define FP_FETCH_BEGIN_0(x) (fetchBeginFromFirst ? FP0(x) : FP1(x))
#define FP_FETCH_BEGIN_1(x) (fetchBeginFromFirst ? FP1(x) : FP0(x))
#define LEVEL_SHIFT_FETCH_BEGIN_0 (fetchBeginFromFirst ? 0 : d)
#define LEVEL_SHIFT_FETCH_BEGIN_1 (fetchBeginFromFirst ? d : 0)
#define SHIFT_FETCH_BEGIN_0 (fetchBeginFromFirst ? eqBegin : 0)
#define SHIFT_FETCH_BEGIN_1 (fetchBeginFromFirst ? 0 : eqBegin)
#define SIZE_FETCH_BEGIN_0 (fetchBeginFromFirst ? size0 : size1)
#define SIZE_FETCH_BEGIN_1 (fetchBeginFromFirst ? size1 : size0)

#define FP_FETCH_END_0(x) (fetchEndFromFirst ? FP0(x) : FP1(x))
#define FP_FETCH_END_1(x) (fetchEndFromFirst ? FP1(x) : FP0(x))
#define LEVEL_SHIFT_FETCH_END_0 (fetchEndFromFirst ? 0 : d)
#define LEVEL_SHIFT_FETCH_END_1 (fetchEndFromFirst ? d : 0)
#define SIZE_FETCH_END_0 (fetchEndFromFirst ? size0 : size1)
#define SIZE_FETCH_END_1 (fetchEndFromFirst ? size1 : size0)

	float intensity02 = *mdl->Offset(FP_FETCH_BEGIN_1 (0).x,
									 FP_FETCH_BEGIN_1 (0).y,
									 currentSlice + LEVEL_SHIFT_FETCH_BEGIN_1);
	//printf("About to fetch beginning\n");
	if (eqBegin >= SIZE_FETCH_BEGIN_0 - 2) return;
	for (uint32_t i = 0; i < eqBegin - 1; i++)
	{
		float intensity0 = *mdl->Offset(FP_FETCH_BEGIN_0(i).x,
										FP_FETCH_BEGIN_0(i).y,
										currentSlice + LEVEL_SHIFT_FETCH_BEGIN_0);
		float intensity1 = *mdl->Offset(FP_FETCH_BEGIN_0(i + 1).x,
										FP_FETCH_BEGIN_0(i + 1).y,
										currentSlice + LEVEL_SHIFT_FETCH_BEGIN_0);

		INDEX(0)
		SET_POS (FP_FETCH_BEGIN_0(i).x, 
				 FP_FETCH_BEGIN_0(i).y, 
				 LEVEL_SHIFT_FETCH_BEGIN_0)
		SET_COLOR (intensity0)
		PUSH

		INDEX(0)
		SET_POS (FP_FETCH_BEGIN_0(i + 1).x, 
				 FP_FETCH_BEGIN_0(i + 1).y, 
				 LEVEL_SHIFT_FETCH_BEGIN_0)
		SET_COLOR(intensity1)
		PUSH

		INDEX(0)
		SET_POS (FP_FETCH_BEGIN_1(0).x, 
				 FP_FETCH_BEGIN_1(0).y, 
				 LEVEL_SHIFT_FETCH_BEGIN_1)
		SET_COLOR(intensity02)
		PUSH
		if (i >= SIZE_FETCH_BEGIN_0 - 2) return;
	}
	//printf("About to connect the middle parts\n");

	if (SHIFT_FETCH_BEGIN_0 >= SIZE_FETCH_BEGIN_0 - 1) return;

	for (uint32_t i = 0; i < SIZE_FETCH_BEGIN_1 - 1; i++)
	{
		float intensity0 = *mdl->Offset(FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i).x,
										FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i).y,
										currentSlice + LEVEL_SHIFT_FETCH_BEGIN_0);
		//printf ("index = %d, size = %d\n", SHIFT_FETCH_BEGIN_0 + i + 1, SIZE_FETCH_BEGIN_0);
		//_getch();
		float intensity1 = *mdl->Offset(FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i + 1).x,
										FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i + 1).y,
										currentSlice + LEVEL_SHIFT_FETCH_BEGIN_0);
		float intensity2 = *mdl->Offset(FP_FETCH_BEGIN_1(i).x,
										FP_FETCH_BEGIN_1(i).y,
										currentSlice + LEVEL_SHIFT_FETCH_BEGIN_1);
		float intensity3 = *mdl->Offset(FP_FETCH_BEGIN_1(i + 1).x,
										FP_FETCH_BEGIN_1(i + 1).y,
										currentSlice + LEVEL_SHIFT_FETCH_BEGIN_1);

		INDEX(0)
		SET_POS(FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i).x, 
				FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i).y, 
				LEVEL_SHIFT_FETCH_BEGIN_0)
		SET_COLOR(intensity0)
		PUSH

		INDEX(0)
		SET_POS(FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i + 1).x, 
				FP_FETCH_BEGIN_0(SHIFT_FETCH_BEGIN_0 + i + 1).y, 
				LEVEL_SHIFT_FETCH_BEGIN_0)
		SET_COLOR(intensity1)
		PUSH

		INDEX(0)
		INDEX(0)
		SET_POS(FP_FETCH_BEGIN_1(i).x, 
				FP_FETCH_BEGIN_1(i).y, 
				LEVEL_SHIFT_FETCH_BEGIN_1)
		SET_COLOR(intensity2)
		PUSH

		INDEX(0)
		SET_POS(FP_FETCH_BEGIN_1(i + 1).x, 
				FP_FETCH_BEGIN_1(i + 1).y, 
				LEVEL_SHIFT_FETCH_BEGIN_1)
		SET_COLOR(intensity3)
		PUSH
		INDEX(3)
		if (i + SHIFT_FETCH_BEGIN_0 >= SIZE_FETCH_BEGIN_0 - 2) return;
	}


	intensity02 = *mdl->Offset(FP_FETCH_END_1(sizeEnd - 1).x,
							   FP_FETCH_END_1(sizeEnd - 1).y,
							   currentSlice + LEVEL_SHIFT_FETCH_END_1);

	//printf("About to fetch ending\n");

	for (uint32_t i = 0; 
		 i < SIZE_FETCH_END_0 - 1 - eqEnd;
		 i++)
	{
		float intensity0 = *mdl->Offset(FP_FETCH_END_0(eqEnd + i).x,
										FP_FETCH_END_0(eqEnd + i).y,
										currentSlice + LEVEL_SHIFT_FETCH_END_0);
		float intensity1 = *mdl->Offset(FP_FETCH_END_0(eqEnd + i + 1).x,
										FP_FETCH_END_0(eqEnd + i + 1).y,
										currentSlice + LEVEL_SHIFT_FETCH_END_0);

		INDEX(0)
		SET_POS(FP_FETCH_END_0(eqEnd + i).x,
				FP_FETCH_END_0(eqEnd + i).y,
				LEVEL_SHIFT_FETCH_END_0)
		SET_COLOR(intensity0)
		PUSH

		INDEX(0)
		SET_POS(FP_FETCH_END_0(eqEnd + i + 1).x,
				FP_FETCH_END_0(eqEnd + i + 1).y,
				LEVEL_SHIFT_FETCH_END_0)
		SET_COLOR(intensity1)
		PUSH

		INDEX(0)
		SET_POS(FP_FETCH_END_1(sizeEnd - 1).x,
				FP_FETCH_END_1(sizeEnd - 1).y,
				LEVEL_SHIFT_FETCH_END_1)
		SET_COLOR(intensity02)
		PUSH
	}

#undef FP0
#undef FP1
#undef IND1
#undef SET_POS
#undef SET_COLOR
#undef PUSH
#undef INDEX
#undef FP_FETCH_BEGIN_0
#undef FP_FETCH_BEGIN_1
#undef LEVEL_SHIFT_FETCH_BEGIN_0
#undef LEVEL_SHIFT_FETCH_BEGIN_1
#undef SHIFT_FETCH_BEGIN_0
#undef SHIFT_FETCH_BEGIN_1
#undef FP_FETCH_END_0
#undef FP_FETCH_END_1
#undef LEVEL_SHIFT_FETCH_END_0
#undef LEVEL_SHIFT_FETCH_END_1
END_EXCEPTION_HANDLING (CONNECT_SLICES)

}


void FrontVisualizer::BuildPolygons(MeteoDataLoader* mdl)
{
	//printf("OBJECT %llu\n", object_->GetID ());
	std::vector<Vertex_t>& vertices = object_->GetVertices();
	std::vector<UINT> indices;

	int i = 0;

	for (auto front : level0_->fronts_)
	{
		//printf("    loop %d\n", i);
		front.FindEquivalentFront(level1_->fronts_);
		i++;
		if (front.equivalentFront_ == NO_EQUIVALENT) continue;

		ConnectSlices (mdl,
					   vertices,
					   indices,
					   front,
					   level1_->fronts_[front.equivalentFront_],
					   zeroLevel_,
					   +1);

		/*BuildSingleTriangle(mdl,
							  vertices,
							  indices,
							  level1_->fronts_[front.equivalentFront_],
							  front,
							  zeroLevel_ + 1,
							  -1);*/

	}
	if (vertices.size() == 0) return;
	object_->AddIndexArray(indices.data (), indices.size());
	proc_->AttachShaderToObject(object_, vertS_);
	proc_->AttachShaderToObject(object_, pixS_);
	proc_->SetLayout(object_, layout_);
	proc_->RegisterObject(object_);
	//object_->SetupBuffers(proc_->GetDevice());
}

EquivalentPointData_t FindEquivalentPoint(FrontInfo_t * front, FrontInfo_t * frontTarget, uint32_t point)
{
#define FP0(x) front->points_[front->skeleton0_[x]]
#define FP1(x) frontTarget->points_[frontTarget->skeleton0_[x]]

	double nearD = 10000000000.0;
	uint32_t nearPointIndex = 0;
	size_t size = frontTarget->skeleton0_.size ();

	for (uint32_t i = 0; i < size; i++)
	{
		double newD = Dist(FP0(point), FP1(i));
		
		if (nearD > newD)
		{
			nearD = newD;
			nearPointIndex = i;
		}
	}
	return EquivalentPointData_t { nearPointIndex, nearD };

#undef FP0
#undef FP1
}
