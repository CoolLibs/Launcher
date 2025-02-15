#include "App.hpp"
#include <imgui.h>
#include "Cool/CommandLineArgs/CommandLineArgs.h"
#include "Cool/DebugOptions/debug_options_windows.h"
#include "Cool/ImGui/ColorThemes.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/markdown.h"
#include "Cool/Log/ToUser.h"
#include "Cool/Task/TaskManager.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "LauncherSettings.hpp"
#include "Task_CheckForLongPathsEnabled.hpp"
#include "Version/VersionManager.hpp"
#include "Version/VersionName.hpp"
#include "Version/VersionRef.hpp"

App::App(Cool::WindowManager& windows, Cool::ViewsManager& /* views */)
    : _window{windows.main_window()}
{}

void App::init()
{
    // When double-clicking on a Coollab file, this will open the launcher and pass the path to that file as a command-line argument
    // We want to launch that project asap
    if (!Cool::command_line_args().get().empty())
        launch(Cool::command_line_args().get()[0]);

#if defined(_WIN32)
    if (_project_manager.has_some_projects()) // Don't show it the first time users open the launcher after installing it, because we don't want to scare them with something that might look like a virus
    {
        Cool::task_manager().submit(after(500ms), // Small delay to make sure users see it pop up and it draws their attention
                                    std::make_shared<Task_CheckForLongPathsEnabled>());
    }
#endif
}

void App::update()
{
    if (inputs_are_allowed() && !ImGui::GetIO().WantTextInput)
    {
    }
}

void App::on_shutdown()
{
    launcher_settings().save(); // Even if the user doesn't change the settings, we will save the settings they have seen once, so that if a new version of the software comes with new settings, we will not change settings that the user is used to
}

void App::imgui_windows()
{
    Cool::Log::ToUser::console().imgui_window();
    Cool::debug_options_windows(nullptr, _window);
    { // Versions
        ImGui::Begin("Versions");
        version_manager().imgui_manage_versions();
        ImGui::End();
    }

    { // New Project
        ImGui::Begin("New Project");
        if (Cool::ImGuiExtras::colored_button("New Project", Cool::color_themes()->editor().get_color("Accent")))
            version_manager().install_ifn_and_launch(_version_to_use_for_new_project, FolderToCreateNewProject{_projects_folder});
        ImGui::SameLine();
        version_manager().imgui_versions_dropdown(_version_to_use_for_new_project);
        Cool::ImGuiExtras::folder("", &_projects_folder);
        ImGui::SetItemTooltip("%s", "Folder where the new project will be saved.\nIf left empty or relative, it will be relative to the launcher's User Data folder");
        ImGui::End();
    }

    {
        ImGui::Begin("Projects");
        _project_manager.imgui([&](Project const& project) { launch(project); });
        ImGui::End();
    }

    // ImGui::ShowDemoWindow();
}

void App::imgui_menus()
{
    if (ImGui::BeginMenu("Settings"))
    {
        launcher_settings().imgui();
        ImGui::EndMenu();
    }

    ImGui::SetCursorPosX( // HACK while waiting for ImGui to support right-to-left layout. See issue https://github.com/ocornut/imgui/issues/5875
        ImGui::GetWindowSize().x
        - ImGui::CalcTextSize("Debug").x
        - 3.f * ImGui::GetStyle().ItemSpacing.x
        - ImGui::GetStyle().WindowPadding.x
    );
    if (ImGui::BeginMenu("Debug"))
    {
        DebugOptionsManager::imgui_ui_for_all_options();
        ImGui::EndMenu();
    }
}

void App::launch(Project const& project)
{
    auto const version = project.version_to_launch();
    if (!version.has_value())
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Error,
            .title   = "Can't open project",
            .content = "Unknown version",
        });
        return;
    }
    version_manager().install_ifn_and_launch(*version, FileToOpen{project.file_path()});
}

void App::launch(std::filesystem::path const& project_file_path)
{
    launch(Project{project_file_path, {}});
}