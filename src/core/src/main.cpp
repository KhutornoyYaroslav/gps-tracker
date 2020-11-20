#include <iostream>
#include <opencv2/core.hpp>
#include <Logging.h>
#include <PltParser.h>
#include <GPS_algorithms.h>
#include <GPS_viewer.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#define LOG_TAG "Core: "

int main(int argc, char* argv[])
{
	logging::initLogger("./logs/core.log");
	struct logger_deinit { ~logger_deinit() { logging::shutdown(); } } logger_deinit_obj;

	const char* gps_test_filename = "./data/data.plt";
	std::vector<gps::Coordinate> data;
	if (!plt::Parser::parse(gps_test_filename, data))
	{
		LOG_ERROR(LOG_TAG "Can't parse {0}", gps_test_filename);
		return -1;
	}

	gps::debug::Viewer gps_viewer;

	// data
	if (!gps_viewer.dynamic_view(data, {800,800}, 10))
	{
		LOG_ERROR(LOG_TAG "Can't view data");
		return -1;
	}

	//for (const auto& gps : data)
	//{
	//	LOG_INFO(LOG_TAG "lat {0}, lon {1}, alt {2}, time {3}", 
	//		gps.latitude,
	//		gps.longitude,
	//		gps.altitude,
	//		gps.time);
	//}

	std::cin.get();
	return 0;

	// ************************

	double min_x = std::numeric_limits<double>::max();
	double min_y = std::numeric_limits<double>::max();
	double min_z = std::numeric_limits<double>::max();

	std::vector<cv::Point3d> world_data;
	world_data.reserve(data.size());

	for (const auto& gps : data)
	{
		const auto world = gps::algorithms::gps2world(gps);
		world_data.push_back(world);

		min_x = std::min(min_x, world.x);
		min_y = std::min(min_y, world.y);
		min_z = std::min(min_z, world.z);

		//LOG_INFO(LOG_TAG "lat {0}, lon {1}, alt {2}, time {3}", 
		//	gps.latitude,
		//	gps.longitude,
		//	gps.altitude,
		//	gps.time);
	}

	cv::Mat main_frame = cv::Mat(800, 800, CV_8UC1, cv::Scalar::all(0));

	for (auto& world : world_data)
	{
		world.x -= min_x;
		world.y -= min_y;
		world.z -= min_z;

		//LOG_INFO(LOG_TAG "gps2world = {0:.2f}m, {1:.2f}m, {2:.2f}m", world.x,world.y, world.z);
	}

	//for (size_t i = 0; i < world_data.size() - 2; ++i)
	//{
	//	const auto& coord_prev = world_data[i];
	//	const auto& coord_curr = world_data[i + 1];

	//	auto fr_c1 = cv::Point(coord_prev.x, coord_prev.y);
	//	auto fr_c2 = cv::Point(coord_curr.x, coord_curr.y);

	//	cv::circle(main_frame, fr_c1, 2, cv::Scalar(255), 1);
	//	cv::circle(main_frame, fr_c2, 2, cv::Scalar(255), 1);

	//	cv::line(main_frame, fr_c1, fr_c2, cv::Scalar(255), 1);
	//}

	//cv::imshow("Debug", main_frame);
	//cv::waitKey(0);

	for (size_t i = 0; i < data.size() - 2; ++i)
	{
		const auto& coord_prev = data[i];
		const auto& coord_curr = data[i + 1];

		const auto delta = gps::algorithms::calcGPSdelta(coord_prev, coord_curr);

		const auto world_prev = gps::algorithms::gps2world(coord_prev);
		const auto world_curr = gps::algorithms::gps2world(coord_curr);

		LOG_INFO(LOG_TAG "delta {0:.2f}m / {1:.2f}m [dAlt = {2:.3f}m]", 
			delta, 
			cv::norm(world_prev - world_curr),
			abs(world_prev.z - world_curr.z));
	}

	std::cin.get();
	return 0;
}
