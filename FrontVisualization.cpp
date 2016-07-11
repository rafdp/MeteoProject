
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
void FrontVisualizer::BuildSingleTriangle (MeteoDataLoader* mdl,
										   std::vector<Vertex_t>& vertices,
										   std::vector<UINT>& indices,
										   FrontInfo_t& front,
										   FrontInfo_t& frontTarget,
										   int8_t currentLevel,
										   int8_t d)
{
	uint32_t size0 = front.skeleton0_.size();
	int current = 0;

#define FP0(x) front.points_[front.skeleton0_[x]]
#define FP1(x) frontTarget.points_[frontTarget.skeleton0_[x]]

	double nearD = 10000000000.0;
	double nearD1 = 10000000000.0;
	uint32_t nearPointIndex0 = 0;
	uint32_t nearPointIndex1 = 0;
	uint32_t nearPointIndex11 = 0;

	while (size0 - current > 0)
	{
		Incrementator_t inc_ (current);
		//printf("loop %d\n", current);
		nearD = 10000000000.0;
		nearPointIndex0 = 0;
		for (uint32_t i = current + 1; i < size0 - 1; i++)
		{
			double newD = Dist(FP0(current), FP0(i));
			if (nearD > newD)
			{
				nearD = newD;
				nearPointIndex0 = i;
			}
		}

		if (nearD > SKELETON0_RANGE*2.0) continue;


		SPOINT_t finalP = (FP0(current) + FP0(nearPointIndex0)) / 2;

		uint32_t size1 = frontTarget.skeleton0_.size();

		nearD = 10000000000.0;
		nearD1 = 10000000000.0;
		nearPointIndex1 = 0;
		nearPointIndex11 = 0;
		for (uint32_t i = 0; i < size1 - 1; i++)
		{
			double newD = Dist(FP0(current), FP1(i));
			if (nearD > newD)
			{
				nearD = newD;
				nearPointIndex1 = i;
			}
			else
			if (nearD1 > newD)
			{
				nearD1 = newD;
				nearPointIndex11 = i;
			}
		}
		if (nearD > FRONT_SHIFT*2.0) continue;
		if (nearD1 > FRONT_SHIFT*2.0) continue;
		float intensity0 = *mdl->Offset(FP0(current).x, FP0(current).y, currentLevel);
		float intensity1 = *mdl->Offset(FP0(nearPointIndex0).x, FP0(nearPointIndex0).y, currentLevel);
		float intensity2 = *mdl->Offset(FP1(nearPointIndex1).x, FP1(nearPointIndex1).y, currentLevel + d);
		float intensity3 = *mdl->Offset(FP1(nearPointIndex11).x, FP1(nearPointIndex11).y, currentLevel + d);

		Vertex_t currentVertex = {};

		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(current).x,
							 -REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* currentLevel,
							 -REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(current).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity0,
							   0.85f - 0.85f * intensity0,
							   0.85f - 0.139f * intensity0,
							   sin(intensity0 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);

		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP0(nearPointIndex0).x,
						 	 -REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* currentLevel,
						 	 -REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP0(nearPointIndex0).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity1,
							   0.85f - 0.85f * intensity1,
							   0.85f - 0.139f * intensity1,
						       sin(intensity1 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);

		indices.push_back(vertices.size());
		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP1(nearPointIndex1).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* (currentLevel + d),
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP1(nearPointIndex1).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity2,
			0.85f - 0.85f * intensity2,
			0.85f - 0.139f * intensity2,
			sin(intensity2 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);


		indices.push_back(vertices.size());
		currentVertex.SetPos(-REGION_X / 2.0f + REGION_X / (DATA_WIDTH)* FP1(nearPointIndex11).x,
			-REGION_Z / 2.0f + REGION_Z / (SLICES - 1)* (currentLevel + d),
			-REGION_Y / 2.0f + REGION_Y / (DATA_HEIGHT)* FP1(nearPointIndex11).y);
		currentVertex.SetColor(0.85f + 0.107f * intensity3,
			0.85f - 0.85f * intensity3,
			0.85f - 0.139f * intensity3,
			sin(intensity3 * 3.141593f / 2.0f));
		vertices.push_back(currentVertex);

		indices.push_back(vertices.size() - 2);

		//current++;
	}
#undef FP
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

		BuildSingleTriangle(mdl,
							vertices,
							indices,
							front,
							level1_->fronts_[front.equivalentFront_],
							zeroLevel_,
							+1);

		BuildSingleTriangle(mdl,
							vertices,
							indices,
							level1_->fronts_[front.equivalentFront_],
							front,
							zeroLevel_ + 1,
							-1);

	}
	if (vertices.size() == 0) return;
	object_->AddIndexArray(indices.data (), indices.size());
	proc_->AttachShaderToObject(object_, vertS_);
	proc_->AttachShaderToObject(object_, pixS_);
	proc_->SetLayout(object_, layout_);
	proc_->RegisterObject(object_);
	//object_->SetupBuffers(proc_->GetDevice());
}