#pragma once

#include <string>

namespace seop::util
{
template <typename T>
constexpr size_t to_idx(T e)
{
    return static_cast<size_t>(e);
}

constexpr std::string get_extension(const std::string& path)
{
    return path.substr(path.rfind('.') + 1);
}

} // namespace seop::util
