#pragma once
#include <cstdint>

namespace cuda_adapter {
// Resolve a unique physical identity, not the number of DXGI aliases. A failed
// CUDA query or two different matching identities must still fail closed.
struct GraphicsIdentitySelection {
    uint64_t luid = 0;
    bool selected = false;
    bool ambiguous = false;
    bool incomplete = false;

    constexpr void Observe(uint64_t candidate, bool complete, uint32_t matches) noexcept {
        if (!complete) { incomplete = true; return; }
        if (!matches) return;
        if (matches != 1 || (selected && luid != candidate)) {
            ambiguous = true;
            return;
        }
        luid = candidate;
        selected = true;
    }
    constexpr bool Unique() const noexcept { return selected && !ambiguous && !incomplete; }
};
}
