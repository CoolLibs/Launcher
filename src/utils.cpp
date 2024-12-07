#include "utils.hpp"

// which OS ?
auto get_OS() -> std::string
{
#ifdef __APPLE__
    return "MacOS";
#elif _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#else
    static_assert(false);
#endif
}

// check if the download url is targeting a zip file.
auto is_zip_download(const std::string& download_url) -> bool
{
    return (download_url.find(get_OS() + ".zip") != std::string::npos);
}
