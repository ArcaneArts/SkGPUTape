#include "AssetDiscovery.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}
int main() {
    const auto normalized = tape::NormalizeModelPath("  Data/Meshes/Clutter/Test.NIF\r ");
    require(normalized && *normalized == "meshes\\clutter\\test.nif", "normalize separators, case, prefix");
    require(tape::ModelDBPath(*normalized) == "clutter\\test.nif", "model resource prefix");
    for (const auto* bad : {"", "../x.nif", "/x.nif", "C:\\x.nif", "meshes//x.nif", "meshes/./x.nif", "x.dds", "meshes/../x.nif", "meshes/a /x.nif"})
        require(!tape::NormalizeModelPath(bad), bad);
    std::istringstream input("\xEF\xBB\xBF# UTF8 comment\n;comment\n\nMeshes/A.nif\na.NIF\n../bad.nif\nb.nif\nc.nif\nc.nif\n");
    const auto list = tape::ReadModelList(input, 2);
    require(list.discovered == 6 && list.paths.size() == 2 && list.duplicates == 2 && list.invalid == 1 && list.excludedByLimit == 1, "list accounting");
    require(list.invalidLines == std::vector<std::size_t>{6}, "invalid line number");
    std::istringstream unlimited("a.nif\nb.nif\n");
    require(tape::ReadModelList(unlimited, 0).paths.size() == 2, "zero means unlimited");
    std::cout << "Discovery tests passed\n";
}
