#pragma once
#include "Cool/AppManager/IApp.h"
#include "Cool/View/ViewsManager.h"
#include "Cool/Window/Window.h"
#include "Cool/Window/WindowManager.h"
#include "ProjectManager.hpp"
#include "ReleaseManager.hpp"

class App : public Cool::IApp {
public:
    App(Cool::WindowManager& windows, Cool::ViewsManager& /* views */);

    void imgui_windows() override;
    auto wants_to_show_menu_bar() const -> bool override { return false; }

private:
    void launch(Project const& project);
    void launch(std::filesystem::path const& project_file_path);

private:
    ReleaseManager _release_manager{};
    ProjectManager _project_manager{};
    Release const* _release_to_use_for_new_project{};
    Cool::Window&  _window; // NOLINT(*avoid-const-or-ref-data-members)

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& /* archive */)
    {
        // archive(
        //     ser20::make_nvp("Project Manager", _project_manager)
        // );
    }
};

// #include <cstdlib>
// #include <iostream>
// // #include "ProjectManager/path_to_project_to_open_on_startup.hpp"
// #include <fstream>
// #include <string>
// #include "ReleaseManager.hpp"
// #include "boxer/boxer.h"

// auto main(int argc, char** argv) -> int
// {
//     // TODO : faire une fonction!
//     std::string version;
//     if (argc > 1)
//     {
//         std::ifstream infile(argv[1]);
//         if (infile.is_open())
//             std::getline(infile, version);
//         infile.close();
//     }

//     try
//     {
// #ifdef __APPLE__
//         // Install Homebrew & FFmpeg on MacOS
//         install_macos_dependencies_if_necessary();
// #endif

//         ReleaseManager release_manager;
//         // release_manager.display_all_release();

//         Release const* release_to_launch = nullptr;

//         // L'utilisateur a déjà un projet sur une version différente de la latest
//         // Il a le choix de poursuivre son projet sur sa version, ou de télécharger la latest.
//         release_to_launch = release_manager.find_release(version);
//         if (release_to_launch == nullptr)
//         {
//             // std::cerr << "La version " << version << " n'a pas pu être installée, installation de la dernière version (par défaut).";
//             release_to_launch = release_manager.get_latest_release();
//             if (release_to_launch == nullptr)
//             {
//                 boxer::show("Please connect to the Internet so that we can install the latest version of Coollab.\nYou don't have any version installed yet.", "You are offline");
//                 return 0;
//             }
//         }

//         // Si la latest n'est pas installée -> on l'installe
//         if (!release_to_launch->is_installed())
//             release_manager.install_release(*release_to_launch);

//         release_manager.launch_release(*release_to_launch);
//     }
//     catch (const std::exception& e)
//     {
//         std::cerr << "Exception occurred: " << e.what() << std::endl;
//     }
// }
