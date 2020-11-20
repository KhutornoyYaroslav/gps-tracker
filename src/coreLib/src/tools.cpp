#include <tools.h>

#ifdef linux
#define sscanf_s sscanf
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>  /* Mutex, Process */
#include "psapi.h"  /* Process status API (PSAPI) for process stat */
//#include "TCHAR.h"
#include "pdh.h" /* Performance Data Helper library (PDH) for System Mem/CPU Stat */
#include "pdhmsg.h" /* PDH defines/messsages */
#else
#include <fcntl.h>  /* fcntl, open */
#include <sys/types.h>
#include <sys/sysinfo.h>  /* sysinfo memory */
#include <unistd.h>
#include <cerrno> /*errno*/
#include <strings.h>
#include <time.h>
#endif

#include <locale>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>

#include <QString>
#include <unordered_map>

#include <Filesystem.h>

namespace tools {

	uint64_t getPerfCntrMs(bool refreshFreq)
	{
		namespace ch = std::chrono;
		return ch::duration_cast<ch::milliseconds>(ch::steady_clock::now().time_since_epoch()).count();
	}

	int64_t local_timezone_offset()
	{
		std::time_t gmt, rawtime = time(NULL);
		std::tm ttm;
		std::tm* ptm;
#if defined(_WIN32)
		ptm = &ttm;
		gmtime_s(ptm, &rawtime);
#else
		std::tm gbuf;
		ptm = gmtime_r(&rawtime, &gbuf);
#endif
		ptm->tm_isdst = -1;  // Request that mktime() looksup dst in timezone database
		gmt = mktime(ptm);

		return (int64_t)std::difftime(rawtime, gmt);
	}

	bool str_datetime_to_timems(const char *str, uint64_t &timems, bool toUTC)
	{
		int yr, mon, day, hr, min, sec, ms;
		timems = 0;

		// Parse
		if (sscanf_s(str, "%4d-%2d-%2d %2d:%2d:%2d.%3d", &yr, &mon, &day, &hr, &min, &sec, &ms) != 7
			|| (yr < 1900) || (yr >= 3000)
			|| (mon < 1) || (mon > 12)
			|| (day < 1) || (day > 31)
			|| (hr < 0) || (hr > 23)
			|| (min < 0) || (min > 59)
			|| (sec < 0) || (sec > 59) // todo: mysql leap sec=60?
			|| (ms < 0)
			)
		{
			return false;
		}

		// Convert
		std::tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		tm_time.tm_year = yr - 1900;
		tm_time.tm_mon = mon - 1;
		tm_time.tm_mday = day;
		tm_time.tm_hour = hr;
		tm_time.tm_min = min;
		tm_time.tm_sec = sec;
		std::time_t gmt = mktime(&tm_time);  // XXX: warn! converting uses time as localtime with timezone offset, time_t will be utc time!
		//printf("tm_time.tm_isdst=%d\r\n", tm_time.tm_isdst);
		if (gmt == -1)
		{
			return false;
		}
		timems = gmt;  // TODO: time 32/64 bit stored in windows/linux? need test in linux!
		if (!toUTC)
		{
			// Revert back local time offset
			timems += tools::local_timezone_offset();
		}
		timems *= 1000;
		timems += ms;

		return true;
	}

	bool str_verifydate_to_timems(const char *str, uint64_t &timems, bool toUTC)
	{
		int yr, mon, day;
		timems = 0;

		/// Parse string
		if (sscanf_s(str, "%2d.%2d.%4d", &day, &mon, &yr) != 3
			|| (yr < 1900) || (yr >= 3000)
			|| (mon < 1) || (mon > 12)
			|| (day < 1) || (day > 31))
		{
			return false;
		}

		/// Convert
		std::tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		tm_time.tm_year = yr - 1900;
		tm_time.tm_mon = mon - 1;
		tm_time.tm_mday = day;
		tm_time.tm_hour = 0;
		tm_time.tm_min = 0;
		tm_time.tm_sec = 0;
		std::time_t gmt = mktime(&tm_time);  // XXX: warn! converting uses time as localtime with timezone offset, time_t will be utc time!
		//printf("tm_time.tm_isdst=%d\r\n", tm_time.tm_isdst);
		if (gmt == -1)
		{
			return false;
		}
		timems = gmt;  // TODO: time 32/64 bit stored in windows/linux? need test in linux!
		if (!toUTC)
		{
			// Revert back local time offset
			timems += tools::local_timezone_offset();
		}
		timems *= 1000;

		return true;
	}

