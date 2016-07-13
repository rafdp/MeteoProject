
#include "Builder.h"

#define NEED_DEBUG 0

#if (NEED_DEBUG > 0)
#define DEBUG_X 29
#define DEBUG_Y 21
#define DEBUG_SLICE 0
int CURRENT_SLICE = 0;
#endif

FrontAnalyzer::FrontAnalyzer (MeteoDataLoader* mdl, int slice)
try :
	fronts_(),
	set_   (new NeighbourN_t[DATA_WIDTH*DATA_HEIGHT]),
	mdl_   (mdl),
	slice_ (slice),
	toFlush_ ()
{
#if (NEED_DEBUG > 0)
	CURRENT_SLICE = slice;
#endif
	for (uint32_t i = 0; i < DATA_WIDTH * DATA_HEIGHT; i++)
		set_[i] = { 0, CELL_NOT_CHECKED};
	if (!mdl_)
		_EXC_N(NULL_THIS, "Null MeteoDataLoader ptr");

	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			if (set_[x*DATA_HEIGHT + y].n != CELL_NOT_CHECKED) continue;
			AnalyzeCell(x, y);
		}
	}

	FlushBadCells ();
	FillFronts	(mdl, slice);
	//FrontThinner ft (&fronts_);
	//ft.FindSkeleton();


	//printf("Slice %d fronts %llu\n", slice, fronts_.size());
	if (slice != 0) return;
	FILE* f = nullptr;
	fopen_s(&f, "Front.data", "wb");
	fwrite(&DATA_WIDTH, sizeof(int), 1, f);
	fwrite(&DATA_HEIGHT, sizeof(int), 1, f);
	size_t Nfronts_ = fronts_.size();
	fwrite(&Nfronts_, sizeof(size_t), 1, f);
	size_t currentSize = 0;
	for (uint32_t i = 0; i < Nfronts_; i++)
	{
		currentSize = fronts_[i].points_.size();
		//printf("SIZE %llu\n", currentSize);

		fwrite(&currentSize, sizeof(size_t), 1, f);
		for (uint32_t j = 0; j < currentSize; j++)
		{
			fwrite(&fronts_[i].points_[j], sizeof(POINT), 1, f);
		}
		//fwrite(fronts_[i].skeleton0_.data(), sizeof(POINT), currentSize, f);
	}
	//printf("%llu\n", fronts_.size());
	//while (!GetAsyncKeyState(VK_SPACE));
	fclose(f);
}
_END_EXCEPTION_HANDLING(CTOR)

FrontAnalyzer::~FrontAnalyzer()
{
	fronts_.clear();
	mdl_ = nullptr;
	slice_ = -1;
	delete[] set_;
	set_ = nullptr;
}

std::vector<FrontInfo_t> & FrontAnalyzer::GetFront()
{
	return fronts_;
}

int32_t FrontAnalyzer::NeighbourShift(uint8_t neighbour)
{
	BEGIN_EXCEPTION_HANDLING
#define RETURN(name, ret) if (NEIGHBOUR_##name == neighbour) return ret; else

	RETURN (TOPLEFT,     -1 * DATA_HEIGHT - 1)
	RETURN (TOP,         -1)
	RETURN (TOPRIGHT,    +1 * DATA_HEIGHT - 1)
	RETURN (LEFT,        -1 * DATA_HEIGHT)
	RETURN (RIGHT,       +1 * DATA_HEIGHT)
	RETURN (BOTTOMLEFT,  -1 * DATA_HEIGHT + 1)
	RETURN (BOTTOM,      +1)
	RETURN (BOTTOMRIGHT, +1*DATA_HEIGHT + 1)
		
	_EXC_N (INVALID_NEIGHBOUR_INDEX, "Invalid neighbour index (0b%d%d%d%d%d%d%d%d)" _ 
			neighbour & 0b10000000 ? 1 : 0 _ 
			neighbour & 0b01000000 ? 1 : 0 _
			neighbour & 0b00100000 ? 1 : 0 _
			neighbour & 0b00010000 ? 1 : 0 _
			neighbour & 0b00001000 ? 1 : 0 _
			neighbour & 0b00000100 ? 1 : 0 _
			neighbour & 0b00000010 ? 1 : 0 _
			neighbour & 0b00000001 ? 1 : 0)

	return 0;
END_EXCEPTION_HANDLING(NEIGHBOUR_SHIFT)
#undef RETURN
}

