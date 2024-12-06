#include "Path.hpp"
#include "Cool/Path/Path.h"

namespace Path {

auto installed_versions_folder() -> std::filesystem::path
{
    return Cool::Path::user_data() / "Installed Versions";
}

auto projects_info_folder() -> std::filesystem::path
{
    return Cool::Path::user_data() / "Projects";
}

} // namespace Path