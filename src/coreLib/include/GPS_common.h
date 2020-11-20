#pragma once
#include "Common.h"

#define EARTH_RAD 6372795.0
#define ELLIPSOID_A 6378137.0
#define ELLIPSOID_B 6356752.31

namespace gps
{
	struct Coordinate
	{
		double latitude = 0.0;
		double longitude = 0.0;
		double altitude = 0.0;
		TimeMS time = 0;
	};
}