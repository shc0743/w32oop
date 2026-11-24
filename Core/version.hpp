#include <cstdint>
namespace w32oop::internal {
    inline constexpr uint64_t MakeVersion(uint16_t major, uint16_t minor, uint16_t patch) {
        return static_cast<uint64_t>(major) * 1000000000000ull +
            static_cast<uint64_t>(minor) * 1000000ull +
            patch;
    }
}
namespace w32oop::core {
    constexpr uint64_t version = w32oop::internal::MakeVersion(1, 0, 0);
    constexpr const char *version_string() noexcept {
        return "w32oop::version_string 1.0.0-rc.1 (C++ Win32 Object-Oriented Programming Framework)";
    }
}
