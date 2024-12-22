#pragma once
#include <httplib.h>
#include "Cool/Task/Task.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"

class Task_FetchListOfVersions : public Cool::Task {
public:
    explicit Task_FetchListOfVersions(std::optional<ImGuiNotify::NotificationId> warning_notification_id = {})
        : _warning_notification_id{warning_notification_id}
    {}

    void do_work() override;
    void cancel() override { _cancel.store(true); }
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return false; }
    auto name() const -> std::string override { return "Fetching the list of versions that are available online"; }

private:
    void handle_error(httplib::Result const& res);

private:
    std::atomic<bool>                          _cancel{false};
    std::optional<ImGuiNotify::NotificationId> _warning_notification_id{};
};