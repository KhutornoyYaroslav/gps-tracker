#include "GPS_viewer.h"
#include "GPS_algorithms.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>

#include <Logging.h>
#define LOG_TAG "GpsViewer: "

namespace gps::debug
{
	Viewer::Viewer()
	{

	}

	Viewer::~Viewer()
	{

	}

	bool Viewer::view(const std::vector<Coordinate>& data, cv::Size wnd_size, int delay_ms)
	{
		if (data.empty() || wnd_size.empty())
			return false;

		/// Create window
		const char* wnd_name = "Gps viewer";
		cv::namedWindow(wnd_name);

		/// Create canvas
		cv::Mat canvas = cv::Mat(wnd_size, CV_8UC3, cv::Scalar::all(0));

		/// Convert coordinates to planar
		std::vector<cv::Point2d> view_data;
		view_data.reserve(data.size());

		for (const auto& c : data)
		{
			const auto p = gps::algorithms::gps2world(c);
			view_data.push_back(cv::Point2d(p.x, p.y));
		}

		/// Normalize coordinates
		const auto x_min = std::min_element(view_data.begin(), view_data.end(), [](const cv::Point2d& lhs, const cv::Point2d& rhs) { return lhs.x < rhs.x; })->x;
		const auto y_max_ = std::max_element(view_data.begin(), view_data.end(), [](const cv::Point2d& lhs, const cv::Point2d& rhs) { return lhs.y < rhs.y; })->y;
		std::transform(view_data.begin(), view_data.end(), view_data.begin(), [&](cv::Point2d& p) { return cv::Point2d((p.x - x_min), (y_max_ - p.y)); });

		/// Aspect ratio, padding

		// todo: make single aspect ratio ! not x and y

		const auto x_max = std::max_element(view_data.begin(), view_data.end(), [](const cv::Point2d& lhs, const cv::Point2d& rhs) { return lhs.x < rhs.x; })->x;
		const auto y_max = std::max_element(view_data.begin(), view_data.end(), [](const cv::Point2d& lhs, const cv::Point2d& rhs) { return lhs.y < rhs.y; })->y;

		cv::Size work_canvas_size(wnd_size.width - 2 * m_padding, wnd_size.height - 2 * m_padding);
		cv::Point2d aspect((double)work_canvas_size.width / x_max, (double)work_canvas_size.height / y_max);  // TODO: deviding by zero

		std::transform(view_data.begin(), view_data.end(), view_data.begin(), 
			[&](cv::Point2d& p) { return cv::Point2d(m_padding + (p.x * aspect.x), m_padding + (p.y * aspect.y)); });

		/// Draw grid
		int grid_x_step = work_canvas_size.width / 10.0;
		int grid_y_step = work_canvas_size.height / 10.0;

		for (int x = m_padding; x < wnd_size.width; x += grid_x_step)
			cv::line(canvas, cv::Point(x, 0), cv::Point(x, canvas.rows), cv::Scalar::all(100));

		for (int y = m_padding; y < wnd_size.height; y += grid_y_step)
			cv::line(canvas, cv::Point(0, y), cv::Point(canvas.cols, y), cv::Scalar::all(100));

		cv::putText(canvas, std::to_string(grid_x_step / aspect.x) + "m (X-axis)", { m_padding, m_padding }, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255));
		cv::putText(canvas, std::to_string(grid_y_step / aspect.y) + "m (Y-axis)", { m_padding, 10 + 2 * m_padding }, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255));

		/// Draw data
		for (size_t i = 0; i < view_data.size() - 1; ++i)
		{
			const auto& coord_prev = view_data[i];
			const auto& coord_curr = view_data[i + 1];

			auto fr_c1 = cv::Point(coord_prev.x, coord_prev.y);
			auto fr_c2 = cv::Point(coord_curr.x, coord_curr.y);

			cv::circle(canvas, fr_c1, 2, cv::Scalar(255), 2);
			cv::circle(canvas, fr_c2, 2, cv::Scalar(255), 2);

			cv::line(canvas, fr_c1, fr_c2, cv::Scalar(255), 1);

			//LOG_INFO(LOG_TAG "data: {0:.2f}m, {1:.2f}m", p.x, p.y);
		}

		cv::circle(canvas, view_data[view_data.size() - 1], 3, cv::Scalar(0, 0, 255), 3);

		/// Show frame
		cv::imshow(wnd_name, canvas);
		cv::waitKey(delay_ms);

		/// Destroy window
		//cv::destroyWindow(wnd_name);

		return true;
	}

	bool Viewer::dynamic_view(const std::vector<Coordinate>& data, cv::Size wnd_size, int delay_ms)
	{
		if (data.empty() || wnd_size.empty())
			return false;

		std::vector<Coordinate> view_data;
		view_data.reserve(data.size());

		for (const auto& d : data)
		{
			view_data.push_back(d);
			if (!view(view_data, wnd_size, delay_ms))
				return false;
		}

		return true;
	}
}