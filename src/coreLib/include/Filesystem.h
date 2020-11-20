#pragma once

#include <filesystem>

//Restrict scope to msvc 2017
#if (defined(_MSC_VER) && (_MSC_VER >= 1911) && (_MSC_VER < 1920)) || (defined(__GNUC__) && (__GNUC__ <= 7))
namespace std
{
	namespace filesystem
	{
		using namespace experimental::filesystem;
	}
}
#endif

namespace duet
{
	namespace tools
	{
		inline std::uintmax_t directory_size(const std::filesystem::path& path, std::error_code& err) noexcept
		{
			namespace fs = std::filesystem;

			std::uintmax_t size{};

			for (fs::recursive_directory_iterator next{ path, err }, end{}; !err && (next != end); ++next)
			{
				if (fs::is_regular_file(next->status()))
				{
					auto fsize = fs::file_size(next->path(), err);
					if (!err)
					{
						size += fsize;
					}
					else
					{
						// TODO: add own message about file path
						break;
					}
				}
			}

			return size;
		}

		inline int64_t get_path_depth(const std::filesystem::path& p) noexcept
		{
			return  (p.empty()) ? 0 : std::distance(p.begin(), p.end());
		}
	}
}
