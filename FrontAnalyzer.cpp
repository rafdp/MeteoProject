
#include "Builder.h"
#include <complex>


FrontAnalyzer::FrontAnalyzer (MeteoDataLoader* mdl, int slice)
try :
	fronts_(),
	set_   (),
	mdl_   (mdl),
	slice_ (slice)
{
	ZeroMemory(set_, DATA_WIDTH * DATA_HEIGHT);
	if (!mdl_)
		_EXC_N(NULL_THIS, "Null MeteoDataLoader ptr");
	FrontInfo_t current;
	FILE* f = nullptr;
	fopen_s(&f, "_Front.data", "wb");

	for (int x = 0; x < DATA_WIDTH; x++)
	{
		for (int y = 0; y < DATA_HEIGHT; y++)
		{
			if (set_[x][y]) continue;
			RecursiveFrontFinder (x, y, current);
			if (!current.empty ())
				fronts_.push_back (current);
			current.clear ();
		}
	}
	int allSize = fronts_.size ();

	fwrite(&DATA_WIDTH, sizeof(int), 1, f);
	fwrite(&DATA_HEIGHT, sizeof(int), 1, f);

	fwrite(&SECTIONS_X, sizeof(int), 1, f);
	fwrite(&SECTIONS_Y, sizeof(int), 1, f);

	fwrite(&allSize, sizeof(int), 1, f);

	for (auto currentFront : fronts_)
	{
		currentFront.CalculateNear(fronts_);
		int size = currentFront.size();
		fwrite (&size, sizeof (int), 1, f);
		fwrite(&currentFront.sections_, sizeof(SectionsType_t), 1, f);
		fwrite(currentFront.data(), sizeof(POINT), size, f);
	}
	printf("%llu\n", fronts_.size());
	//while (!GetAsyncKeyState(VK_SPACE));
	fclose(f);
}
_END_EXCEPTION_HANDLING(CTOR)

FrontAnalyzer::~FrontAnalyzer()
{
	fronts_.clear();
	mdl_ = nullptr;
	slice_ = -1;
}

void FrontAnalyzer::ok()
{
	DEFAULT_OK_BLOCK
	if (slice_ == -1)
		_EXC_N(NEGATIVE_SLICE, "Negative slice");

	if (SECTIONS_X * SECTIONS_Y > 8 * sizeof (SectionsType_t))
		_EXC_N(NOT_ENOUGH_SECTIONS, "Not enough section bits");

}

void FrontAnalyzer::RecursiveFrontFinder(int x, int y, FrontInfo_t& current)
{
	const int RANGE = 2;

	if (set_[x][y]) return;
	float intensity = *mdl_->Offset(x, y, slice_);

	if (intensity < 0.0001f || intensity > 12.001f)
	{
		set_[x][y] = 1;
		return;
	}

	set_[x][y] = 2;

	char n = 0;
	if (x >= 1 && ((intensity = *mdl_->Offset(x - 1, y, slice_)) >= 0.0001f && intensity <= 12.001f))
		n++;
	if (x < DATA_WIDTH - 1 && ((intensity = *mdl_->Offset(x + 1, y, slice_)) >= 0.0001f && intensity <= 12.001f))
		n++;
	if (y >= 1 && ((intensity = *mdl_->Offset(x, y - 1, slice_)) >= 0.0001f && intensity <= 12.001f))
		n++;
	if (y < DATA_HEIGHT - 1 && ((intensity = *mdl_->Offset(x, y + 1, slice_)) >= 0.0001f && intensity <= 12.001f))
		n++;
	if (n == 4) current.AddPoint(x, y);

	for (int x_ = -RANGE; x_ <= RANGE; x_++)
	{
		for (int y_ = -RANGE; y_ <= RANGE; y_++)
		{
			if (!x_ && !y_) continue;
			if (x + x_ < DATA_WIDTH && 
				x + x_ >= 0 && 
				y + y_ < DATA_HEIGHT &&
				y + y_ >= 0 &&
				!set_[x + x_][y + y_])
				RecursiveFrontFinder(x + x_, y + y_, current);
		}
	}
}


void FloatPOINT::Normalize()
{
	float l = sqrt (x*x + y*y);

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
	return points_.size ();
}

POINT * FrontInfo_t::data()
{
	if (empty()) return nullptr;
	return points_.data ();
}

void FrontInfo_t::AddPoint(int x, int y)
{
	unsigned char bit = int (x*SCALING_X) + 8 * int (y*SCALING_Y);
	sections_ |= 1 << bit;
	points_.push_back({ x, y });
}

void FrontInfo_t::CalculateNear(const std::vector<FrontInfo_t>& data)
{

	SectionsType_t nearSections = 0;
	for (char shift = 0; shift < sizeof(SectionsType_t) * 8; shift++)
	{
		if (sections_ & 1 << shift)
			nearSections |= 1 << shift;

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
		if (data[i].sections_ & nearSections) near_.push_back(i);
	}

}

void FrontInfo_t::FindEquivalentFront(const std::vector<FrontInfo_t>& data)
{
	CalculateNear (data);
	equivalentFront_ = -1;
	if (points_.empty()) return;
	if (near_.empty()) return;

	POINT current = {};
	uint32_t size = points_.size();
	uint32_t notChecked = size;
	bool* checkedPoints = new bool[size];
	for (int i = 0; i < size; i++) checkedPoints[i] = 0;
	notChecked--;

	float* distances = new float[near_.size()];

	std::vector<POINT>* directions = new std::vector<POINT> [near_.size()];

	std::vector<FloatPOINT> directionsSummed;

	POINT currentDirection = { 0, 0 };

	uint32_t currentPoint = 0;

	bool* possibleFronts = new bool[near_.size()];

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
			directions[nearFront][currentPoint] = { 2.0f*FRONT_SHIFT, 2.0f*FRONT_SHIFT };
			const FrontInfo_t& compareAgainst = data[near_[nearFront]];
			for (int pointIndex = 0; pointIndex < compareAgainst.points_.size(); pointIndex++)
			{
				int16_t dx = compareAgainst.points_[pointIndex].x - current.x;
				int16_t dy = compareAgainst.points_[pointIndex].y - current.y;
				float currentDistance = 0.0f;
				if (abs (dx) <= FRONT_SHIFT && abs(dy) <= FRONT_SHIFT)
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
			possibleFronts[nearFront] = true;
			l.Normalize();
			directionsSummed[nearFront] += l;
		}
		directionsSummed[nearFront].Normalize();
	}

	//! NEXT -- CHECK WHICH IS THE BEST MATCH


	delete[] checkedPoints;
	checkedPoints = nullptr;
	delete[] distances;
	distances = nullptr;
	delete[] directions;
	directions = nullptr;
	delete[] possibleFronts;
	possibleFronts = nullptr;
}

