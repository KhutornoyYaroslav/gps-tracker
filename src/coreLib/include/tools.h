#pragma once

#include <vector>
#include <chrono>
#include <string>

namespace tools {

	/////////////
	/// Time
	/////////////

	extern uint64_t getPerfCntrMs(bool refreshFreq = false);

	/**
	 * Get local timezone offset in seconds
	 */
	extern int64_t local_timezone_offset();

	/**
	 * Convert datetime string to int timestamp in milliseconds
	 * input: str
	 * output: timems
	 * Example input datetime format:
	 *   YYYY-MM-DD hh:mm:ss.nnnnnn
	 *   2020-03-24 15:16:04.930000
	 *   2020-03-24 15:16:18.510063
	 * Note: cuts datetime to ms
	 */
	extern bool str_datetime_to_timems(const char *str, uint64_t &timems, bool toUTC = false);

	/**
	 * Convert verify date string to int timestamp in milliseconds
	 * input: str
	 * output: timems
	 * Example input date format:
	 *   DD.MM.YYYY
	 *   24.03.2020
	 *   03.02.2018
	 * Note: cuts input datetime to ms
	 */
	extern bool str_verifydate_to_timems(const char *str, uint64_t &timems, bool toUTC = false);

	/**
	 * Convert datetime string to int timestamp in seconds
	 * input: str
	 * output: timet
	 * Return: bool Success or fail
	 * Example input datetime format:
	 *   YYYY-MM-DD hh:mm:ss
	 *   2020-03-24 15:16:04
	 *   2020-03-24 15:16:18
	 */
	extern bool str_datetime_to_time(const char *str, time_t &timet, bool toUTC = false);

	/**
	 * Convert ISO 8601 datetime string to int timestamp in seconds
	 * input: str
	 * output: timet, offset_h
	 * Return: bool Success or fail
	 * Example input datetime format:
	 *   YYYY-MM-DDThh:mm:ss±hh
	 *   2020-03-24T15:16:18+03
	 *   2020-03-24T10:16:18-02
	 *   2020-03-24T12:16:18+00
	 *   2020-03-24T12:16:18Z
	 */
	extern bool str_datetime_iso_to_time(const char *str, time_t &timet, int64_t &offset_h, bool toUTC = false);

	/**
	  * Convert int timestamp in milliseconds to datetime string
	  * input: timems
	  * Return: std::string
	  * Example output datetime format:
	  *   YYYY-MM-DD hh:mm:ss.nnnnnn
	  *   2020-03-24 15:16:04.930000
	  *   2020-03-24 15:16:18.510063
	  */
	extern std::string timems_to_str_datetime(uint64_t timems);
	extern int timems_to_str_datetime(uint64_t timems, char* buffer, size_t bufferSize, char dateDelim = '-');

	extern std::string timems_to_str_date(uint64_t timems);
	extern int timems_to_str_date(uint64_t timems, char* buffer, size_t bufferSize, char dateDelim = '-');

	/**
	  * Convert int timestamp in milliseconds to datetime ISO 8601 format string
	  * input: timems
	  * Return: std::string
	  * Example output datetime format:
	  *   YYYY-MM-DDThh:mm:ss±hh
	  *   2020-03-24T15:16:04+03
	  *   2020-03-24T15:16:18-01
	  *   2020-03-24T15:15:18+00
	  */
	extern std::string timems_to_str_datetime_iso(uint64_t timems, int64_t offset_h = 0);
	extern int timems_to_str_datetime_iso(uint64_t timems, char* buffer, size_t bufferSize, int64_t offset_h = 0);

	/**
	  * Convert int offset in milliseconds to ISO 8601 tz offset string
	  * input: int64_t
	  * Return: std::string
	  * Example output offset format:
	  *   ±hh:mm
	  *   +03:30
	  *   -01:00
	  */
	extern std::string offsetms_to_str_iso(int64_t offsetms);
	extern int offsetms_to_str_iso(int64_t offsetms, char* buffer, size_t bufferSize);

	/**
	 * Convert offset iso string to int in milliseconds
	 * input: str
	 * output: int64_t
	 * Example input string format:
	 *   ±hh:mm
	 *   -03:30 -> -12600000
	 */
	extern bool str_offset_iso_to_offsetms(const char *str, int64_t &offsetms);

	/**
	 * Convert int timestamp in seconds to datetime string
	 * input: timet
	 * Return: std::string
	 * Example output datetime format:
	 *   YYYY-MM-DD hh:mm:ss
	 *   2020-03-24 15:16:04
	 *   2020-03-24 15:16:18
	 */
	extern std::string time_to_str_datetime(time_t timet);

	/**
	  * Convert int timestamp in milliseconds to datetime string
	  * input: timems
	  * Return: std::string
	  * Example output datetime path format:
	  *   YYYY/MM/DD/hh_mm_ss_nnn
	  *   2020/03/24/15_16_04_930
	  * Example output datetime path format (date_only):
	  *   YYYY/MM/DD
	  *   2020/03/24
	  */
	extern std::string timems_to_str_datepath(uint64_t timems, bool date_only = false);

