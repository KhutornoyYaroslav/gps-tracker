#pragma once
#include "GPS_common.h"
#include <opencv2/core.hpp>
#include <vector>

namespace gps::algorithms
{
	extern cv::Point3d gps2world(Coordinate c);
	extern double calcGPSdelta(Coordinate c1, Coordinate c2);
	extern void normalize(std::vector<Coordinate>& data);
}