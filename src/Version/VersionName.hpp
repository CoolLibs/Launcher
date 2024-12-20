#pragma once
#include <compare>
#include <string>

class VersionName {
public:
    explicit VersionName(std::string name);

    auto as_string() const -> std::string const& { return _name; }

    auto major() const -> int { return _major; };
    auto minor() const -> int { return _minor; };
    auto patch() const -> int { return _patch; };
    auto is_experimental() const -> bool { return _is_experimental; }
    auto is_beta() const -> bool { return _is_beta; }

    auto is_valid() const -> bool { return _is_valid; };

    friend auto operator<=>(VersionName const&, VersionName const&) -> std::strong_ordering;
    friend auto operator==(VersionName const&, VersionName const&) -> bool;

private:
    std::string _name{};
    int         _major{0};
    int         _minor{0};
    int         _patch{0};
    bool        _is_experimental{false};
    bool        _is_beta{false};

    bool _is_valid{true};
};