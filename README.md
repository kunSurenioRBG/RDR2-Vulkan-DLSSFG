# RDR2-Vulkan-DLSSFG

**Version 1.0.1 · kunSurenioRBG · Red Dead Redemption 2 only**

Frame generation for Red Dead Redemption 2 while the game renders with Vulkan. Version 1.0.1 improves XeSS focus recovery, game input, frame association and output telemetry.

![Native in-game menu rendered in an isolated preview with example telemetry](docs/screenshots/menu-generation.png)

## Features

- Native Vulkan DLSS-G path with matching game inputs.
- XeSS frame generation through the existing internal Vulkan/D3D12 interoperability path.
- CUDA-backed NVIDIA adapter selection to handle certain duplicate DXGI identities.
- Cached XeSS multiplier changes and corrected 2x presentation timing.
- Fresh-input recovery after Alt+Tab, with host keyboard and mouse input preserved.
- Safe association of DLSS input snapshots recorded before the next output-image acquire.
- Optional bounded diagnostics, disabled by default.

This project targets RDR2. Experimental multipliers and unofficial GPU unlocks retain their existing limitations.

## Download and install

### 1. Check the requirements

| Requirement | What to check |
|---|---|
| Game and operating system | An installed copy of Red Dead Redemption 2 and Windows 10/11 x64. The bundled DLSS-G integration requires at least Windows 10 version 2004, build 19041. |
| Graphics card | An NVIDIA RTX GPU on which RDR2's own DLSS Super Resolution works. The development gameplay tests used an RTX 3080 Ti; other GPUs are not equally validated. Unofficial unlocks and maximum multipliers depend on the GPU and runtime. |
| Graphics driver | A compatible NVIDIA driver with working Vulkan and DLSS support. No single minimum driver version has been validated for every GPU in this project. |
| Graphics API | **Vulkan** in RDR2. |
| In-game upscaler | **NVIDIA DLSS enabled**, including when the mod's frame-generation provider is **XeSS FG**. The mod captures the game's DLSS depth/motion inputs; XeSS FG does not replace this requirement. |
| Windows runtime | Install or repair the [Microsoft Visual C++ v14 Redistributable, x64](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist). This is a runtime installer; Visual Studio is not required. |
| GPU scheduling | For DLSS-G, enable **Hardware-accelerated GPU scheduling (HAGS)** in Windows and restart the PC. See the steps below. |

