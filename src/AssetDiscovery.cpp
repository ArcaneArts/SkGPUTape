#include "AssetDiscovery.h"
#include <unordered_set>
namespace tape {
static std::string_view Trim(std::string_view s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == s.npos) return {};
    return s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}
std::optional<std::string> NormalizeModelPath(std::string_view input) {
    input = Trim(input);
    if (input.empty()) return std::nullopt;
    std::string path(input);
    for (auto& c : path) {
        if (static_cast<unsigned char>(c) < 32 || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') return std::nullopt;
        if (c == '/') c = '\\';
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    }
    if (path.starts_with("data\\")) path.erase(0, 5);
    if (!path.starts_with("meshes\\")) path = "meshes\\" + path;
    if (path.size() > 259 || !path.ends_with(".nif")) return std::nullopt;
    std::size_t start = 0;
    while (start < path.size()) {
        auto end = path.find('\\', start);
        if (end == path.npos) end = path.size();
        const auto part = std::string_view(path).substr(start, end - start);
        if (part.empty() || part == "." || part == ".." || part.back() == ' ' || part.back() == '.') return std::nullopt;
        start = end + 1;
    }
    return path;
}
ModelList ReadModelList(std::istream& input, std::size_t maxModels) {
    ModelList result;
    std::unordered_set<std::string> seen;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        if (++lineNumber == 1 && line.starts_with("\xEF\xBB\xBF")) line.erase(0, 3);
        auto value = Trim(line);
        if (value.empty() || value.front() == '#' || value.front() == ';') continue;
        ++result.discovered;
        auto path = NormalizeModelPath(value);
        if (!path) { ++result.invalid; result.invalidLines.push_back(lineNumber); continue; }
        if (!seen.insert(*path).second) { ++result.duplicates; continue; }
        if (maxModels && result.paths.size() >= maxModels) { ++result.excludedByLimit; continue; }
        result.paths.push_back(std::move(*path));
    }
    return result;
}
std::string ModelDBPath(std::string_view normalized) { return std::string(normalized.substr(7)); }
}
