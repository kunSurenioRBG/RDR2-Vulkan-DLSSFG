# Building version 1.0

Use Windows PowerShell, Git and Zig 0.16.0. The main ASI and core use the GNU Windows target. The modified Streamline interposer requires MSVC and a matching Microsoft C++ runtime.

## Main ASI

```powershell
.\Build.ps1 -ZigPath 'C:\tools\zig\zig.exe'
```

Missing dependencies are fetched into `.deps`, or the directory passed with `-DependencyRoot`, at these exact commits:

| Dependency | Commit |
|---|---|
| Microsoft Detours | `1dbd4ea281d73d478aa8d09a8b83fbd265f715f7` |
| Khronos Vulkan-Headers | `c46850864f4661461b0f6cb9922c058ffea4915e` |
| NVIDIA Streamline | `2122257e0fce486f91b385aa63b9a09b0a34b363` |

The script writes `.deps/rdr2-fg-1.0-build/RDR2VulkanFG.asi`, copies the ASI to the repository root, and builds the diagnostic helper. Compile-time regressions run automatically. The normal build does not launch RDR2.

`Build.ps1 -RunChecks` also contains deliberate crash fixtures; it is not needed for the normal build or the focused checks below.

## Focused regression checks and corrected core

```powershell
.\Test-Fixes.ps1 -ZigPath 'C:\tools\zig\zig.exe'
.\Build-Core.ps1 -ZigPath 'C:\tools\zig\zig.exe'
```

Run the main build first to obtain its pinned dependencies. The test script checks adapter identity, XeSS pacing and provider lifecycle with assertions enabled. The core build compiles the vendored MinHook sources and writes `.deps/rdr2-fg-custom-core-build/RTX40MFGCore.dll`.

## Native menu preview

```powershell
.\Build-MenuPreview.ps1 -ZigPath 'C:\tools\zig\zig.exe'
```

This builds the actual menu in an isolated hidden fixture and writes six PNGs to `docs/screenshots/`: three tabs at two scales. D3D11 is used only to render this fixture. The production menu retains its Vulkan renderer. Example telemetry and preview-only entry points are excluded from the production ASI.

## Modified interposer

Run `Build-Interposer.ps1` from an x64 MSVC developer shell, or pass `-PortableMsvcRoot` containing `VC/Tools/MSVC` and `Windows Kits/10`. It applies `patches/streamline-2.14.1-queue-allocation.patch` to the pinned SDK source and produces `.deps/Streamline-2.14.1/_artifacts/queuefix-build/sl.interposer.dll`.

Keep the matching production plugins and runtime files. Updating the ASI alone does not apply the interposer correction. Locally rebuilt DLLs are not vendor-signed releases.

## Packaging

Use the pinned runtime family listed in `runtime-manifest.json` and the official Ultimate ASI Loader build identified there. The clean installer contains only the loader, main ASI, default configuration, runtime binaries and a consolidated mandatory notice file.

```powershell
python tools/package_install.py --runtime-root 'C:\RDR2Runtime\RDR2VulkanFG' --loader 'C:\ASILoader\dinput8.dll' --output '.\RDR2-Vulkan-DLSSFG-v1.0-full.zip' --manifest '.\.deps\package-report.json'
```

The main build supplies the ASI and diagnostic helper. Runtime input hashes must match the pinned family; deliberate binary updates require reviewing the manifest and compatibility checks. Default INIs are generated from release settings, never copied from a player's preferences. Legal notices are preserved verbatim and consolidated in `licenses/NOTICES.txt`.

`tools/package_runtime.py` remains available for updating an external complete runtime template. `package_install.py` produces the clean release layout.

The project does not rebuild proprietary SDK runtime DLLs. Building and packaging successfully do not establish gameplay compatibility on every machine. Do not package local logs, dumps, personal INIs or credentials.
