#pragma once
#include "Common.h"
#include "GPS_common.h"
#include <vector>

namespace plt
{
	class Parser
	{
	public:
		Parser();
		~Parser();

		static bool parse(const char* filename, std::vector<gps::Coordinate>& data);
	};
}