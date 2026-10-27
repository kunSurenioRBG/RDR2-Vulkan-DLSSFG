# Changelog

## 1.0.1

- Preserve host raw input and release messages, avoid menu mouse-capture ownership changes and clear overlay input on close.
- Suspend XeSS on focus loss and resume only with fresh matching frame inputs.
- Accept valid copy-source depth/motion layouts without relaxing native shader-tag requirements.
- Safely bind a unique current DLSS snapshot recorded before the next output-image acquire.
- Preserve asynchronous output FPS measurements and validate XeSS queries independently of latency status.
- Apply and save each multiplier change once; clarify the menu's restart/provider controls.
- Add bounded diagnostics and focused regressions for focus, input, frame association and recovery.
- Retain the complete clean installer and the verified 1.0 runtime DLL family.

## 1.0

- Added clickable YouTube and GitHub footer links.
- Assembled the complete clean installation bundle with the matching runtimes and official ASI loader.

- Redesigned the native RDR2 menu with a dark theme, warm red accents, live metrics and three control tabs.
- Added the kunSurenioRBG signature to the window title and footer.
- Selected the CUDA-backed NVIDIA identity when duplicate DXGI entries are present.
- Prevented repeated XeSS multiplier requests from resetting pacing history.
- Anchored XeSS 2x deadlines to the matching first generated submission.
- Preserved the native Vulkan DLSS-G route and existing XeSS interoperability.
- Unified project labels, documentation, package names and release metadata in English.