uint8_t FrontAnalyzer::InverseNeighbour(uint8_t neighbour)
{
	uint8_t result = 0;
	for (uint8_t i = 0; i < 8; i ++)
		result |= ((neighbour >> i) & 1) << (7 - i);
	return result;
}

void FrontAnalyzer::ok()
{
	DEFAULT_OK_BLOCK
	if (slice_ == -1)
		_EXC_N(NEGATIVE_SLICE, "Negative slice");

	if (SECTIONS_X * SECTIONS_Y > 8 * sizeof (SectionsType_t))
		_EXC_N(NOT_ENOUGH_SECTIONS, "Not enough section bits");

}

void FrontAnalyzer::AnalyzeCell(int x, int y)
{
	if (set_[x*DATA_HEIGHT + y].n != CELL_NOT_CHECKED) return;
	set_[x*DATA_HEIGHT + y].n = 0;
	float intensity = *mdl_->Offset(x, y, slice_);

	if (intensity < 0.0001f || intensity > 12.001f)
	{
		set_[x*DATA_HEIGHT + y].clear();
		return;
	}

	//set_[x*DATA_HEIGHT + y] = 2;

	//char n = 0;
#if (NEED_DEBUG > 0)
	if (x == DEBUG_X && y == DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE)
		printf("DEBUG     RMA before (%x %d)\n", set_[x*DATA_HEIGHT + y].neighbours, set_[x*DATA_HEIGHT + y].n);
#endif

	if (x >= 1 &&
		((intensity = *mdl_->Offset(x - 1, y, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_LEFT;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (x < DATA_WIDTH - 1 &&
		((intensity = *mdl_->Offset(x + 1, y, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_RIGHT;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (y >= 1 &&
		((intensity = *mdl_->Offset(x, y - 1, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_TOP;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (y < DATA_HEIGHT - 1 &&
		((intensity = *mdl_->Offset(x, y + 1, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_BOTTOM;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (y >= 1 && x >= 1 &&
		((intensity = *mdl_->Offset(x - 1, y - 1, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_TOPLEFT;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (y >= 1 && x <= DATA_WIDTH - 1 &&
		((intensity = *mdl_->Offset(x + 1, y - 1, slice_)) >= 0.0001f
			&& intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_TOPRIGHT;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (y <= DATA_HEIGHT - 1 && x >= 1 &&
		((intensity = *mdl_->Offset(x - 1, y + 1, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_BOTTOMLEFT;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (y <= DATA_HEIGHT - 1 && x <= DATA_WIDTH - 1 &&
		((intensity = *mdl_->Offset(x + 1, y + 1, slice_)) >= 0.0001f &&
			intensity <= 12.001f))
	{
		set_[x*DATA_HEIGHT + y].neighbours |= NEIGHBOUR_BOTTOMRIGHT;
		set_[x*DATA_HEIGHT + y].n++;
	}

	if (set_[x*DATA_HEIGHT + y].n > 2)
		toFlush_.push_back(x*DATA_HEIGHT + y);

	uint8_t bits = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		if (set_[x*DATA_HEIGHT + y].neighbours >> i & 1) bits++;
	}

	if (bits != set_[x*DATA_HEIGHT + y].n)
		_EXC_N (NEIGHBOURS_N_NOT_COINCIDENT, "Neighbours bits (0b%d%d%d%d%d%d%d%d) and neighbour count (%d) not coincident" _ 
				set_[x*DATA_HEIGHT + y].neighbours & 0b10000000 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b01000000 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b00100000 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b00010000 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b00001000 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b00000100 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b00000010 ? 1 : 0 _
				set_[x*DATA_HEIGHT + y].neighbours & 0b00000001 ? 1 : 0 _ 
				set_[x*DATA_HEIGHT + y].n)

#if (NEED_DEBUG > 0)
	if (x == DEBUG_X && y == DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE)
		printf("DEBUG     RMA after (%x %d)\n", set_[x*DATA_HEIGHT + y].neighbours, set_[x*DATA_HEIGHT + y].n);
#endif
}

void FrontAnalyzer::FlushBadCells ()
{
	BEGIN_EXCEPTION_HANDLING

	for (auto iter : toFlush_)
	{
#if (NEED_DEBUG > 0)

#define DELETE_LINKS(name) \
if (iter == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE) \
		printf ("DEBUG     FLUSH before (%x %d)\n", set_[iter].neighbours, set_[iter].n); \
if (iter + NeighbourShift(NEIGHBOUR_##name) == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE) \
		printf ("DEBUG     FLUSH by neighbour before (%x %d)\n", set_[iter + NeighbourShift(NEIGHBOUR_##name)].neighbours, set_[iter + NeighbourShift(NEIGHBOUR_##name)].n); \
if (set_[iter].neighbours & NEIGHBOUR_##name) \
{ \
	set_[iter].n--; \
	set_[iter].neighbours &= ~NEIGHBOUR_##name; \
	set_[iter + NeighbourShift(NEIGHBOUR_##name)].neighbours &= ~InverseNeighbour (NEIGHBOUR_##name); \
	set_[iter + NeighbourShift(NEIGHBOUR_##name)].n--; \
} \
if (iter == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE) \
		printf ("DEBUG     FLUSH after (%x %d)\n", set_[iter].neighbours, set_[iter].n); \
if (iter + NeighbourShift(NEIGHBOUR_##name) == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE) \
		printf ("DEBUG     FLUSH by neighbour after (%x %d)\n", set_[iter + NeighbourShift(NEIGHBOUR_##name)].neighbours, set_[iter + NeighbourShift(NEIGHBOUR_##name)].n); 

#else

#define DELETE_LINKS(name) \
if (set_[iter].neighbours & NEIGHBOUR_##name) \
{\
	set_[iter].n--; \
	set_[iter].neighbours &= ~NEIGHBOUR_##name; \
	set_[iter + NeighbourShift(NEIGHBOUR_##name)].neighbours &= ~InverseNeighbour (NEIGHBOUR_##name); \
	set_[iter + NeighbourShift(NEIGHBOUR_##name)].n--; \
} \
if (set_[iter].n == 2) continue;
//
#endif
		DELETE_LINKS (TOPLEFT)
		DELETE_LINKS (TOP)
		DELETE_LINKS (TOPRIGHT)
		DELETE_LINKS (LEFT)
		DELETE_LINKS (RIGHT)
		DELETE_LINKS (BOTTOMLEFT)
		DELETE_LINKS (BOTTOM)
		DELETE_LINKS (BOTTOMRIGHT)
		

#undef DELETE_LINKS
		//set_[iter].clear ();
	}
	END_EXCEPTION_HANDLING (FLUSH_BAD_CELLS)
}

void FrontAnalyzer::FillFronts (MeteoDataLoader* mdl, int slice)
{
	BEGIN_EXCEPTION_HANDLING
	uint32_t pointIndex = 0;
	uint32_t loop = 0;
	uint32_t pointIndexOld = 0;
	FrontInfo_t current;
	bool next = false;

	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			pointIndex = x*DATA_HEIGHT + y;

			if (set_[pointIndex].n != 1) continue;
			
			current.points_.push_back(SPOINT_t(x, y));
			loop = 0;
			next = false;
			while (!loop || set_[pointIndex].n == 2)
			{
				loop++;
#if (NEED_DEBUG > 0)
				if (next || pointIndex == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE)
				{
					next = true;
					printf("DEBUG     Fill fronts loop%d %x %d %d\n",
						loop,
						set_[pointIndex].neighbours,
						pointIndex / DATA_HEIGHT,
						pointIndex % DATA_HEIGHT);
				}
#endif
				pointIndexOld = pointIndex;
				
				pointIndex += NeighbourShift (set_[pointIndexOld].neighbours);
#if (NEED_DEBUG > 0)
				if (next || pointIndex == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE)
					printf("DEBUG     Fill fronts NEXT before flush loop%d %x %d %d\n",
						loop,
						set_[pointIndex],
						pointIndex / DATA_HEIGHT,
						pointIndex % DATA_HEIGHT);
#endif
				set_[pointIndex].neighbours &= ~InverseNeighbour (set_[pointIndexOld].neighbours);
				set_[pointIndexOld].clear ();
#if (NEED_DEBUG > 0)
				if (next || pointIndex == DEBUG_X*DATA_HEIGHT + DEBUG_Y && CURRENT_SLICE == DEBUG_SLICE)
					printf("DEBUG     Fill fronts NEXT after flush loop%d %x %d %d\n",
						loop,
						set_[pointIndex],
						pointIndex / DATA_HEIGHT,
						pointIndex % DATA_HEIGHT);
#endif
				current.AddPoint (pointIndex / DATA_HEIGHT,
								  pointIndex % DATA_HEIGHT);

			}
			set_[pointIndex].clear ();
			if (current.size() > 2)
			{
				//current.Process(mdl, slice);
				if (next) printf("DEBUG     Before adding %llu\n", fronts_.size());
				fronts_.push_back(current);
				if (next) printf("DEBUG     After adding %llu\n", fronts_.size());
				current.clear();
			}
		}
	}
	END_EXCEPTION_HANDLING (FILL_FRONTS)
}


double Dist(const SPOINT_t& x, const SPOINT_t& y)
{
	return sqrt((y.x - x.x)*(y.x - x.x) + (y.y - x.y)*(y.y - x.y));
}

SPOINT_t operator + (const SPOINT_t& x, const SPOINT_t& y)
{
	return SPOINT_t(x.x + y.x, x.y + y.y);
}

SPOINT_t operator / (const SPOINT_t& x, int y)
{
	return SPOINT_t(x.x / y, x.y / y);
}


void FloatPOINT::Normalize()
{
	float l = sqrt(x*x + y*y);
	if (fabs(l) < 0.0001f) return;

	x /= l;
	y /= l;
}

FloatPOINT& FloatPOINT::operator=(const POINT& that)
{
	x = that.x*1.0f;
	y = that.y*1.0f;
	return *this;
}

FloatPOINT& FloatPOINT::operator+=(const FloatPOINT& that)
{
	x += that.x;
	y += that.y;
	return *this;
}

FrontInfo_t::FrontInfo_t() :
	points_(),
	sections_(),
	near_(),
	equivalentFront_(-1)
{
}


void FrontInfo_t::clear()
{
	points_.clear();
	sections_ = 0;
}

bool FrontInfo_t::empty()
{
	return points_.empty();
}

size_t FrontInfo_t::size()
{
	return points_.size();
}

SPOINT_t * FrontInfo_t::data()
{
	if (empty()) return nullptr;
	return points_.data();
}

void FrontInfo_t::AddPoint(int x, int y)
{
	unsigned char bit = int(x*SCALING_X) + 8 * int(y*SCALING_Y);
	sections_ |= 1 << bit;
	points_.push_back({ x, y });
}


void FrontInfo_t::Process(MeteoDataLoader* mdl, int slice)
{
	FillSkeleton0(mdl, slice);
}


void FrontInfo_t::FillSkeleton0(MeteoDataLoader* mdl, int slice)
{
	if (points_.empty()) return;
	if (!skeleton0_.empty()) skeleton0_.clear();
	SPOINT_t current(points_[0]);
	skeleton0_.push_back(0);
	for (auto iter = points_.begin() + 1; iter < points_.end(); iter++)
	{
		float intensity = 0.0f;
		char n = 0;
		if (iter->x >= 1 && ((intensity = *mdl->Offset(iter->x - 1, iter->y, slice)) >= 0.001f && intensity <= 12.001f))
			n++;
		if (iter->x < DATA_WIDTH - 1 && ((intensity = *mdl->Offset(iter->x + 1, iter->y, slice)) >= 0.0001f && intensity <= 12.001f))
			n++;
		if (iter->y >= 1 && ((intensity = *mdl->Offset(iter->x, iter->y - 1, slice)) >= 0.001f && intensity <= 12.001f))
			n++;
		if (iter->y < DATA_HEIGHT - 1 && ((intensity = *mdl->Offset(iter->x, iter->y + 1, slice)) >= 0.0001f && intensity <= 12.001f))
			n++;

		if ((abs(current.x - iter->x) >= SKELETON0_RANGE &&
			abs(current.y - iter->y) >= SKELETON0_RANGE &&
			n == 4) ||
			(abs(current.x - iter->x) >= 3 * SKELETON0_RANGE &&
				abs(current.y - iter->y) >= 3 * SKELETON0_RANGE))
		{
			current = *iter;
			skeleton0_.push_back(iter - points_.begin());
		}
	}
}

void FrontInfo_t::CalculateNear(const std::vector<FrontInfo_t>& data)
{

	SectionsType_t nearSections = 0;
	for (char shift = 0; shift < sizeof(SectionsType_t) * 8; shift++)
	{

		if (sections_ & 1 << shift)
			nearSections |= 1 << shift;
		else continue;

		if (shift >= SECTIONS_X)
			nearSections |= 1 << (shift - SECTIONS_X);

		if (shift < SECTIONS_X * (SECTIONS_Y - 1))
			nearSections |= 1 << (shift + SECTIONS_X);

		if ((shift % SECTIONS_X) > 0)
		{
			nearSections |= 1 << (shift - 1);
			if (shift >= SECTIONS_X)
				nearSections |= 1 << (shift - SECTIONS_X - 1);
			if (shift < SECTIONS_X * (SECTIONS_Y - 1))
				nearSections |= 1 << (shift + SECTIONS_X - 1);
		}

		if ((shift % SECTIONS_X) < SECTIONS_X - 1)
		{
			nearSections |= 1 << (shift + 1);
			if (shift >= SECTIONS_X)
				nearSections |= 1 << (shift - SECTIONS_X + 1);
			if (shift < SECTIONS_X * (SECTIONS_Y - 1))
				nearSections |= 1 << (shift + SECTIONS_X + 1);
		}
	}

	for (uint32_t i = 0; i < data.size(); i++)
	{
		if (data[i].sections_ & nearSections)
			near_.push_back(i);
	}

}

void FrontInfo_t::FindEquivalentFront(const std::vector<FrontInfo_t>& data)
{
	CalculateNear(data);
	equivalentFront_ = -1;
	if (points_.empty()) return;
	if (near_.empty()) return;

	SPOINT_t current;
	uint32_t size = points_.size();
	int32_t notChecked = size;
	bool* checkedPoints = new bool[size];
	for (int i = 0; i < size; i++) checkedPoints[i] = 0;
	notChecked--;

	float* distances = new float[near_.size()];

	std::vector<POINT>* directions = new std::vector<POINT>[near_.size()];

	std::vector<FloatPOINT> directionsSummed(near_.size(), { 0.0f, 0.0f });

	POINT currentDirection = { 0, 0 };

	uint32_t currentPoint = 0;

	std::vector<uint32_t> possibleFronts(near_.size(), 0);

	while (notChecked > 0)
	{
		for (uint32_t i = 0; i < size; i++)
		{
			if (!checkedPoints[i])
			{
				current = points_[i];
				checkedPoints[i] = true;
				notChecked--;
				break;
			}
		}
		for (uint32_t i = 0; i < size; i++)
		{
			if (checkedPoints[i]) continue;
			if (abs(points_[i].x - current.x) <= FRONT_SHIFT &&
				abs(points_[i].y - current.y) <= FRONT_SHIFT)
			{

				checkedPoints[i] = true;
				notChecked--;
			}
		}

		for (int nearFront = 0; nearFront < near_.size(); nearFront++)
		{
			possibleFronts[nearFront] = false;

			distances[nearFront] = 2.0f*FRONT_SHIFT;
			directions[nearFront].push_back({ 2 * FRONT_SHIFT, 2 * FRONT_SHIFT });
			const FrontInfo_t& compareAgainst = data[near_[nearFront]];
			for (int pointIndex = 0; pointIndex < compareAgainst.points_.size(); pointIndex++)
			{
				int16_t dx = compareAgainst.points_[pointIndex].x - current.x;
				int16_t dy = compareAgainst.points_[pointIndex].y - current.y;
				float currentDistance = 0.0f;
				if (abs(dx) <= FRONT_SHIFT && abs(dy) <= FRONT_SHIFT)
				{
					currentDistance = sqrt(dx*dx + dy*dy);
					if (distances[nearFront] > currentDistance)
					{
						distances[nearFront] = currentDistance;
						directions[nearFront][currentPoint] = { dx, dy };
					}
				}

			}
		}
		currentPoint++;
	}

	for (uint32_t nearFront = 0; nearFront < near_.size(); nearFront++)
	{
		for (uint32_t point = 0; point < currentPoint; point++)
		{
			FloatPOINT l = {};
			l = directions[nearFront][point];
			if (l.x > 1.5f*FRONT_SHIFT || l.y > 1.5f*FRONT_SHIFT)
				continue;
			possibleFronts[nearFront]++;
			l.Normalize();
			directionsSummed[nearFront] += l;
		}
		directionsSummed[nearFront].Normalize();
	}

	uint32_t max = possibleFronts[0];
	uint32_t index = 0;

	for (uint32_t nearFront = 1; nearFront < near_.size(); nearFront++)
	{
		if (possibleFronts[nearFront] > max)
		{
			max = possibleFronts[nearFront];
			index = nearFront;
		}
	}

	if (max != 0 && max >= currentPoint * 0.25f) equivalentFront_ = near_[index];
	//printf("Equivalent front %d\n", equivalentFront_);

	delete[] checkedPoints;
	checkedPoints = nullptr;
	delete[] distances;
	distances = nullptr;
	delete[] directions;
	directions = nullptr;
}

SPOINT_t::SPOINT_t() :
	x(),
	y()
{

}

SPOINT_t::SPOINT_t(int x_, int y_) :
	x(x_),
	y(y_)
{}

void NeighbourN_t::clear()
{
	neighbours = n = 0;
}


/*
void FrontAnalyzer::SortFronts()
{
	BEGIN_EXCEPTION_HANDLING

	static bool done[DATA_WIDTH][DATA_HEIGHT] = {};

	for (auto currentFront = fronts_.begin();
		currentFront < fronts_.end();
		currentFront++)
	{
		POINT_N start = {};
		bool foundStart = false;
		const float THRESHOLD = 60.0f * center;
		float k = 1.0f;

		{
			char z = 0;
			for (POINT_N currentPoint : currentFront->points_)
			{

				if (currentPoint.n == 4 &&
					frontPos_[currentPoint.x * DATA_HEIGHT + currentPoint.y] > k * THRESHOLD)
				{
					if (z < 4)
					{
						z++; continue;
					}
					start = currentPoint;
					foundStart = true;
					break;
				}
			}
		}

		if (!foundStart) continue;

		float currentIntensity = frontPos_[start.x * DATA_HEIGHT + start.y];
		POINT currentPoint = { start.x, start.y };

		bool next = true;
		if (currentIntensity < k * THRESHOLD)
			_EXC_N(INVALID_INTENSITY_THRESHOLD, "Invalid intensity found (%d %f)" _ start.n _ currentIntensity)
			float grad = 0.0f;
		uint32_t nextIndex = 0;
		float tempIntensity = 0;
		const char SUM = 10;
		const char LAST_TOP = 1,
			LAST_BOTTOM = SUM - LAST_TOP,
			LAST_LEFT = 2,
			LAST_RIGHT = SUM - LAST_LEFT,
			LAST_LEFT_TOP = 3,
			LAST_RIGHT_BOTTOM = SUM - LAST_LEFT_TOP,
			LAST_RIGHT_TOP = 4,
			LAST_LEFT_BOTTOM = SUM - LAST_RIGHT_TOP;
		char lastPos = 0;
		char newLastPos = 0;

		bool foundFirstEnd = false;
		bool swapDir = false;
		char firstLastPos = 0;
		bool once = false;

		while (true)
		{
			grad = -10000.0f;
			currentFront->skeleton1_.push_back(currentPoint);
			next = false;

#define OPERATION(ifParam, dx, dy, lastPos1) \
if (lastPos != LAST_##lastPos1 && currentPoint. ifParam && !done[currentPoint.x + (dx)][currentPoint.y + (dy)]) \
{ \
	tempIntensity = frontPos_[(currentPoint.x + (dx)) * DATA_HEIGHT + currentPoint.y  + (dy)]; \
	/*printf("Result: %d, %f, %f\n", lastPos - LAST_##lastPos1, tempIntensity, tempIntensity-currentIntensity); *
	if (tempIntensity - currentIntensity > grad && tempIntensity > k * THRESHOLD) \
	{ \
		nextIndex = (currentPoint.x + (dx)) * DATA_HEIGHT + currentPoint.y + (dy); \
		grad = tempIntensity - currentIntensity; \
		next = true; \
		newLastPos = 6 - LAST_##lastPos1; \
		done[currentPoint.x + (dx)][currentPoint.y + (dy)] = true; \
	} \
} 
			/*printf("Before %s\n",
			lastPos == LAST_TOP ? "TOP" : (lastPos == LAST_BOTTOM ? "BOTTOM" : (lastPos == LAST_LEFT ? "LEFT" : "RIGHT")));*
			OPERATION(x > 0, -1, 0, LEFT)
				OPERATION(x < DATA_WIDTH - 1, +1, 0, RIGHT)
				OPERATION(y > 0, 0, -1, TOP)
				OPERATION(y < DATA_HEIGHT - 1, 0, +1, BOTTOM)

				OPERATION(x > 0 && currentPoint.y > 0, -1, -1, LEFT_TOP)
				OPERATION(x > 0 && currentPoint.y < DATA_HEIGHT - 1, -1, +1, LEFT_BOTTOM)
				OPERATION(x < DATA_WIDTH - 1 && currentPoint.y > 0, +1, -1, RIGHT_TOP)
				OPERATION(x < DATA_WIDTH - 1 && currentPoint.y < DATA_HEIGHT - 1, +1, +1, RIGHT_BOTTOM)

				lastPos = newLastPos;
			/*if (next)
			printf("Next %s, grad %f, dir swap %d, %d, %d\n",
			lastPos == LAST_TOP ? "TOP" : (lastPos == LAST_BOTTOM ? "BOTTOM" : (lastPos == LAST_LEFT ? "LEFT" : "RIGHT")),
			grad, swapDir, currentPoint.x, currentPoint.y);
			else
			printf("No next\n");*

			if (!once && !next)
			{
				currentFront->skeleton1_.clear();
				break;
			}
			//_EXC_N (FOUND_ISLE, "Found single pixel, no near pixel passes threshold (%f)" _ currentIntensity)
			else
				if (!once)
				{
					firstLastPos = lastPos;
					once = true;
				}

			if (!next && foundFirstEnd)
			{
				if (k > 0.3f && (start.x - currentPoint.x) * (start.x - currentPoint.x) + (start.y - currentPoint.y)*(start.y - currentPoint.y) < 100.0f)
				{
					k *= 0.9f;
					continue;
				}
				else
				{
					printf("NO NEXT %f\n", currentIntensity);
					break;
				}
			}

			if (!next && !foundFirstEnd)
			{
				next = true;
				lastPos = 6 - firstLastPos;
				size_t size = currentFront->skeleton1_.size();
				for (uint32_t i = 0; i < size / 2; i++)
					std::swap(currentFront->skeleton1_[i],
						currentFront->skeleton1_[size - i - 1]);
				currentPoint = { start.x, start.y };
				currentIntensity = frontPos_[currentPoint.x * DATA_HEIGHT + currentPoint.y];
				foundFirstEnd = true;
			}
			else
			{
				currentPoint = { static_cast<LONG> (nextIndex / DATA_HEIGHT),
					static_cast<LONG> (nextIndex % DATA_HEIGHT) };
				currentIntensity = frontPos_[nextIndex];
				if (k < 0.9f) k *= 1.1f;
			}

#undef OPERATION
		}
}*/

#undef DEBUG_SLICE
#undef DEBUG_X
#undef DEBUG_Y