	extern std::string duration2stringMcs(std::chrono::microseconds mcs);
	extern std::string duration2stringMs(std::chrono::milliseconds ms);


	/////////////
	/// Strings
	/////////////

	// convert UTF-8 string to wstring
	extern std::wstring utf8_to_wstring(const std::string &str);


	// convert wstring to UTF-8 string
	extern std::string wstring_to_utf8(const std::wstring &str);

	extern bool caseInsCompare(const std::string &s1, const std::string &s2);

	extern bool caseInsCompare(std::wstring s1, std::wstring s2);

	inline std::wstring s2ws(const std::string &str)
	{
		return utf8_to_wstring(str);
	}

	inline std::string ws2s(const std::wstring &wstr)
	{
		return wstring_to_utf8(wstr);
	}

	extern std::string stringToUpper(const std::string &str);

	extern std::string stringToLower(const std::string &str);

	extern std::string replaceAll(const std::string &str, const std::string &from, const std::string &to);

	extern std::vector<std::string> split(const std::string &s, char delim);

	inline bool starts_with(const std::string &str, char beg)
	{
		return str.size() >= 1 && str[0] == beg;
	}

	extern bool starts_with(const std::string &str, const std::string &beg);

	extern bool ends_with(const std::string &str, char ending);

	extern bool ends_with(const std::string &str, const std::string &ending);

	// Find the first occurrence of the byte string s in byte string l.
	extern uint8_t *memmem(const uint8_t *l, size_t l_len, const uint8_t *s, size_t s_len);

	/**
	 * Replace latin license plate symbols to cyrillic
	 * Replaces only these symbols:
	 *   A -> à, B -> â, C -> ñ, E -> å, H -> í, K -> ê,
	 *   M -> ì, O -> î, P -> ð, T -> ò, X -> õ, Y -> y
	 */
	extern std::string liplate_lat2cyr(const std::string& liplate);

	/**
	 * Replace cyrillic license plate symbols to latin
	 * Replaces only these symbols:
	 *   à -> A, â -> B, ñ -> C, å -> E, í -> H, ê -> K,
	 *   ì -> M, î -> O, ð -> P, ò -> T, õ -> X, y -> Y
	 */
	extern std::string liplate_cyr2lat(const std::string& liplate);

	/**
	 * Replaces path-invalid chars to specified symbol ('_' as default)
	 */
	extern std::string replaceNonPathChars(const std::string& str, char sym = '_');

	/**
	 * Replaces all char ch from string
	 */
	extern std::string removeAll(const std::string &str, char ch);

	/**
	 * Replaces all char symbols from string
	 */
	extern std::string removeAll(const std::string &str, const std::vector<char> &symbols);


	/////////////
	/// BASE64
	/////////////

	inline bool is_base64(unsigned char c)
	{
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	/////////////
	/// System
	/////////////

	namespace system {

		struct SysStatMem
		{
			unsigned long long mem_virt_total = 0;
			unsigned long long mem_virt_used = 0;
			unsigned long long mem_virt_proc_used = 0;

			unsigned long long mem_phy_total = 0;
			unsigned long long mem_phy_used = 0;
			unsigned long long mem_phy_proc_used = 0;

			long long cpu_used = 0;
			long long cpu_proc_used = 0;
		};

		struct SysStatCpu
		{
			double cpuTotal = 0.0;
			double cpuCurrentValue = 0.0;
		};

		class SysStatCountersContext
		{
		public:
			SysStatCountersContext();
			~SysStatCountersContext();
			void cleanup();

#if defined(_WIN32)
			void *query = nullptr; //PDH_HQUERY
			// Counters
			void *cpuTotal = nullptr; //PDH_HCOUNTER

#elif defined(linux)
			// Counters
			unsigned long long
				cpuLastTotalUser = 0,
				cpuLastTotalUserLow = 0,
				cpuLastTotalSys = 0,
				cpuLastTotalIdle = 0;
#endif
		};

		extern bool isAdmin();

		extern unsigned long getCurrentProcessId();

		extern void setConsoleTitle(const std::string &title);

		/**
		 * @brief  Try to lock some global named object in system
		 * @returns 1, if locking was successful
		 * @returns 0, when lock cannot be done
		 * @returns <0, on error
		 */
#if defined(_WIN32)
		extern int trySetInstance(const std::string &uniqName);
#elif defined(linux)
		extern int trySetInstance(const std::string &uniqName, const std::string &filepath, int *err);
#endif

		extern std::string getCurrentPath();

		extern int perfCountersInit(SysStatCountersContext *context);
		extern int getPerfCurrentValueCpuTotal(SysStatCountersContext *context, double &result);
		extern int getSysStatMem(SysStatMem &stat);

	} // namespace system
} // namespace duet::tools

