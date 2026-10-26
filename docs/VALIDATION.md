# Version 1.0 validation

## Functional baseline

The GPU identity and XeSS timing fixes were tested on an RTX 3080 Ti. The user confirmed improved smoothness. The corrected core SHA256 is `0db2ad12bd534254e62d59c18c5a2f1708586a57b418bfd1c928c7d366387ea4`.

A post-fix capture verified the rebased deadline in all 592 complete 2x batches in that capture. It also contained 3x output, so the overall average is not a controlled 2x comparison. Software presentation timing is not an optical display measurement.

## Build and regression checks

- The main ASI and corrected core build with the project scripts and pinned official dependencies.
- Adapter identity regressions pass.
- 4338 XeSS pacing checks pass without failures.
- The production-runtime lifecycle fixture passes, including 100 ordered shutdowns.
- The actual menu renders in an isolated fixture on Generation, Display and Diagnostics at 100% and 150% scale.

Preview images contain example telemetry. `FGVK_MENU_PREVIEW` and its example data are not enabled in production builds. The new version label and frontend still require confirmation during gameplay. No universal hardware support, perfect HUD isolation or perfectly uniform physical scanout is claimed.

## Reproduce

```powershell
.\Build.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Test-Fixes.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Build-Core.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Build-MenuPreview.ps1 -ZigPath 'C:\tools\zig\zig.exe'
```

The normal scripts do not start RDR2. Deliberate crash fixtures under `Build.ps1 -RunChecks` are outside this validation procedure.

## Installation package

The full installer was compared with the working mod runtime tree. Its official loader matches the installed loader byte for byte. All pinned runtime binary hashes and all ZIP entry hashes were verified. The 17 binary modules are Windows x64, and their static imports resolve to packaged modules or installed Windows, driver and Visual C++ components. This is a static packaging check, not a clean-machine gameplay test.

The root contains only `dinput8.dll`, `RDR2VulkanFG.asi` and `RDR2VulkanFG.ini`, plus the runtime and license directories. The only text document in the ZIP is `licenses/NOTICES.txt`. All original notice contents were verified to be present there. No game-owned DLLs, logs, PDBs, dumps, backups or personal INIs were copied.