	bool str_datetime_to_time(const char *str, time_t &timet, bool toUTC)
	{
		//int yr, mon, day, hr, min, sec, ms;
		timet = 0;

		// Parse
		/*
		if (sscanf_s(str, "%4d-%2d-%2d %2d:%2d:%2d", &yr, &mon, &day, &hr, &min, &sec, &ms) != 7
			|| (yr < 1900) || (yr >= 3000)
			|| (mon < 1) || (mon > 12)
			|| (day < 1) || (day > 31)
			|| (hr < 0) || (hr > 23)
			|| (min < 0) || (min > 59)
			|| (sec < 0) || (sec > 59) // todo: mysql leap sec=60?
			|| (ms < 0)
			)
		{
			return false;
		}
		*/
		std::tm tm_time{ 0 };
		std::istringstream ss(str);
		//ss.imbue(std::locale("en_US.utf-8"));
		ss >> std::get_time(&tm_time, "%Y-%m-%d %H:%M:%S");

		// Convert
		/*
		std::tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		tm_time.tm_year = yr - 1900;
		tm_time.tm_mon = mon - 1;
		tm_time.tm_mday = day;
		tm_time.tm_hour = hr;
		tm_time.tm_min = min;
		tm_time.tm_sec = sec;
		*/
		timet = mktime(&tm_time);  // XXX: warn! converting uses time as localtime with timezone offset, time_t will be utc time!
		if (timet == -1)
		{
			timet = 0;
			return false;
		}
		if (!toUTC)
		{
			// Revert back local time offset
			timet += tools::local_timezone_offset();
		}

		return true;
	}

	bool str_datetime_iso_to_time(const char *str, time_t &timet, int64_t &offset_h, bool toUTC)
	{
		int yr = 0, mon = 0, day = 0,
			hr = 0, min = 0, sec = 0,
			tzh = 0,
			n = 0;
		const unsigned int min_datetime_len = 19;

		timet = 0;
		offset_h = 0;

		// Parse
		auto ret = sscanf_s(str, "%4d-%2d-%2dT%2d:%2d:%2d%n%3d", &yr, &mon, &day, &hr, &min, &sec, &n, &tzh);

		if ((ret == 7 || ret == 6) &&
			(n == min_datetime_len))
		{
			if ((yr < 1900) || (yr >= 3000)
				|| (mon < 1) || (mon > 12)
				|| (day < 1) || (day > 31)
				|| (hr < 0) || (hr > 23)
				|| (min < 0) || (min > 59)
				|| (sec < 0) || (sec > 59))
			{
				return false;
			}

			// Check UTC format (no TZ)
			if (ret == 6)
			{
				if (strlen(str) > min_datetime_len)
				{
					if (str[19] == 'Z')
					{
						// Format UTC: YYYY-MM-DDThh:mm:ssZ
						tzh = 0;
					}
					else
					{
						return false;
					}
				}
				else
				{
					tzh = 0;
				}
			}
		}
		else
		{
			return false;
		}

		// Convert
		std::tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		tm_time.tm_year = yr - 1900;
		tm_time.tm_mon = mon - 1;
		tm_time.tm_mday = day;
		tm_time.tm_hour = hr;
		tm_time.tm_min = min;
		tm_time.tm_sec = sec;

		timet = mktime(&tm_time);  // XXX: warn! converting uses time as localtime with timezone offset, time_t will be utc time!
		if (timet == -1)
		{
			timet = 0;
			return false;
		}
		if (!toUTC)
		{
			// Revert back local time offset
			timet += tools::local_timezone_offset();
		}

		offset_h = tzh;

		return true;
	}

	int timems_to_str_datetime(uint64_t timems, char* buffer, size_t bufferSize, char dateDelim)
	{
		int offset = 0, tmp;

		std::tm tm_struct{};
		const std::time_t timeSec = timems / 1000;
#if defined(_WIN32)
		gmtime_s(&tm_struct, &timeSec);
#else
		std::tm* tm_ret;
		tm_ret = gmtime_r(&timeSec, &tm_struct);
#endif

		char formatBuffer[32] = "%Y-%m-%d %H:%M:%S.";
		formatBuffer[2] = dateDelim;
		formatBuffer[5] = dateDelim;

		tmp = (int)strftime(buffer, bufferSize, formatBuffer, &tm_struct);
		if (tmp == 0) return 0;
		offset += tmp;
		const int part_ms = int(timems % 1000);
		tmp = snprintf(buffer + offset, bufferSize - offset, "%03d", part_ms);
		if (tmp <= 0) return 0;
		offset += tmp;

		return tmp;
	}

	std::string timems_to_str_datetime(uint64_t timems)
	{
		char buffer[32];
		timems_to_str_datetime(timems, buffer, sizeof(buffer));
		return std::string(buffer);
	}

	int timems_to_str_date(uint64_t timems, char* buffer, size_t bufferSize, char dateDelim)
	{
		std::tm tm_struct{};
		const std::time_t timeSec = timems / 1000;
#if defined(_WIN32)
		gmtime_s(&tm_struct, &timeSec);
#else
		std::tm* tm_ret;
		tm_ret = gmtime_r(&timeSec, &tm_struct);
#endif

		char formatBuffer[16] = "%Y-%m-%d";
		formatBuffer[2] = dateDelim;
		formatBuffer[5] = dateDelim;

		return (int)strftime(buffer, bufferSize, formatBuffer, &tm_struct);
	}

