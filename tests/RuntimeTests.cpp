#include <REL/REL.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

void Require(bool condition, const char* message) {
    if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}
template <class T> void Write(std::ofstream& file, const T& value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}
void MakeLibrary(REL::Version version, bool ae) {
    std::filesystem::create_directories("Data/SKSE/Plugins");
    const auto path = std::string("Data/SKSE/Plugins/") + (ae ? "versionlib-" : "version-") + version.string() + ".bin";
    std::ofstream file(path, std::ios::binary);
    file.exceptions(std::ios::badbit | std::ios::failbit);
    Write(file, std::int32_t(ae ? 2 : 1));
    for (int i = 0; i < 4; ++i) Write(file, std::int32_t(version[i]));
    Write(file, std::int32_t(0)); // No embedded executable name.
    Write(file, std::int32_t(8)); // Pointer size.
    Write(file, std::int32_t(1)); // One uncompressed mapping.
    Write(file, std::uint8_t(0));
    Write(file, std::uint64_t(ae ? 222 : 111));
    Write(file, std::uint64_t(0x1234));
}
void CheckLookup(bool ae) {
    Require(REL::Module::IsAE() == ae, "Wrong runtime classification");
    const REL::RelocationID id(111, 222);
    Require(id.id() == (ae ? 222 : 111), "Wrong SE/AE relocation ID");
    Require(id.offset() == 0x1234, "Wrong address-library mapping");
}
int main(int argc, char** argv) {
    Require(argc > 0, "Missing executable path");
    const auto executable = std::filesystem::absolute(argv[0]);
    // Fixtures live only in the build's test directory, never in an installed game.
    std::filesystem::create_directories("runtime-fixtures");
    std::filesystem::current_path("runtime-fixtures");
    for (const auto version : {REL::Version(1,5,97,0), REL::Version(1,6,1170,0),
                              REL::Version(1,7,99,0), REL::Version(1,7,104,0)}) {
        const bool ae = version[1] != 5;
        MakeLibrary(version, ae);
        Require(REL::Module::mock(version), "Cannot mock runtime");
        CheckLookup(ae);
        std::cout << "Passed runtime + database: " << version.string() << '\n';
    }
    // Exercise production load_version with real Windows PE version metadata,
    // not just the testing mock's separate classification branch.
    Require(REL::Module::inject(executable.wstring()), "Cannot load PE version fixture");
    Require(REL::Module::get().version() == REL::Version(1,7,104,0), "Wrong PE version");
    CheckLookup(true);
    std::cout << "Passed production PE runtime detection and AE database lookup\n";
}
