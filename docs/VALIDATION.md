# Version 1.0.1 validation

## XeSS input/focus recovery update

The gameplay-tested build was verified by SHA256 as `84af1675217a3858baf069427a19645308715b3c85265a656be305b8d4a0458f`. The user reported smooth gameplay on an RTX 3080 Ti after installation. The release rebuild updates version strings to 1.0.1; no functional changes were made after that live test.

One five-second XeSS 3x recovery sample contained 224 presents, 224 eligible inputs, 224 HUD-less captures, one initial history reset, 224 SDK-enabled batches and zero disabled batches. The SDK reported 670 outputs across 223 multi-output batches; the DXGI proxy reported 667 submissions. These counters are asynchronous and do not measure physical scanout. The previous measured interval had 240 presents, 223 eligible inputs, 17 disabled batches and 16 history resets. The intervals are not a controlled scene benchmark.

The new pre-acquire association was exercised after that sample. It confirms the corrected ordering occurs in RDR2; because the clean sample precedes the first binding, it does not establish that the ordering caused every earlier gap. The later 3x → 2x → 4x → 2x changes produced three brief reports with valid snapshots but no recent SR activity, coinciding with SDK reconfiguration. These remain a known transition behavior.

The separate native DLSS-G session reported stable 2x and 3x output ratios with zero query errors. It is not used as evidence of XeSS behavior. There is no exhaustive long-duration, physical scanout, all-hardware or clean-machine gameplay validation.

Test-AltTab passes focus-generation and pre-acquire association checks (including stale, ambiguous and outstanding-image rejection), 1000 focus cycles, input layouts, host raw-input/release preservation, contracts, history, recreation, FPS and recovery checks. Its isolated previous-behavior fixtures reproduce the expected failures without crashing. Test-Fixes covers adapter identity, XeSS pacing and runtime lifecycle. Release-default and package checks verify that diagnostic/developer settings are disabled and no personal configuration is shipped.

## Version 1.0 functional baseline

The GPU identity and XeSS timing fixes were tested on an RTX 3080 Ti. The user confirmed improved smoothness. The corrected core SHA256 is `0db2ad12bd534254e62d59c18c5a2f1708586a57b418bfd1c928c7d366387ea4`.

A post-fix capture verified the rebased deadline in all 592 complete 2x batches in that capture. It also contained 3x output, so the overall average is not a controlled 2x comparison. Software presentation timing is not an optical display measurement.

## Build and regression checks

- The main ASI and corrected core build with the project scripts and pinned official dependencies.
- Adapter identity regressions pass.
- 4338 XeSS pacing checks pass without failures.
- The production-runtime lifecycle fixture passes, including 100 ordered shutdowns.
- The actual menu renders in an isolated fixture on Generation, Display and Diagnostics at 100% and 150% scale.

Preview images contain example telemetry. `FGVK_MENU_PREVIEW` and its example data are not enabled in production builds. The final 1.0.1 label requires a separate visual check; the menu and functional fixes were exercised in the previous build. No universal hardware support, perfect HUD isolation or perfectly uniform physical scanout is claimed.

## Reproduce

```powershell
.\Build.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Test-Fixes.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Test-AltTab.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Build-Core.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Build-MenuPreview.ps1 -ZigPath 'C:\tools\zig\zig.exe'
```

The normal scripts do not start RDR2. Deliberate crash fixtures under `Build.ps1 -RunChecks` are outside this validation procedure.

## Installation package

The full installer was compared with the working mod runtime tree. Its official loader matches the installed loader byte for byte. All pinned runtime binary hashes and all ZIP entry hashes were verified. The 17 binary modules are Windows x64, and their static imports resolve to packaged modules or installed Windows, driver and Visual C++ components. This is a static packaging check, not a clean-machine gameplay test.

The root contains only `dinput8.dll`, `RDR2VulkanFG.asi` and `RDR2VulkanFG.ini`, plus the runtime and license directories. The only text document in the ZIP is `licenses/NOTICES.txt`. All original notice contents were verified to be present there. No game-owned DLLs, logs, PDBs, dumps, backups or personal INIs were copied.