	std::string timems_to_str_date(uint64_t timems)
	{
		char buffer[32];
		timems_to_str_date(timems, buffer, sizeof(buffer));
		return std::string(buffer);
	}

	std::string time_to_str_datetime(std::time_t timet)
	{
		std::tm tm_struct;
		char buf[300];
		{
#if defined(_WIN32)
			gmtime_s(&tm_struct, &timet);
#else
			std::tm* tm_ret;
			tm_ret = gmtime_r(&timet, &tm_struct);
#endif
			strftime(buf, 30, "%Y-%m-%d %H:%M:%S", &tm_struct);
		}
		return std::string(buf);
	}

	int timems_to_str_datetime_iso(uint64_t timems, char* buffer, size_t bufferSize, int64_t offset_h)
	{
		size_t len = 0, tmp = 0;

		std::tm tm_struct{};
		const std::time_t timeSec = timems / 1000;
#if defined(_WIN32)
		gmtime_s(&tm_struct, &timeSec);
#else
		std::tm* tm_ret;
		tm_ret = gmtime_r(&timeSec, &tm_struct);
#endif

		tmp = strftime(buffer, bufferSize, "%Y-%m-%dT%H:%M:%S", &tm_struct);
		if (tmp == 0) return 0;

		len += tmp;
		tmp = snprintf(buffer + len, bufferSize - len, (offset_h < 0) ? "%03lld" : "+%02lld", (long long)offset_h);
		if (tmp <= 0) return 0;

		len += tmp;
		return (int)len;
	}

	std::string timems_to_str_datetime_iso(uint64_t timems, int64_t offset_h)
	{
		char buffer[32];
		timems_to_str_datetime_iso(timems, buffer, sizeof(buffer), offset_h);
		return std::string(buffer);
	}

	int offsetms_to_str_iso(int64_t offsetms, char* buffer, size_t bufferSize)
	{
		const int64_t hh = offsetms / 3600000ll;
		const int64_t mm = (abs(offsetms) % 3600000ll) / 60000ll;
		return snprintf(buffer, bufferSize, (hh < 0) ? "%03lld:%02lld" : "+%02lld:%02lld", (long long)hh, (long long)mm);
	}

	std::string offsetms_to_str_iso(int64_t offsetms)
	{
		char buffer[32];
		offsetms_to_str_iso(offsetms, buffer, sizeof(buffer));
		return std::string(buffer);
	}

	bool str_offset_iso_to_offsetms(const char *str, int64_t &offsetms)
	{
		char sign = '0';
		unsigned long long hh = 0, mm = 0;

		offsetms = 0;

		if (sscanf_s(str, "%1c%02llu:%02llu", &sign,
#ifdef _WIN32
		(unsigned)1,
#endif
			&hh, &mm) != 3)
			return false;

		int64_t off_abs = (hh * 3600000ll) + (mm * 60000ll);

		if (sign == '-')
			offsetms = -off_abs;
		else if (sign == '+')
			offsetms = off_abs;
		else
			return false;

		return true;
	}

	std::string timems_to_str_datepath(uint64_t timems, bool date_only)
	{
		std::tm tm_struct;
		char buf[300];
		int part_ms = (timems % 1000);
		uint64_t timesec = timems;
		timesec /= 1000;
		{
			// TODO: test gmtime in linux
#if defined(_WIN32)
			gmtime_s(&tm_struct, (std::time_t*)&timesec);
#else
			std::tm* tm_ret;
			tm_ret = gmtime_r((std::time_t*)&timesec, &tm_struct);
#endif
			if (date_only)
			{
				strftime(buf, 30, "%Y/%m/%d", &tm_struct);
			}
			else
			{
				strftime(buf, 30, "%Y/%m/%d/%H_%M_%S_", &tm_struct);
			}
		}

		std::string result = std::string(buf);

		if (!date_only)
		{
			snprintf(buf, sizeof(buf), "%03d", part_ms);
			result += buf;
		}

		return result;
	}

	std::string duration2stringMcs(std::chrono::microseconds mcs)
	{
		std::stringstream ss;

		auto h = std::chrono::duration_cast<std::chrono::hours>(mcs);
		mcs -= h;
		auto m = std::chrono::duration_cast<std::chrono::minutes>(mcs);
		mcs -= m;
		auto s = std::chrono::duration_cast<std::chrono::seconds>(mcs);
		mcs -= s;

		ss << std::setfill('0') << std::setw(2) << h.count() << ":"
			<< std::setw(2) << m.count() << ":"
			<< std::setw(2) << s.count() << "."
			<< std::setw(6) << mcs.count();

		return ss.str();
	}

