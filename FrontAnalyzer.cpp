
#include "Builder.h"


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
	fopen_s(&f, "Front.data", "wb");

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
	printf("%d\n", fronts_.size());
	while (!GetAsyncKeyState(VK_SPACE));
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

	if (SECTIONS_X * SECTIONS_Y > 8 * sizeof (FrontInfo_t::sections_))
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

	current.AddPoint(x, y);

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
	near_ = 0;

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
		near_ += (data[i].sections_ & nearSections) ? 1 : 0;

	while (!GetAsyncKeyState(VK_SPACE));
	while (GetAsyncKeyState(VK_SPACE));


}
