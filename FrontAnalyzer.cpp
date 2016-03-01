
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
	std::vector<IntPair_t> current;
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

	fwrite(&allSize, sizeof(int), 1, f);

	for (auto currentFront : fronts_)
	{
		int size = currentFront.size();
		fwrite (&size, sizeof (int), 1, f);
		fwrite (currentFront.data (), sizeof(IntPair_t), size, f);
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
}

void FrontAnalyzer::RecursiveFrontFinder(int x, int y, std::vector<IntPair_t>& current)
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

	current.push_back({ x, y });

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