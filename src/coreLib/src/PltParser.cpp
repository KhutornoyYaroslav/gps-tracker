#include "PltParser.h"
#include <fstream>
#include <sstream>
#include <string>
#include "Logging.h"
#include "tools.h"

#define LOG_TAG "PLTParser: "

namespace plt
{
	Parser::Parser()
	{

	}

	Parser::~Parser()
	{

	}

	bool Parser::parse(const char* filename, std::vector<gps::Coordinate>& data)
	{
		std::ifstream fs(filename, std::ifstream::binary);
		if (!fs)
			return false;

		data.clear();

		std::string line;
		int line_offset = 0;
		while (std::getline(fs, line))
		{
			if (++line_offset <= 6)
				continue;

			std::istringstream iss(line);
			
			gps::Coordinate item;
			char comma;
			double dummy;
			std::string time; /// 2008-10-24,02:09:59
				
			if (!(iss >> item.latitude >> comma >> item.longitude >> comma >> 
				dummy >> comma >> item.altitude >> comma >> dummy >> comma >> time))
			{
				LOG_ERROR(LOG_TAG "failed to parse line: {0}", line);
				return false;
			}

			if (abs(item.altitude - 777.0) <= DBL_EPSILON)
			{
				item.altitude = 0.0;
			}

			auto time_ = tools::replaceAll(time, ",", " ") + ".000000";
			if (!tools::str_datetime_to_timems(time_.c_str(), item.time))
			{
				LOG_ERROR(LOG_TAG "failed to parse time: {0}", time);
				return false;
			}

			data.push_back(item);			
		}

		fs.close();
		return true;
	}
}