For HAGS, open **Windows Settings → System → Display → Graphics**, then open **Default graphics settings** or **Advanced graphics settings** (the label varies with Windows). Enable **Hardware-accelerated GPU scheduling**, then restart Windows. If the option is unavailable, check your Windows/driver/GPU support; an INI edit cannot enable an unsupported Windows feature. The Windows build and HAGS requirements are documented in the [bundled Streamline version's DLSS-G guide](https://github.com/NVIDIA-RTX/Streamline/blob/2122257e0fce486f91b385aa63b9a09b0a34b363/docs/ProgrammingGuideDLSS_G.md#10-requirements).

Also check **Windows Settings → System → Display → Advanced display** and select the intended monitor refresh rate. A higher output counter alone cannot make a 60 Hz display show 120 distinct frames per second.

The default `CameraSource=Neutral` does not use ScriptHookRDR2 camera data. **ScriptHookRDR2 is not required for this default setup**, and the ASI loader is already included. Keep `EnableExperimentalRTX20=0` for the standard setup; RTX 20 support is a separate experimental opt-in, not a confirmed compatibility promise.

### 2. Download the complete release

Download [RDR2-Vulkan-DLSSFG-v1.0.1-full.zip](https://github.com/kunSurenioRBG/RDR2-Vulkan-DLSSFG/releases/download/v1.0.1/RDR2-Vulkan-DLSSFG-v1.0.1-full.zip) from the [1.0.1 release](https://github.com/kunSurenioRBG/RDR2-Vulkan-DLSSFG/releases/tag/v1.0.1).

Choose the asset ending in **`-full.zip`**. GitHub's automatic **Source code (zip)** and **Source code (tar.gz)** downloads contain source files, not a ready-to-install runtime bundle. The `.sha256` asset is an optional download-integrity checksum, not an installer.

The complete ZIP includes the official ASI loader, main ASI, default INI, corrected core, matching Streamline/XeSS runtimes and diagnostic helper. Runtime identities are pinned in [runtime-manifest.json](runtime-manifest.json); the exact 20-file release inventory is in [docs/INSTALL-FILES.json](docs/INSTALL-FILES.json).

### 3. Back up and extract beside RDR2.exe

1. Save your game and **close RDR2 completely** before installing or replacing mod files.
2. Open the installation directory containing the actual **`RDR2.exe`**. In Steam, use **RDR2 → Properties → Installed Files → Browse**. A typical directory is `...\SteamLibrary\steamapps\common\Red Dead Redemption 2\`. For another launcher, locate that launcher's installed game directory.
3. Back up any existing `RDR2VulkanFG.asi`, `RDR2VulkanFG.ini`, `RDR2VulkanFG/` directory and `dinput8.dll` before replacing them.
4. If another mod already supplies `dinput8.dll`, preserve that file and check its ASI-loading compatibility before replacing it. The filename alone does not identify the loader, and other mods may depend on it.
5. Extract the **contents** of the full ZIP directly into the directory containing `RDR2.exe`, retaining all subdirectories. Do not leave the files inside an extra `RDR2-Vulkan-DLSSFG-v1.0.1-full` folder, and do not put them in `Documents\Rockstar Games` or a generic `mods` folder.

The resulting layout should be:

```text
Red Dead Redemption 2/
├── RDR2.exe                         # Existing game executable
├── dinput8.dll                      # Included ASI loader
├── RDR2VulkanFG.asi
├── RDR2VulkanFG.ini
├── RDR2VulkanFG/
│   ├── RDR2VulkanFGDiagnostics.dll
│   ├── Core/
│   │   ├── RTX40MFGCore.dll
│   │   └── RTX20/                   # Included; experimental path disabled by default
│   ├── Streamline/                  # Keep the entire supplied directory
│   └── XeSS/                        # Keep the entire supplied directory
└── licenses/
    └── NOTICES.txt
```

Keep the supplied DLLs together in their original locations. Copying only the ASI is not a complete installation. Keep the game's existing `nvngx_dlss.dll`; replacing that Super Resolution DLL is not part of this installation. Avoid combining this package with another FG/upscaler injector for the first test: another loaded Streamline interposer is explicitly rejected by this mod. Back up and isolate competing mod files rather than deleting unknown or game-owned DLLs.

### 4. Configure RDR2

Launch the game through your usual launcher and open its graphics settings:

| Setting | Initial setup |
|---|---|
| Graphics API | **Vulkan**. It is in the advanced graphics settings; unlock those settings if necessary. Apply the change and restart the game when requested. |
| NVIDIA DLSS | **Enabled**. Start with **Quality** as a visual baseline. |
| Resolution | Your intended display/output resolution. Reduce GPU-heavy settings or change DLSS quality later if the base game frame rate is too low. |
| VSync | For an initial diagnostic baseline, use **Off**. This is a starting configuration, not a universal VRR/VSync recommendation; evaluate synchronization separately after FG is working. |
| Other graphics settings | Start from a configuration that already runs steadily with FG disabled. Keep resolution/display-mode changes separate from the initial FG test. |

**DLSS Super Resolution and frame generation are different settings.** DLSS in RDR2 supplies the rendered image and inputs; the mod menu chooses **DLSS-G** or **XeSS FG** to generate additional frames. Turning off RDR2's DLSS, or using its TAA/FSR path instead, does not provide the input path required by this build.

Load an actual save and wait for gameplay before judging FG. Loading screens and menus may not provide the same DLSS inputs as the rendered world. As a practical starting target, aim for a stable base rate around 50–60 rendered FPS before increasing the multiplier; this is a tuning guideline, not a hard requirement or a guaranteed result. Generated frames do not multiply the game's simulation/input update rate.

### 5. Configure the mod

Press **Insert** in the loaded game to open the mod. If you retained an older INI, your saved `MenuKey` takes precedence.

1. In **Generation**, enable **Enable frame generation**.
2. Start with **Provider: DLSS-G** and **Multiplier: 2x**. The default INI uses the native Vulkan DLSS-G path.
3. Close the overlay with Insert and check camera movement and normal controls.
4. To test **XeSS FG**, select it in **Provider**. When the menu says **Restart game to apply XeSS FG**, save, close and relaunch RDR2. Switching between native DLSS-G and XeSS requires a full game restart; the **Restart FG** button does not replace it. Keep the game's API on Vulkan and its DLSS enabled.
5. Once 2x is stable, test the higher multipliers exposed by the runtime. **XeSS 3x/4x are experimental.** Change one option at a time and allow the provider to recover before judging it. Multiplier changes apply automatically; do not repeatedly press Restart FG to apply them.
6. In **Display**, leave **FPS limit: 0 / Unlimited** for the first test and enable **Show FPS counter**. Tune a single limiter afterward if needed; avoid adding several competing limits while diagnosing smoothness.

The game remains on **Vulkan** with either provider. XeSS uses the mod's existing internal Vulkan/D3D12 interoperability; selecting XeSS does not mean changing RDR2's API to DX12.

For an initial manual setup, keep these values in the existing `[fgvk]` section of **`RDR2VulkanFG.ini`**:

```ini
[fgvk]
Enabled=1
FGProvider=DLSSG
DLSSGBackend=Native
DLSSGFrames=1
UserFGOff=0
CameraSource=Neutral
ShowFPS=1
FPSCap=0
OutputFPSCap=0
ReflexMode=1
ReflexSleep=0
NativeHUDProtection=1
AntiTearing=0
AntiGhostHistory=1
EnableExperimentalRTX20=0
DebugLogging=0
Diagnostics=0
IntervalLogging=0
```

This is a **subset of the supplied INI**, not a replacement for the whole file. Edit it only with RDR2 closed, and do not duplicate the `[fgvk]` section. To start with XeSS, change only `FGProvider=XeSS` and then launch the game. `DLSSGBackend=Native` controls the DLSS-G selection; it does not turn XeSS into a native Vulkan SDK implementation.

| INI setting | Meaning |
|---|---|
| `DLSSGFrames=1` | One generated frame per rendered frame: requested **2x** output. `2` requests 3x; `3` requests 4x. Available values depend on runtime capability. |
| `UserFGOff=0` | The user's FG toggle is on. `1` turns it off even if the mod is loaded. |
| `FPSCap=0` | No base/rendered-frame cap from this setting. The menu's FPS limit controls this value. |
| `OutputFPSCap=0` | Leave the advanced output-target cap disabled for the baseline. |
| `ReflexSleep=0` | Keep the release's native Vulkan wait setting. This does not control XeSS pacing. |
| `NativeHUDProtection=1` | Keep the release's experimental native HUD protection enabled. It is not a promise of artifact-free UI. |
| `AntiGhostHistory=1` | Keep input-history recovery enabled. |
| Diagnostic settings | Keep all three at `0` for normal gameplay. Basic errors/recovery evidence are still written to the normal log. |

Leave the other supplied settings, including preset and camera options, unchanged for the first run. The menu saves ordinary setting changes; manually editing the INI while RDR2 is open can be overwritten by those saves.

### 6. Verify generation and smoothness

- Check **Generation → GENERATING**, the active provider in **Diagnostics**, and **Game FPS** versus **Output FPS / SDK**. With a stable 2x request, output should tend toward twice the rendered rate once the provider has valid history; it need not match exactly during startup or recovery.
- Move the camera and walk for at least a minute. Compare the same scene with FG on and off using **Numpad \***. Check keyboard/mouse response as well as the counter.
- For XeSS, a brief **WAITING FOR INPUTS** during loading, focus recovery or multiplier reconfiguration can occur. Repeated transitions during steady gameplay, prolonged waiting, or loss of input should be investigated rather than treated as successful generation.
- Check Alt+Tab and a single multiplier change after the basic test is stable. Return to gameplay and allow a short recovery period. The menu being open does not itself pause frame generation.

The output counter reports SDK/proxy submissions, not verified physical display refreshes. For example, 50 rendered FPS with 100 reported output FPS still has a base input/simulation rate near the rendered rate. Smoothness also depends on frame pacing and the monitor; a larger number alone is not proof that every generated frame reaches the display evenly.

### 7. Troubleshooting

| Symptom | Checks and next step |
|---|---|
| Insert does not open the menu | Verify the files are beside the actual `RDR2.exe`, the game uses Vulkan, and your ASI loader loads this ASI. Check the saved `MenuKey` in an older INI. Look for `RDR2VulkanFG.log` beside the executable; no new log suggests a loading/location problem. |
| Persistent `WAITING FOR INPUTS` or FG-104/input errors | Confirm in-game DLSS is enabled and test in the loaded world. Verify version 1.0.1 or later and the complete matching runtime bundle. If errors continue, record the exact code and log; do not force unverified resource/layout settings. |
| FG-201 / FG-202 / provider initialization or settings errors | Check **Error details and next steps**, HAGS for DLSS-G, driver compatibility and the complete `Core`, `Streamline` and `XeSS` directories. Restore the DLL family from the same full release if files have been mixed. A loaded core alone does not prove the provider initialized successfully. |
| Provider selection has no effect | Follow the menu's restart message. Changing between native DLSS-G and XeSS requires closing and reopening the game. |
| High output FPS but poor motion or delayed input | Compare base FPS and frame times with FG off; check the monitor refresh rate, conflicting limits, synchronization and overlays. Return to 2x and a stable base configuration. More generated frames do not repair a low or irregular simulation rate. |
| Input fails after Alt+Tab or multiplier changes | Close the overlay with Insert, save if possible, and restart the game. Confirm the current full package is installed. If reproducible, report the exact sequence and both logs described below. Version 1.0.1 includes fixes for these paths, but does not establish every sequence is fault-free. |
| The game fails to start after installation | Restore your backed-up mod/loader files to isolate the change. Check the Visual C++ x64 runtime and conflicting injectors. Keep game-owned DLLs intact; use the launcher to verify game files if those were previously replaced. |

For a report, include the mod release, GPU, driver version, Windows version/HAGS setting, game API, DLSS quality, FG provider, multiplier and reproduction steps. Attach **`RDR2VulkanFG.log`** from beside `RDR2.exe`; also retain **`RDR2VulkanFG.previous.log`** if you restarted after the failure. Review logs for personal paths before sharing them. Detailed logging is optional and should be enabled only for a targeted capture, then disabled again.

### Updating or uninstalling

**Update:** close the game, back up the current mod, and extract the new full release in the same layout. Keep your personal INI if you want your settings, then compare it with the new defaults and release notes. Update the ASI and matching runtime bundle together. A retained provider/backend selection takes precedence over the new-install defaults.

**Reset settings:** with the game closed, rename your current INI as a backup and copy the default INI from the release. This restores the release's native DLSS-G 2x baseline and default Insert menu key.

**Uninstall:** close RDR2, then remove this mod's `RDR2VulkanFG.asi`, `RDR2VulkanFG.ini` and dedicated `RDR2VulkanFG/` directory, or restore your pre-install backup. Remove/restore `dinput8.dll` only if it was installed solely for this mod; other ASI mods may need it. Leave the game's own DLLs and files used by other mods intact. The optional FG logs can be archived or removed separately.

## Controls

| Control | Action |
|---|---|
| Insert | Open or close the menu; existing custom keys are preserved. |
| Numpad * | Toggle frame generation. |
| End | Cycle the multiplier when supported. |

Provider availability and maximum multipliers depend on the selected runtime and GPU. XeSS 3x/4x remain experimental.

## Build and validation

See [build instructions](docs/BUILD.md), [validation](docs/VALIDATION.md), [release notes](docs/RELEASE-1.0.1.md) and [implementation notes](docs/MAINTAINER-NOTES.md).

The fixes were tested on an RTX 3080 Ti, and the user confirmed smooth gameplay. A five-second XeSS 3x sample accepted all 224 game frames, with zero SDK-disabled batches and one initial history reset. Brief recovery can still occur when changing the multiplier. Output telemetry measures SDK/proxy submissions rather than physical scanout. Menu preview images use example telemetry. See validation for the exact test scope.

## License

The required copyright notices and third-party terms remain in [LICENSE.txt](LICENSE.txt), [ACKNOWLEDGEMENTS.txt](ACKNOWLEDGEMENTS.txt) and `licenses/`. The interface signature identifies this edition; it does not replace the historical authorship of the underlying code.

## Links

[YouTube](https://www.youtube.com/@kunSure%C3%B1o) · [GitHub](https://github.com/kunSurenioRBG/RDR2-Vulkan-DLSSFG)
