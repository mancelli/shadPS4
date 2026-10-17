<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# What this fork changes, commit by commit

A map for porting these fixes to a newer shadPS4 or Shadlix. State: code at d552df8
(2026-10-15; branch `mancelli-shadPS4-UFC3/4` since `beta` was merged into it on 2026-10-17),
66 commits on top of Shadlix `88bd144` (diegolix29/shadPS4, branch `Shadlix`,
2026-10-05). Every commit message explains the problem, the change and how it was checked; read
it with `git show <hash>` before porting. User-facing wording of the same changes:
[CHANGELOG.md](../../CHANGELOG.md).

"Switch" is an environment variable that turns the change off for one run (A/B testing). Most of
these fixes are on for every GPU; the ones limited to some GPUs say so.

## Graphics

| Commit | Change | Main files | Switch |
|---|---|---|---|
| 8034718 | Intel subgroups: require subgroup size 32 where the driver could pick less, LDS barriers for subgroups < 64, shared-memory barriers in divergent code, 1D compute workgroups, NaN output guard. Fixes black squares, floor flicker, shadow gaps on Arc | `vk_instance.cpp/.h`, `vk_pipeline_cache.cpp`, `shared_memory_barrier_pass.cpp`, `emit_spirv_barriers.cpp`, `profile.h` | `SHADPS4_LDS_ORDER=0`, `SHADPS4_FLATTEN_WG=0/1`, `SHADPS4_NAN_GUARD=0/1` |
| 731a29e | NaN guard and 1D workgroups also on NVIDIA (proprietary and NVK). Shader cache version 7 → 8. Untested on NVIDIA | `vk_pipeline_cache.cpp`, `vk_pipeline_serialization.cpp` | same as above |
| 02d3b06 | min/max/clamp return the non-NaN operand like GCN (NMin/NMax) | `emit_spirv_floating_point.cpp` | none |
| c08218d | Depth bias × 256 when Z16+S8 is emulated with a 24/32-bit depth format (Intel, NVIDIA): shadow acne | `vk_rasterizer.cpp` | none |
| 69ab199 | DB_ALPHA_TO_MASK (alpha to coverage, dither on single-sample targets): UFC 4 hair | `export.cpp`, `runtime_info.h`, `vk_graphics_pipeline.cpp` | `SHADPS4_ALPHA_TO_MASK=0` |
| 789551b | FMASK decompress runs the fast clear expansion; untouched MSAA images are cleared: skins showing the previous fighter | `vk_rasterizer.cpp` | `SHADPS4_FMASK_DECOMPRESS=0` |
| 01b32f4 | GPU texture compression through memory (BC view of an uncompressed image), write back of storage aliases, flush GPU images before CPU writes: grey skin in Create Fighter | `texture_cache.cpp`, `vk_rasterizer.cpp`, `storage_image_sync.cpp`, `page_manager.cpp` | `SHADPS4_BC_MODE` (bit 0 view through memory, bit 1 write back; default 3), `SHADPS4_FLUSH_CPU_WRITE=0` |
| 5b0e995 | Write back of storage aliases only when needed (PendingWriteBack): about 87 % less write-back traffic in Create Fighter | `texture_cache.cpp` | `SHADPS4_LAZY_WRITEBACK=0` |
| 630b4e7 | CMASK of read-only bound targets reported as fully written: UI visible at native and higher resolutions | `vk_rasterizer.cpp` | `SHADPS4_CMASK_EXPAND=0` |
| 84c6ede | Fault storm self-heal: 64 write faults in a row on one page make it writable and log the page state (random boot freeze) | `vk_rasterizer.cpp`, `page_manager.cpp`, `texture_cache.cpp` | none |
| 5dedb48 | `num_allocated_vgprs` gets 9 bits (256 VGPRs read as 0 after upstream #5257) | `runtime_info.h` | none |
| e565b66 | Download linear 2D storage images ≥ 64x64 written by compute: black avatars of created fighters | `texture_cache.cpp` | `SHADPS4_LINEAR_STORAGE_READBACK=0` |
| 7225129 | Depth image read with a color format gets a converted color copy: UFC 3/4 shadows in the wrong place (Arc), unlit floor (Radeon 780M) | `vk_rasterizer.cpp`, `texture_cache.cpp`, `depth_to_color.comp` | `SHADPS4_DEPTH_AS_COLOR=0` |
| cdfb6fa | Readbacks: Off/On instead of five Readback Speed modes, no hidden override for UFC 4 (replaces ef844c9). See [readbacks.md](readbacks.md) | `region_manager.h`, `config.cpp`, settings dialogs | launcher setting |
| d552df8 | FSR RCAS gets the attenuation in stops (it got 250 instead of 0.25 and did not sharpen). See [fsr-xess.md](fsr-xess.md) | `fsr_pass.cpp` | none |

## Picked from the official shadPS4 (October 2026)

| Commit | Upstream PR | Change |
|---|---|---|
| ca838c5 | #5254 | ajm: MP3 parser out-of-bounds read, Xing/VBRI headers (game audio crash) |
| 5157080 | #5257 | shader_recompiler: NaN handling |
| 5f7d8a1 | #5346 | V_RCP_CLAMP_F32, V_RSQ_CLAMP_F32, V_RSQ_LEGACY_F32, V_LOG_CLAMP_F32 |
| 914c5a1 | #5317 | address_space: restore protections after splitting a mapped region on Windows |
| 55e14c6 | #5340 | no 33 ms analog smoothing on controller sticks and triggers |
| 7e0d7b9 | #5197 | zero/one MIN/MAX blend factors |
| caf3d7c | #5220 | preserve EXEC scope after an empty branch |
| 1cc85e3 | #5260 | unbiasing for all instance id uses |
| ca46b8b | #5285 | subgroup barriers for shared memory dependencies |
| 36c9f6c | #5274 | step rates with base instance |

## Shader and pipeline cache

| Commit | Change | Switch |
|---|---|---|
| e669792 | VkPipelineCache saved to `<cache dir>/<serial>_vk_<vendor>_<device>.bin` (after preload, every 50 new pipelines, on exit); loose shader cache files written atomically | needs `pipelineCacheEnable` |
| a29dc76 | Shader patch lookup once per permutation, not on every draw | none |
| 6a70c91 | `SHADPS4_GPU=<part of device name>` picks the Vulkan device, with its own shader cache folder | `SHADPS4_GPU` |

## UFC 3 / UFC 4 specific

| Commit | Change | Main files |
|---|---|---|
| d97144a, fa3c329, 12739d2 | Render resolution patch: replaces the 1600x900 constants in the games' resolution setup (UFC 4 US CUSA14204 and EU CUSA14209, UFC 3 EU CUSA06534). Searched by bytes; the log says when not found | `memory_patcher.cpp` |
| 1a54440, ab79153 | Launcher selector for it (`GPU.renderResolution`; `SHADPS4_RENDER_RES=WxH` overrides), sizes 640x360 … 3840x2160, below 1280x720 marked experimental | `settings_dialog.cpp/.ui`, `config.cpp` |
| 7421a24 | Skip Intro Video: `open()`/`stat()` report `.../movie/bootflow/` files as missing (on by default) | `config.cpp`, `file_system.cpp` |
| 15b0af1 | FPS limiter limits the frame rate, not the vblank rate (UFC flips every second vblank, so "60" gave 30 FPS) | videoout `driver.cpp` |
| b7285a7, f0038b9 | New install: shader cache on disk, readbacks on, FPS limiter value = monitor refresh rate (limiter off). Existing configs are not touched | `config.cpp`, `common/thread.cpp` |

## Input

| Commit | Change |
|---|---|
| 428ad3b | Two gamepads: one ordering for the pressed-keys list, no more stuck triggers/buttons. See [two-gamepads-stuck-buttons.md](two-gamepads-stuck-buttons.md) |
| be60f09 | Mute hotkeys (F1, combo) also in the SDL build |

## Launcher, PKG install, updater

See [launcher-pkg-and-qt.md](launcher-pkg-and-qt.md) for the PKG and Qt plugin investigations.

| Commit | Change |
|---|---|
| 6750a4c | Qt plugin lookup ignores `QT_PLUGIN_PATH`, `QT_QPA_PLATFORM_PLUGIN_PATH` and `QT_QPA_PLATFORM`, adds `<exe>/qtplugins`; argv kept alive for QApplication |
| a971a34 | DLC (`CATEGORY=ac`) installed to `<DLC folder>/<title id>/<entitlement label>`; `gp` = update; filesystem errors shown instead of a crash |
| a3ee89a | PKG install closed the launcher (heap corruption in CryptoPP RSA): the library's ASM/alignment definitions are now INTERFACE definitions too (`externals/CMakeLists.txt`) |
| 63f4890 | PKG extraction on up to 8 worker threads, progress in bytes, Cancel; about 2.5x faster |
| d5cc667 | Launcher crash on exit: the shared IpcClient no longer has a Qt parent (was deleted twice) |
| 1a54440 | Hover descriptions for every setting, including what it costs |
| 2c718d4 | Updater: only mancelli/shadPS4, channels Main (latest release) and Beta (pre-release `beta-latest`), on by default, compares the release's target commit |

## Diagnostics

| Commit | Change | Switches |
|---|---|---|
| 1119359 | "Collect Debug Logs": GameFPS/PerfStats lines every 120 frames, freeze watchdog with all thread stacks | `SHADPS4_DEBUG_LOGS=1/0`, `SHADPS4_FREEZE_DUMP_S`, `SHADPS4_FREEZE_EXIT` |
| b89f36f | Test hooks: input record/replay, screenshot series, watched shader, PS4 Pro override | `SHADPS4_INPUT_RECORD`, `SHADPS4_INPUT_PLAY`, `SHADPS4_SCREENSHOT_SERIES`, `SHADPS4_EXIT_AT_MARKER`, `SHADPS4_CAPTURE_DELAY_MS`, `SHADPS4_WATCH_SHADER`, `SHADPS4_PS4PRO` |
| 7587972 | Windows: unhandled exceptions logged (code, address, module+offset, thread) before the process ends | none |
| ed80f53 | Windows: the log is flushed when an ASSERT ends the process (with "group identical lines" the last line was lost) | none |

## Build, CI, documents

- a9c1f99: CI builds Windows only (`windows-sdl`, `windows-qt`); Linux, macOS, Flatpak, clang-format,
  REUSE, translation and Shadlix auto-release jobs removed. 21aaee8: manual runs on `beta` also
  publish the `beta-latest` pre-release.
- 2f33dd5: formatting only.
- 64daad8 … 9a080be (README/CHANGELOG/FUNDING commits) and 1214e73 (merge): documents only.
- ef844c9: UFC 4 ran Readback Speed Disable/Unsafe as Low; replaced by cdfb6fa.

## Shader cache versions

69ab199 (cache version 7) and 731a29e (version 8) change generated shaders. A newer upstream has
its own versions; when porting, bump the cache version once more so old caches are rebuilt.
