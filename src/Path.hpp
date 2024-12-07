#pragma once

namespace Path {

/// Folder where all the Coollab releases will be installed
auto installed_versions_folder() -> std::filesystem::path;
/// Folder where all the projects info are stored, for all the projects that are tracked by the launcher
auto projects_info_folder() -> std::filesystem::path;

} // namespace Path