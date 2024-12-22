#include "Task_FetchListOfVersions.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/Log/ToUser.h"
#include "Cool/Task/TaskManager.hpp"
#include "VersionManager.hpp"
#include "nlohmann/json.hpp"

static auto get_OS() -> std::string
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
auto is_zip_download(std::string const& download_url) -> bool
{
    return download_url.find(get_OS() + ".zip") != std::string::npos;
}

void Task_FetchListOfVersions::do_work()
{
    auto cli = httplib::Client{"https://api.github.com"};
    cli.set_follow_location(true);
    // Don't cancel if we have a bad internet connection. This is done in a Task so this is non-blocking anyways
    cli.set_connection_timeout(99999h);
    cli.set_read_timeout(99999h);
    cli.set_write_timeout(99999h);

    auto const res = cli.Get("https://api.github.com/repos/CoolLibs/Lab/releases", [&](uint64_t, uint64_t) {
        return !_cancel.load();
    });

    if (!res || res->status != 200)
    {
        handle_error(res);
        return;
    }

    try // TODO(Launcher) Put a try on each iteration of the loop, so that an exception doesn't prevent other versions from loading
    {
        auto const json_response = nlohmann::json::parse(res->body);
        for (auto const& version : json_response)
        {
            if (_cancel.load())
                return;
            // if (!version["prerelease"])                     // we keep only non pre-release version // TODO actually, Experimental versions will be marked as preversion, but we still want to have them
            for (const auto& asset : version["assets"]) // for all download file of the current version
            {
                if (!is_zip_download(asset["browser_download_url"])) // Good version? => zip download file exists on the version (Only one per OS)
                    continue;

                auto const version_name = VersionName{version["name"]};
                if (!version_name.is_valid())
                    continue;
                version_manager().set_download_url(version_name, asset["browser_download_url"]);
                break;
            }
        }
    }
    catch (nlohmann::json::parse_error const& e)
    {
        // return fmt::format("JSON parsing error: {}", e.what());
    }
    catch (std::exception& e)
    {
        // return fmt::format("{}", e.what());
    }
    if (_warning_notification_id.has_value())
        ImGuiNotify::close_immediately(*_warning_notification_id);
}

void Task_FetchListOfVersions::handle_error(httplib::Result const& res)
{
    if (Cool::DebugOptions::log_debug_warnings())
    {
        Cool::Log::ToUser::warning(
            "Download version",
            !res ? httplib::to_string(res.error())
                 : fmt::format("Status code {}", std::to_string(res->status))
        );
    }

    auto message              = std::optional<std::string>{};
    auto duration_until_reset = std::chrono::seconds{};

    if (res && res->status == 403)
    {
        auto const it = res->headers.find("X-RateLimit-Reset");
        if (it != res->headers.end())
        {
            try
            {
                auto const reset_time_unix   = time_t{std::stoi(it->second)};
                auto const current_time_unix = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                duration_until_reset         = std::chrono::seconds{reset_time_unix - current_time_unix};
                auto const minutes           = duration_cast<std::chrono::minutes>(duration_until_reset);
                auto const seconds           = duration_until_reset - minutes;
                message                      = fmt::format("You need to wait {}\nYou opened the launcher more than 60 times in 1 hour, which is the maximum number of requests we can make to our online service to check for available versions", minutes.count() == 0 ? fmt::format("{}s", seconds.count()) : fmt::format("{}m {}s", minutes.count(), seconds.count()));
            }
            catch (...)
            {
            }
        }
    }

    auto const notification = ImGuiNotify::Notification{
        .type     = ImGuiNotify::Type::Warning,
        .title    = "Failed to check for new versions online",
        .content  = !res ? "No Internet connection" : message.value_or("Oops, our online versions provider is unavailable"),
        .duration = std::nullopt,
    };

    if (!_warning_notification_id)
        _warning_notification_id = ImGuiNotify::send(notification);
    else
        ImGuiNotify::change(*_warning_notification_id, notification);

    if (!res || message) // Only retry if we failed because we don't have an Internet connection, or because we hit the max number of requests to Github. There is no point in retrying if the service is unavailable, it's probably not gonna get fixed soon, and if we make too many requests to their API, Github will block us
        Cool::task_manager().submit_in(message ? duration_until_reset : 1s, std::make_shared<Task_FetchListOfVersions>(_warning_notification_id));
}