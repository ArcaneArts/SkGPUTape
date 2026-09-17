#pragma once
#include <cstddef>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace tape {
struct ModelList {
    std::vector<std::string> paths;
    std::size_t discovered{}, duplicates{}, invalid{}, excludedByLimit{};
    std::vector<std::size_t> invalidLines;
};
std::optional<std::string> NormalizeModelPath(std::string_view input);
ModelList ReadModelList(std::istream& input, std::size_t maxModels);
// BSModelDB uses paths relative to the meshes resource prefix.
std::string ModelDBPath(std::string_view normalized);
}
