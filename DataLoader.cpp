
#include "Builder.h"
#include <xnamath.h>
#include <complex>


MeteoDataLoader::MeteoDataLoader (std::string cosmomesh,
								  std::string fronts,
								  std::string height,
								  Direct3DProcessor* proc) 
try : 
	data_         (),
	frontColors_  (new float [DATA_WIDTH*DATA_HEIGHT*SLICES*HOURS])
{
	FILE* currentFile = nullptr;

	fopen_s (&currentFile, cosmomesh.c_str (), "rb");
	if (!currentFile)
		_EXC_N (NULL_THIS, "Failed to open file \"%s\"" _ cosmomesh.c_str ());

	fread (&data_.latitude (0, 0), sizeof (float), DATA_WIDTH * DATA_HEIGHT, currentFile);
	fread (&data_.longitude (0, 0), sizeof (float), DATA_WIDTH * DATA_HEIGHT, currentFile);
	fread (&data_.ground (0, 0), sizeof (float), DATA_WIDTH * DATA_HEIGHT, currentFile);

	fclose (currentFile);

	fopen_s (&currentFile, fronts.c_str (), "rb");
	if (!currentFile)
		_EXC_N (NULL_THIS, "Failed to open file \"%s\"" _ fronts.c_str ());

	fread (&data_.front (0, 0, 0, 0), sizeof (float), DATA_WIDTH * DATA_HEIGHT * SLICES * HOURS, currentFile);
		
	fclose (currentFile);

	fopen_s (&currentFile, height.c_str (), "rb");
	if (!currentFile)
		_EXC_N (NULL_THIS, "Failed to open file \"%s\"" _ height.c_str ());

	fread (&data_.height(0, 0, 0, 0), sizeof (float), DATA_WIDTH * DATA_HEIGHT * SLICES * HOURS, currentFile);

	fclose (currentFile);

	//CreateAll (proc);
}
_END_EXCEPTION_HANDLING (CTOR)

void MeteoDataLoader::Float2Color() const
{
	static bool once = false;
	if (once) return;
	once = true;
	float color = 0.0f;
	for (int hour = 0; hour < HOURS; hour++)
	{
		for (int slice = 0; slice < SLICES; slice++)
		{
			for (int x = 0; x < DATA_WIDTH; x++)
			{
				for (int y = 0; y < DATA_HEIGHT; y++)
				{
					color = data_.front(x, y, slice, hour) / 12.0f;
					
					if (color > 2.0f) color = 0.0f;
					if (color > 1.0f) color = 1.0f;

					frontColors_[hour * (SLICES * DATA_HEIGHT * DATA_WIDTH) +
						slice * (DATA_HEIGHT * DATA_WIDTH) +
						y * DATA_WIDTH +
						x] = color;
				}
			}
		}
	}
}

float* MeteoDataLoader::Offset(short x, short y, short slice, char hour) const
{
	return frontColors_ + hour * (SLICES * DATA_HEIGHT * DATA_WIDTH) +
		slice * (DATA_HEIGHT * DATA_WIDTH) +
		y * DATA_WIDTH +
		x;
}

void MeteoDataLoader::ok ()
{
	DEFAULT_OK_BLOCK
}


MeteoDataLoader::~MeteoDataLoader ()
{
	delete[] frontColors_;
}


MeteoData_t::MeteoData_t () 
try :
	latitude_  (new float [DATA_WIDTH*DATA_HEIGHT]),
	longitude_ (new float [DATA_WIDTH*DATA_HEIGHT]),
	ground_    (new float [DATA_WIDTH*DATA_HEIGHT]),
	fronts_    (new float [DATA_WIDTH*DATA_HEIGHT*SLICES*HOURS]),
	height_    (new float [DATA_WIDTH*DATA_HEIGHT*SLICES*HOURS])
{

}
_END_EXCEPTION_HANDLING (CTOR)

MeteoData_t::~MeteoData_t ()
{
	delete [] latitude_;
	latitude_ = nullptr;

	delete[] longitude_;
	longitude_ = nullptr;

	delete[] ground_;
	ground_ = nullptr;

	delete [] fronts_;
	fronts_ = nullptr;

	delete[] height_;
	height_ = nullptr;
}

float & MeteoData_t::latitude (short x, short y) const
{
	//printf ("%d %d\n", y, x);
	return latitude_[y*DATA_WIDTH + x];
}

float & MeteoData_t::longitude (short x, short y) const
{
	return longitude_[y*DATA_WIDTH+ x];
}

float & MeteoData_t::ground (short x, short y) const
{
	return ground_[y*DATA_WIDTH + x];
}

float & MeteoData_t::front (short x, short y, short slice, char hour) const
{
	return fronts_[hour * (SLICES * DATA_HEIGHT * DATA_WIDTH) + slice * (DATA_HEIGHT * DATA_WIDTH) + y * DATA_WIDTH + x];
}

float & MeteoData_t::height (short x, short y, short slice, char hour) const
{
	return height_[hour * (SLICES * DATA_HEIGHT * DATA_WIDTH) + slice * (DATA_HEIGHT * DATA_WIDTH) + y * DATA_WIDTH + x];
}