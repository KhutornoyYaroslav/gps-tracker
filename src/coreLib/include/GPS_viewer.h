#pragma once
#include "GPS_common.h"
#include <vector>
#include <opencv2/core.hpp>

namespace gps::debug
{
	class Viewer
	{
	public:
		Viewer();
		~Viewer();

		bool view(const std::vector<Coordinate>& data, cv::Size wnd_size = {800, 800}, int delay_ms = 0);
		bool dynamic_view(const std::vector<Coordinate>& data, cv::Size wnd_size = { 800, 800 }, int delay_ms = 250);

	private:
		int m_padding = 25;
	};
}