	std::string duration2stringMs(std::chrono::milliseconds ms)
	{
		std::stringstream ss;

		auto h = std::chrono::duration_cast<std::chrono::hours>(ms);
		ms -= h;
		auto m = std::chrono::duration_cast<std::chrono::minutes>(ms);
		ms -= m;
		auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
		ms -= s;

		ss << std::setfill('0') << std::setw(2) << h.count() << ":"
			<< std::setw(2) << m.count() << ":"
			<< std::setw(2) << s.count() << "."
			<< std::setw(3) << ms.count();

		return ss.str();
	}

	std::wstring utf8_to_wstring(const std::string &str)
	{
		QString s = QString::fromUtf8(str.data(), (int)str.size());
		return s.toStdWString();

		/*
	#if defined(_WIN32)
	#if __cplusplus >= 201703L || 1
		const auto size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		static thread_local std::vector<wchar_t> buffer;
		buffer.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.data(), buffer.size());
		return std::wstring(buffer.data());
	#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	#endif
	#else
		// TODO: linux utf8 convert, deprecated in gcc c++17
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	#endif
		*/
	}

	std::string wstring_to_utf8(const std::wstring &str)
	{
		QString s = QString::fromStdWString(str);
		return s.toUtf8().toStdString();

		/*
	#if defined(_WIN32)
	#if __cplusplus >= 201703L || 1
		const auto size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0,
											  NULL,//must be set for CP_UTF8
											  NULL //must be set for CP_UTF8
		);
		static thread_local std::vector<char> buffer;
		buffer.resize(size);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, buffer.data(), buffer.size(),
							NULL,//must be NULL for CP_UTF8
							NULL //must be NULL for CP_UTF8
		);
		return std::string(buffer.data());

	#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(str);
	#endif
	#else
		// TODO: linux utf8 convert, deprecated in gcc c++17
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	#endif
		*/
	}

	bool caseInsCompare(const std::string &s1, const std::string &s2)
	{
		// TODO: thera are issues with current locale and lowercase
#if defined(_WIN32)
		return (_stricmp(s1.c_str(), s2.c_str()) == 0);
#else
		return  (strcasecmp(s1.c_str(), s2.c_str()) == 0);
#endif

	}

	bool caseInsCompare(std::wstring s1, std::wstring s2)
	{
		const auto loc = std::locale("");
		//    std::transform(s1.begin(), s1.end(), s1.begin(), std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>), loc));
		std::transform(s1.begin(), s1.end(), s1.begin(), [&loc](wchar_t c) {return std::tolower(c, loc); });
		//    std::transform(s2.begin(), s2.end(), s2.begin(), std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>), loc));
		std::transform(s2.begin(), s2.end(), s2.begin(), [&loc](wchar_t c) {return std::tolower(c, loc); });

		return (s1 == s2);
	}

	std::string stringToUpper(const std::string &str)
	{
		return QString::fromUtf8(str.c_str(), (int)str.size()).toUpper().toStdString();
	}

	std::string stringToLower(const std::string &str)
	{
		return QString::fromUtf8(str.c_str(), (int)str.size()).toLower().toStdString();
	}

	std::string replaceAll(const std::string &str, const std::string &from, const std::string &to)
	{
		//return std::regex_replace(str, std::regex(from), to);

		std::string res = str;
		std::string::size_type pos = 0u;
		while ((pos = res.find(from, pos)) != std::string::npos)
		{
			res.replace(pos, from.length(), to);
			pos += to.length();
		}
		return res;
	}

	std::string replaceAll(const std::string &str, char from, char to)
	{
		//return std::regex_replace(str, std::regex(from), to);

		std::string res = str;
		std::string::size_type pos = 0u;
		while ((pos = res.find(from, pos)) != std::string::npos)
		{
			res.replace(pos, 1, 1, to);
			pos += 1;
		}
		return res;
	}

