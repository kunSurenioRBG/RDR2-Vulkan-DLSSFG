#include "../vendor/ampere-core/adapter_identity_policy.h"
#include <cstdio>
using cuda_adapter::GraphicsIdentitySelection;
constexpr bool Check() {
    GraphicsIdentitySelection empty;
    if (empty.Unique()) return false;
    GraphicsIdentitySelection duplicate;
    duplicate.Observe(0xed1d, true, 1);
    duplicate.Observe(0x19ebe, true, 0);
    if (!duplicate.Unique() || duplicate.luid != 0xed1d) return false;
    GraphicsIdentitySelection reversed;
    reversed.Observe(0x19ebe, true, 0);
    reversed.Observe(0xed1d, true, 1);
    if (!reversed.Unique() || reversed.luid != 0xed1d) return false;
    duplicate.Observe(0xed1d, true, 1);
    if (!duplicate.Unique()) return false;
    GraphicsIdentitySelection physicals;
    physicals.Observe(1, true, 1);
    physicals.Observe(2, true, 1);
    if (physicals.Unique()) return false;
    GraphicsIdentitySelection failed;
    failed.Observe(1, true, 1);
    failed.Observe(2, false, 0);
    if (failed.Unique()) return false;
    GraphicsIdentitySelection multiple;
    multiple.Observe(1, true, 2);
    if (multiple.Unique()) return false;
    GraphicsIdentitySelection missing;
    missing.Observe(1, true, 0);
    if (missing.Unique()) return false;
    return true;
}
static_assert(Check(), "Adapter identity regressions");
int main() { if (!Check()) return 1; puts("Adapter identity regressions: PASS"); }
