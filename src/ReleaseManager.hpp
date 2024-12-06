#pragma once
#include <string>
#include <tl/expected.hpp>
#include "Release.hpp"

class ReleaseManager {
public:
    ReleaseManager();

    [[nodiscard]] auto get_all_release() const -> const std::vector<Release>&;
    [[nodiscard]] auto get_latest_release() const -> const Release*;
    [[nodiscard]] auto find_release(const std::string& release_version) const -> const Release*;

    void display_all_release();
    auto no_release_installed() -> bool;

    void imgui();

private:
    std::vector<Release> all_release; // Sorted, from latest to oldest release
};