	std::vector<std::string> split(const std::string &s, char delim)
	{
		std::vector<std::string> result;
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim))
		{
			result.push_back(item);
		}

		return result;
	}

	bool starts_with(const std::string &str, const std::string &beg)
	{
		if (str.size() == 0 ||
			beg.size() == 0 ||
			str.size() < beg.size())
		{
			return false;
		}

		//return (str.find(beg) == 0)
		return (str.compare(0, beg.size(), beg) == 0);
	}

	bool ends_with(const std::string &str, char ending)
	{
		if (str.size() >= 1)
		{
			return str[str.size() - 1] == ending;
		}

		return false;
	}

	bool ends_with(const std::string &str, const std::string &ending)
	{
		if (str.size() >= ending.size())
		{
			return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
		}

		return false;
	}

	uint8_t *memmem(uint8_t *l, size_t l_len, uint8_t *s, size_t s_len)
	{
		const uint8_t *cur, *last;
		const uint8_t *cl = l;
		const uint8_t *cs = s;

		// a zero length needle should just return the haystack
		if (s_len == 0)
			return (uint8_t *)cl;

		// "s" must be smaller or equal to "l"
		if (l_len < s_len)
			return NULL;

		// special case where s_len == 1
		if (s_len == 1)
			return (uint8_t *)memchr(l, *cs, l_len);

		// the last position where its possible to find "s" in "l"
		last = cl + l_len - s_len;

		for (cur = cl; cur <= last; cur++)
			if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
				return (uint8_t *)cur;

		return NULL;
	}

	std::string liplate_lat2cyr(const std::string& liplate)
	{
		static const std::unordered_map<uint, uint> unicode_map =
		{
			{ 0x41, 0x0410 }, /// A -> À
			{ 0x42, 0x0412 }, /// B -> Â
			{ 0x43, 0x0421 }, /// C -> Ñ
			{ 0x45, 0x0415 }, /// E -> Å
			{ 0x48, 0x041D }, /// H -> Í
			{ 0x4B, 0x041A }, /// K -> Ê
			{ 0x4D, 0x041C }, /// M -> Ì
			{ 0x4F, 0x041E }, /// O -> Î
			{ 0x50, 0x0420 }, /// P -> Ð
			{ 0x54, 0x0422 }, /// T -> Ò
			{ 0x58, 0x0425 }, /// X -> Õ
			{ 0x59, 0x0423 }, /// Y -> Ó
			{ 0x61, 0x0430 }, /// a -> à
			{ 0x62, 0x0432 }, /// b -> â
			{ 0x63, 0x0441 }, /// c -> ñ
			{ 0x65, 0x0435 }, /// e -> å
			{ 0x68, 0x043D }, /// h -> í
			{ 0x6B, 0x043A }, /// k -> ê
			{ 0x6D, 0x043C }, /// m -> ì
			{ 0x6F, 0x043E }, /// o -> î
			{ 0x70, 0x0440 }, /// p -> ð
			{ 0x74, 0x0442 }, /// t -> ò
			{ 0x78, 0x0445 }, /// x -> õ
			{ 0x79, 0x0443 }  /// y -> ó
		};

		QString data = QString::fromUtf8(liplate.c_str(), (int)liplate.size());
		for (auto it_sym = data.begin(); it_sym != data.end(); ++it_sym)
		{
			const auto map_it = unicode_map.find(it_sym->unicode());

			if (map_it != unicode_map.end())
				*it_sym = QChar(map_it->second);
		}

		return data.toStdString();
	}

	std::string liplate_cyr2lat(const std::string& liplate)
	{
		static const std::unordered_map<uint, uint> unicode_map =
		{
			{ 0x0410, 0x41 }, /// À -> A 
			{ 0x0412, 0x42 }, /// Â -> B
			{ 0x0421, 0x43 }, /// Ñ -> C
			{ 0x0415, 0x45 }, /// Å -> E
			{ 0x041D, 0x48 }, /// Í -> H
			{ 0x041A, 0x4B }, /// Ê -> K
			{ 0x041C, 0x4D }, /// Ì -> M
			{ 0x041E, 0x4F }, /// Î -> O
			{ 0x0420, 0x50 }, /// Ð -> P
			{ 0x0422, 0x54 }, /// Ò -> T
			{ 0x0425, 0x58 }, /// Õ -> X
			{ 0x0423, 0x59 }, /// Ó -> Y
			{ 0x0430, 0x61 }, /// à -> a
			{ 0x0432, 0x62 }, /// â -> b
			{ 0x0441, 0x63 }, /// ñ -> c
			{ 0x0435, 0x65 }, /// å -> e
			{ 0x043D, 0x68 }, /// í -> h
			{ 0x043A, 0x6B }, /// ê -> k
			{ 0x043C, 0x6D }, /// ì -> m
			{ 0x043E, 0x6F }, /// î -> o
			{ 0x0440, 0x70 }, /// ð -> p
			{ 0x0442, 0x74 }, /// ò -> t
			{ 0x0445, 0x78 }, /// õ -> x
			{ 0x0443, 0x79 }  /// ó -> y
		};

		QString data = QString::fromUtf8(liplate.c_str(), (int)liplate.size());
		for (auto it_sym = data.begin(); it_sym != data.end(); ++it_sym)
		{
			const auto map_it = unicode_map.find(it_sym->unicode());

			if (map_it != unicode_map.end())
				*it_sym = QChar(map_it->second);
		}

		return data.toStdString();
	}

	std::string replaceNonPathChars(const std::string& str, char sym)
	{
		std::string res = str;
		std::replace_if(res.begin(), res.end(), [](const char& ch)
		{
			return (ch >= 0x01 && ch <= 0x1F) /// control characters
				|| ch == 0x21 /// '!'
				|| ch == 0x22 /// '"'
				|| ch == 0x2A /// '*'
				|| ch == 0x2F /// '/'
				|| ch == 0x3A /// ':'
				|| ch == 0x3C /// '<'
				|| ch == 0x3E /// '>'
				|| ch == 0x3F /// '?'
				|| ch == 0x5C /// '\'
				|| ch == 0x7C; /// '|'
		}, sym);

		return res;
	}

	std::string removeAll(const std::string &str, char ch)
	{
		std::string res = str;
		res.erase(std::remove(res.begin(), res.end(), ch), res.end());

		return res;
	}

	std::string removeAll(const std::string &str, const std::vector<char> &symbols)
	{
		std::string res = str;
		res.erase(std::remove_if(res.begin(), res.end(), [&symbols](const char& ch)
		{
			for (const auto &s : symbols)
			{
				if (ch == s) return true;
			}
			return false;
		}), res.end());

		return res;
	}

	namespace system {
		bool isAdmin()
		{
			bool ret = false;
#if defined(_WIN32)
			HANDLE hToken = NULL;
			if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			{
				TOKEN_ELEVATION Elevation;
				DWORD cbSize = sizeof(TOKEN_ELEVATION);

				if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
				{
					ret = (bool)Elevation.TokenIsElevated;
				}
			}
			if (hToken)
			{
				CloseHandle(hToken);
			}
#else
			ret = true;
#endif
			return ret;
		}

		unsigned long getCurrentProcessId()
		{
#if defined(_WIN32)
			return GetCurrentProcessId();
#else
			return (unsigned long)(::getpid());
#endif
		}

		void setConsoleTitle(const std::string &title)
		{
#if defined(_WIN32)
			(void)SetConsoleTitle(title.c_str());
#else
			// Not supported under linux
#endif
		}
#if defined(_WIN32)
		int trySetInstance(const std::string &uniqName)
		{
			// Warning: Beware of using boost::interprocess::named_mutex for locks.
			//          There are some issues/bugs on windows when application crashes without unlocking.
			//          #include <boost/interprocess/sync/named_mutex.hpp>
			static HANDLE instance_mutex = INVALID_HANDLE_VALUE;
			if (instance_mutex == INVALID_HANDLE_VALUE)
			{
				instance_mutex = CreateMutex(NULL, TRUE, uniqName.c_str());
				return (GetLastError() != ERROR_ALREADY_EXISTS) ? 1 : 0;
			}
			return -1;
		}
#elif defined(linux)
		int trySetInstance(const std::string &uniqName, const std::string &filepath, int *err)
		{
			// POSIX record locks (fcntl) method with file lockwrite

			// TODO: pre-create lock file on install application from ROOT!
			//       May be no access to create files in /var/lock (removed on reboot! need to pre-create file on system startup always)
			//       /var/run also needs root.
			//       May be /tmp or /var/tmp, but the pidfile's name can't be uniquely determined if it's in a world-writable directory.
			//       or use $HOME (~)
			// Oracul:  /var/lock -> /tmp

			int fd = open(filepath.c_str(), O_RDWR | O_CREAT, 0600);
			if (fd < 0)
			{
				if (err) *err = errno;
				return -1;
			}

			struct flock fl;
			fl.l_start = 0;
			fl.l_len = 0;
			fl.l_type = F_WRLCK;
			fl.l_whence = SEEK_SET;
			if (fcntl(fd, F_SETLK, &fl) < 0)
			{
				return 0;
			}

			// Write self pid to locked file
			char buf[11 + 1 + 1] = { 0 };  // 11 + \n + \0
			unsigned long pid = tools::system::getCurrentProcessId();
			// Format: 11 pid number chars right-justified and padded by space + "\n"
			// See Filesystem Hierarchy Standard.  5.9. /var/lock : Lock files. (UUCP Lock Files format like for devices)
			snprintf(buf, sizeof(buf), "%11lu\n", pid);
			int ret = write(fd, buf, sizeof(buf) - 1);
			if (ret < 0)
			{
				if (err) *err = errno;
				return -1;
			}

			//unlink(filepath.c_str());

			return 1;
		}
#endif

		std::string getCurrentPath()
		{
			std::error_code ec;
			auto p = std::filesystem::current_path(ec);

			return (!ec) ? p.generic_string() : "";
		}

		SysStatCountersContext::SysStatCountersContext()
		{
		}
		SysStatCountersContext::~SysStatCountersContext()
		{
			cleanup();
		}

		void SysStatCountersContext::cleanup()
		{
#if defined(_WIN32)
			if (query)
			{
				PdhCloseQuery(query);
			}
#elif defined(linux)
			cpuLastTotalUser = 0;
			cpuLastTotalUserLow = 0;
			cpuLastTotalSys = 0;
			cpuLastTotalIdle = 0;
#endif
		}

		int perfCountersInit(SysStatCountersContext *context)
		{
			if (!context)
				return 1;

#if defined(_WIN32)
			//PDH_HQUERY cpuQuery;
			//PDH_HCOUNTER cpuTotal;

			auto ret = PdhOpenQuery(NULL, NULL, (PDH_HQUERY*)&context->query);
			if (ret != ERROR_SUCCESS)
			{
				//printf("PdhOpenQuery err: %ld\n", ret);
				return ret;
			}
			// You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
			ret = PdhAddEnglishCounter((PDH_HQUERY)context->query, "\\Processor(_Total)\\% Processor Time", NULL, (PDH_HCOUNTER*)&context->cpuTotal);
			if (ret != ERROR_SUCCESS)
			{
				//printf("PdhAddEnglishCounter err: %ld\n", ret);
				if (ret == PDH_CSTATUS_BAD_COUNTERNAME || ret == PDH_CSTATUS_NO_COUNTER || ret == PDH_CSTATUS_NO_COUNTERNAME || ret == PDH_CSTATUS_NO_OBJECT)
				{
					ret = PdhAddEnglishCounter((PDH_HQUERY)context->query, "\\Processor Information(_Total)\\% Processor Time", NULL, (PDH_HCOUNTER*)&context->cpuTotal);
					if (ret != ERROR_SUCCESS)
					{
						//printf("PdhAddEnglishCounter2 err: %ld\n", ret);
						return ret;
					}
				}
				return ret;
			}

			ret = PdhCollectQueryData((PDH_HQUERY)context->query);
			if (ret != ERROR_SUCCESS)
			{
				//printf("PdhCollectQueryData err: %ld\n", ret);
				return ret;
			}
			//return ERROR_SUCCESS;

#elif defined(linux)
			FILE* file = fopen("/proc/stat", "r");
			if (!file)
			{
				return 1;
			}

			/*
			Example from Linux with 8 CPU cores:
				cpu 79242 0 74306 842486413 756859 6140 67701 0
				cpu0 49663 0 40234 104757317 542691 4420 39572 0
				cpu1 2724 0 2118 105420424 767 1719 6084 0
				cpu2 18578 0 18430 105191522 204592 0 714 0
				cpu3 513 0 979 105428698 739 0 2907 0
				cpu4 1623 0 2105 105426291 444 0 3373 0
				cpu5 3491 0 5326 105414798 7134 0 3087 0
				cpu6 1636 0 3081 105420689 201 0 8229 0
				cpu7 1011 0 2029 105426670 288 0 3731 0
				...

			The meanings of the columns are as follows, from left to right:
				1st : user = normal processes executing in user mode
				2nd : nice = niced processes executing in user mode
				3rd : system = processes executing in kernel mode
				4th : idle = twiddling thumbs
				5th : iowait = waiting for I/O to complete
				6th : irq = servicing interrupts
				7th : softirq = servicing softirqs

			Since Linux 2.6.11, there is an 8th column called 'steal' - counts the ticks spent executing other virtual hosts (in virtualised environments like Xen)
			Since Linux 2.6.24, there is a 9th column called 'guest' - counts the time spent running a virtual CPU for guest operating systems under the control of the Linux kernel

			Sum up all the columns in the 1st line "cpu" :
				( user + nice + system + idle + iowait + irq + softirq )
			this will yield 100% of CPU time

			Calculate the average percentage of total 'idle' out of 100% of CPU time :
				( user + nice + system + idle + iowait + irq + softirq ) = 100%
				( idle ) = X %
			hence
				average idle percentage X % = ( idle * 100 ) / ( user + nice + system + idle + iowait + irq + softirq )
			*/

			auto n = fscanf(file, "cpu %llu %llu %llu %llu",
				&context->cpuLastTotalUser,
				&context->cpuLastTotalUserLow,
				&context->cpuLastTotalSys,
				&context->cpuLastTotalIdle);
			if (n != 4)
			{
				return 1;
			}
			fclose(file);

#endif

			return 0;
		}

		int getPerfCurrentValueCpuTotal(SysStatCountersContext *context, double &result)
		{
			if (!context)
				return 1;

#if defined(_WIN32)
			PDH_FMT_COUNTERVALUE counterVal;

			auto ret = PdhCollectQueryData((PDH_HQUERY)context->query);
			if (ret != ERROR_SUCCESS)
			{
				//printf("PdhCollectQueryData err: %ld\n", ret);
				return ret;
			}

			ret = PdhGetFormattedCounterValue((PDH_HCOUNTER)context->cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
			if (ret != ERROR_SUCCESS)
			{
				//printf("PdhGetFormattedCounterValue err: %ld\n", ret);
				return ret;
			}

			result = counterVal.doubleValue;

			//return ERROR_SUCCESS;
#elif defined(linux)

			double percent = 0.0;
			unsigned long long totalUser = 0, totalUserLow = 0, totalSys = 0, totalIdle = 0, total = 0;

			FILE* file = fopen("/proc/stat", "r");
			if (!file)
			{
				return 1;
			}
			auto n = fscanf(file, "cpu %llu %llu %llu %llu",
				&totalUser, &totalUserLow, &totalSys, &totalIdle);
			if (n != 4)
			{
				return 1;
			}
			fclose(file);

			if (totalUser < context->cpuLastTotalUser ||
				totalUserLow < context->cpuLastTotalUserLow ||
				totalSys < context->cpuLastTotalSys ||
				totalIdle < context->cpuLastTotalIdle)
			{
				//Overflow detection. Just skip this value.
				percent = -1.0;
			}
			else
			{
				total =
					(totalUser - context->cpuLastTotalUser) +
					(totalUserLow - context->cpuLastTotalUserLow) +
					(totalSys - context->cpuLastTotalSys);
				percent = total;
				total += (totalIdle - context->cpuLastTotalIdle);
				if (total != 0)
				{
					percent /= total;
					percent *= 100;
				}
				else
				{
					// some error
					percent = -1.0;
				}
			}

			context->cpuLastTotalUser = totalUser;
			context->cpuLastTotalUserLow = totalUserLow;
			context->cpuLastTotalSys = totalSys;
			context->cpuLastTotalIdle = totalIdle;

			result = percent;

#endif

			return 0;
		}

		int parseMemInfoLine(const char* line)
		{
			int len = (int)strlen(line);
			if (len == 0) return -1;
			//if (len < 3) return -1;
			const char* p = line;
			while ((p < (line + len)) && (*p < '0' || *p > '9'))
			{
				if (*p == 0) return -1;
				p++;
			}
			if (p >= (line + len)) return -1;

			//line[i - 3] = '\0';  // This assumes that a digit will be found and the line ends in " Kb".
			return atoi(p);
		}

		int getSysStatMem(SysStatMem &stat)
		{
			memset(&stat, 0, sizeof(stat));

#if defined(_WIN32)
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);

			if (!GlobalMemoryStatusEx(&memInfo))
				return 1;

			// Total Virtual Memory
			stat.mem_virt_total = memInfo.ullTotalPageFile;

			// Virtual Memory currently used
			stat.mem_virt_used = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

			// Virtual Memory currently used by current process
			PROCESS_MEMORY_COUNTERS_EX pmc;
			if (!GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
				return 1;
			stat.mem_virt_proc_used = pmc.PrivateUsage;

			// Total Physical Memory (RAM)
			stat.mem_phy_total = memInfo.ullTotalPhys;

			// Physical Memory currently used
			stat.mem_phy_used = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

			// Physical Memory currently used by current process
			stat.mem_phy_proc_used = pmc.WorkingSetSize;
#else
			struct sysinfo memInfo;
			if (sysinfo(&memInfo))
			{
				// TODO: use errno for error
				return 1;
			}

			// Total Virtual Memory
			stat.mem_virt_total = memInfo.totalram;
			//add other values in next statement to avoid int overflow on right hand side
			stat.mem_virt_total += memInfo.totalswap;
			stat.mem_virt_total *= memInfo.mem_unit;

			// Virtual Memory currently used
			stat.mem_virt_used = memInfo.totalram - memInfo.freeram;
			//add other values in next statement to avoid int overflow on right hand side
			stat.mem_virt_used += memInfo.totalswap - memInfo.freeswap;
			stat.mem_virt_used *= memInfo.mem_unit;

			int64_t vmsize = 0;
			int64_t vmrss = 0;
			{
				// read /proc/self/status data
				FILE* file = fopen("/proc/self/status", "r");
				if (!file)
				{
					return 1;
				}
				char line[128];
				bool found_vmsize = false;
				bool found_vmrss = false;
				int found_cnt = 0;
				const int found_max_cnt = 2;
				memset(line, 0, sizeof(line));
				while ((found_cnt < found_max_cnt) && fgets(line, 128, file) != NULL)
				{
					if (!found_vmsize && strncmp(line, "VmSize:", 7) == 0)  //Note: this value is in KB!
					{
						vmsize = parseMemInfoLine(line);
						vmsize = (vmsize > 0) ? (vmsize * 1024) : 0;  // not used error checking
						// TODO: check errors while getting mem size values
						found_vmsize = true;
						found_cnt++;
					}

					if (!found_vmrss && strncmp(line, "VmRSS:", 6) == 0)  //Note: this value is in KB!
					{
						vmrss = parseMemInfoLine(line);
						vmrss = (vmrss > 0) ? (vmrss * 1024) : 0;  // not used error checking
						// TODO: check errors while getting mem size values
						found_vmrss = true;
						found_cnt++;
					}
				}
				fclose(file);
			}

			// Virtual Memory currently used by current process
			stat.mem_virt_proc_used = vmsize;

			// Total Physical Memory (RAM)
			stat.mem_phy_total = memInfo.totalram;
			//multiply in next statement to avoid int overflow on right hand side
			stat.mem_phy_total *= memInfo.mem_unit;

			// Physical Memory currently used
			stat.mem_phy_used = memInfo.totalram - memInfo.freeram;
			//multiply in next statement to avoid int overflow on right hand side
			stat.mem_phy_used *= memInfo.mem_unit;

			// Physical Memory currently used by current process
			stat.mem_phy_proc_used = vmrss;

#endif

			return 0;
		}

	}

} // namespace tools

