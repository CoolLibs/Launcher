#include "Task_LaunchVersion.hpp"
#include <ImGuiNotify/ImGuiNotify.hpp>
#include <filesystem>
#include <vector>
#include "Cool/AppManager/close_application.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/File/File.h"
#include "Cool/Log/ToUser.h"
#include "Cool/Task/TaskManager.hpp"
#include "Cool/Utils/overloaded.hpp"
#include "Cool/spawn_process.hpp"
#include "Path.hpp"
#include "Version/VersionRef.hpp"
#include "VersionManager.hpp"
#include "installation_path.hpp"

auto Task_LaunchVersion::name() const -> std::string
{
    return fmt::format(
        "Launching {}",
        std::visit(
            Cool::overloaded{
                [&](FileToOpen const& file) {
                    return fmt::format("\"{}\"", Cool::File::file_name_without_extension(file.path));
                },
                [&](FolderToCreateNewProject const&) {
                    return "a new project"s;
                },
            },
            _project_to_open_or_create
        )
    );
}

Task_LaunchVersion::Task_LaunchVersion(VersionRef version_ref, ProjectToOpenOrCreate project_to_open_or_create)
    : Cool::Task{reg::generate_uuid() /* give a unique id to this task, so that we can cancel it */}
    , _version_ref{std::move(version_ref)}
    , _project_to_open_or_create{std::move(project_to_open_or_create)}
{}

void Task_LaunchVersion::on_submit()
{
    _notification_id = ImGuiNotify::send({
        .type                 = ImGuiNotify::Type::Info,
        .title                = name(),
        .content              = fmt::format("Waiting for {} to install", as_string(_version_ref)),
        .custom_imgui_content = [task_id = owner_id()]() {
            if (ImGui::Button("Cancel"))
                Cool::task_manager().cancel_all(task_id);
        },
        .duration    = std::nullopt,
        .is_closable = false,
    });
}

void Task_LaunchVersion::cleanup(bool /* has_been_canceled */)
{
    if (!_error_message.empty())
    {
        ImGuiNotify::change(
            _notification_id,
            {
                .type     = ImGuiNotify::Type::Error,
                .title    = name(),
                .content  = _error_message,
                .duration = std::nullopt,
            }
        );
    }
    else
    {
        ImGuiNotify::close_immediately(_notification_id);
    }
}

void Task_LaunchVersion::execute()
{
    auto const* const version = version_manager().find_installed_version(_version_ref);
    if (!version || version->installation_status != InstallationStatus::Installed)
    {
        _error_message = fmt::format("Can't launch because we failed to install {}", as_string(_version_ref));
        return;
    }

    auto const path_arg = [](std::filesystem::path const& path) {
        return Cool::File::weakly_canonical(path).string(); // No need to have the path quoted because spawn_process() already handles args with spaces
    };

    auto args = std::vector<std::string>{
        "--projects_info_folder_for_the_launcher",
        path_arg(Path::projects_info_folder()),
    };
    std::visit(
        Cool::overloaded{
            [&](FileToOpen const& file) {
                args.emplace_back("--open_project");
                args.emplace_back(path_arg(file.path));
            },
            [&](FolderToCreateNewProject const& folder) {
                args.emplace_back("--create_new_project_in_folder");
                auto path = folder.path;
                if (path.empty())
                    path = Path::default_projects_folder();
                else if (Cool::File::is_relative(path))
                    path = Path::default_projects_folder() / path;
                args.emplace_back(path_arg(path));
            },
        },
        _project_to_open_or_create
    );

    auto const maybe_error = Cool::spawn_process(executable_path(version->name), args);
    if (maybe_error.has_value())
    {
        if (Cool::DebugOptions::log_internal_warnings())
            Cool::Log::ToUser::warning("Launch", *maybe_error);
        _error_message = fmt::format("{} is corrupted. You should uninstall and reinstall it.", as_string(_version_ref));
        return;
    }

    Cool::close_application_if_all_tasks_are_done(); // If some installations are still in progress we want to keep the launcher open so that they can finish. And once they are done, if we were to close the launcher it would feel weird for the user that it suddenly closed, so we just keep it open.
}