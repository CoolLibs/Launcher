#include "Task_LaunchVersion.hpp"
#include "Cool/AppManager/close_application.hpp"
#include "Cool/Task/TaskManager.hpp"
#include "Cool/spawn_process.hpp"
#include "VersionManager.hpp"
#include "installation_path.hpp"

Task_LaunchVersion::Task_LaunchVersion(VersionRef version_ref, std::optional<std::filesystem::path> project_file_path)
    : Cool::Task{reg::generate_uuid() /* give a unique id to this task, so that we can cancel it */}
    , _version_ref{std::move(version_ref)}
    , _project_file_path{std::move(project_file_path)}
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
    ImGuiNotify::close_immediately(_notification_id);
}

void Task_LaunchVersion::execute()
{
    auto const* const version = version_manager().find_installed_version(_version_ref);
    if (!version || version->installation_status != InstallationStatus::Installed)
    {
        // TODO(Launcher) unexpected error, version got uninstalled before we could launch, please try again
        return;
    }

    Cool::spawn_process(
        executable_path(version->name),
        _project_file_path.has_value() ? std::vector<std::string>{_project_file_path->string()} : std::vector<std::string>{}
    );
    Cool::close_application_if_all_tasks_are_done(); // If some installations are still in progress we want to keep the launcher open so that they can finish. And once they are done, if we were to close the launcher it would feel weird for the user that it suddenly closed, so we just keep it open.
}