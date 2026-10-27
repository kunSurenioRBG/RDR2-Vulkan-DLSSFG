# 1.0.1 — XeSS stability and input recovery

RDR2 Vulkan Frame Generation by **kunSurenioRBG**.

## Changes

- Preserve the game's raw keyboard/mouse input and release messages while using the menu. Feed menu mouse clicks without taking capture ownership from the game, and clear the menu's private input state when closing it.
- Suspend XeSS on focus loss, explicitly disable the SDK and require fresh matching inputs on return. The game retains control of its window and activation.
- Accept the observed depth/motion-vector layouts for the copy transport when the images support transfer-source use. Native shader tagging retains its stricter layout requirements.
- Associate a unique, current DLSS input snapshot recorded before output-image acquisition with the immediately following acquire. Stale, ambiguous and previous-focus inputs remain rejected. This corrects a source of intermittent `WAITING FOR INPUTS` transitions.
- Keep the FPS measurement through asynchronous native/generated batch changes and validate XeSS output-query results independently of unrelated latency calls.
- Apply and save multiplier changes through a single consumer. The menu now states that multiplier changes apply automatically and distinguishes restarting FG from applying a provider change.
- Add bounded focus/input diagnostics and regression coverage for these paths.

## Installation

Download **RDR2-Vulkan-DLSSFG-v1.0.1-full.zip**, close RDR2 and back up your existing mod files. Extract the ZIP beside `RDR2.exe`, preserving its directory structure. Existing users can retain their own `RDR2VulkanFG.ini` and should preserve a customized ASI loader used by other mods. Start with Vulkan and in-game DLSS enabled, then press Insert or your configured menu key.

This is a complete installation package: official ASI loader, main ASI, default INI, corrected core, matching Streamline/XeSS runtimes and diagnostic helper. The only text document inside the ZIP is the required consolidated `licenses/NOTICES.txt`. Runtime DLLs and the loader are unchanged from the full 1.0 package.

## Validation and remaining limits

The user confirmed smooth gameplay on an RTX 3080 Ti. A five-second XeSS 3x sample accepted 224/224 game frames with zero SDK-disabled batches, 223 multi-output batches and one initial history reset. The new pre-acquire association was also exercised later in the same session. The sample itself preceded that first association, so it does not prove that this ordering explained every previous interruption.

The user changed 3x → 2x → 4x → 2x. Three brief missing-recent-SR reports coincided with those multiplier changes; uninterrupted generation during reconfiguration is not claimed. A later, separate native DLSS-G session reported stable 2x and 3x SDK output ratios without query errors.

The focused regression suite covers focus generations, stale/ambiguous inputs, raw input and release forwarding, layouts, history, swapchain recreation and FPS measurement. The final release rebuild changes version strings only relative to the gameplay-tested code. Longer sessions, every Alt+Tab sequence, every GPU and clean-machine installation have not been exhaustively tested. SDK/proxy counts do not measure physical scanout or input latency. XeSS 3x/4x and unofficial GPU unlocks remain experimental.
