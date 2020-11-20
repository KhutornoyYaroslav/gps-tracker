#include "GPS_algorithms.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace gps::algorithms
{
	// TODO: normalize coordinates (-180.0, 180.0, etc)
	cv::Point3d gps2world(Coordinate c)
	{
		cv::Point3d res;

		const double lamda = c.longitude * (M_PI / 180.0);
		const double phi = c.latitude * (M_PI / 180.0);
		const double h = c.altitude;

		const auto k = 1 - (ELLIPSOID_B * ELLIPSOID_B) / (ELLIPSOID_A * ELLIPSOID_A);

		res.x = (ELLIPSOID_A / sqrt(1 - k * sin(phi)*sin(phi)) + h) * cos(phi) * cos(lamda);
		res.y = (ELLIPSOID_A / sqrt(1 - k * sin(phi)*sin(phi)) + h) * cos(phi) * sin(lamda);
		res.z = (ELLIPSOID_A * (1 - k) / sqrt(1 - k * sin(phi)*sin(phi)) + h) * sin(phi);

		return res;
	}

	// TODO: normalize coordinates (-180.0, 180.0, etc)
	double calcGPSdelta(Coordinate c1, Coordinate c2)
	{
		/// https://gis-lab.info/qa/great-circles.html

		auto lat1 = c1.latitude * (M_PI / 180.0);
		auto lat2 = c2.latitude * (M_PI / 180.0);
		auto lon1 = c1.longitude * (M_PI / 180.0);
		auto lon2 = c2.longitude * (M_PI / 180.0);

		const auto cl1 = cos(lat1);
		const auto cl2 = cos(lat2);
		const auto sl1 = sin(lat1);
		const auto sl2 = sin(lat2);
		const auto delta = lon2 - lon1;
		const auto cdelta = cos(delta);
		const auto sdelta = sin(delta);

		auto y = sqrt(pow(cl2*sdelta, 2) + pow(cl1*sl2 - sl1 * cl2*cdelta, 2));
		auto x = sl1 * sl2 + cl1 * cl2*cdelta;
		const auto ad = atan2(y, x);
		const auto dist = ad * EARTH_RAD;

		return dist;
	}

	void normalize(std::vector<Coordinate>& data)
	{
		// TODO:
	}
}