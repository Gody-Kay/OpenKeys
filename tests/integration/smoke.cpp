// Toolchain smoke test: C++17 compiles and runs on every CI OS.
#include <cstdio>
#include <optional>

int main() {
    std::optional<int> voices = 64;  // C++17 feature check; default voice cap (D009)
    std::printf("OpenKeys smoke ok: voice cap %d\n", voices.value());
    return voices.value() == 64 ? 0 : 1;
}
