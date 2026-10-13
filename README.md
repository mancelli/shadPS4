<!--
SPDX-FileCopyrightText: 2024 shadPS4 Emulator Project
SPDX-License-Identifier: GPL-2.0-or-later
-->

# **⚠️ ALL CHANGES IN THIS FORK WERE MADE ENTIRELY WITH AI (CLAUDE BY ANTHROPIC) AND TESTED BY A HUMAN.**

# **⚠️ ВСЕ ИЗМЕНЕНИЯ В ЭТОМ ФОРКЕ ПОЛНОСТЬЮ СДЕЛАНЫ С ПОМОЩЬЮ ИИ (CLAUDE ОТ ANTHROPIC) И ПРОВЕРЕНЫ ЧЕЛОВЕКОМ.**

# UFC 4 / Intel Arc fork of shadPS4

A fork of [Shadlix (diegolix29/shadPS4)](https://github.com/diegolix29/shadPS4), which is itself a fork of the
[shadPS4](https://github.com/shadps4-emu/shadPS4) PS4 emulator. It fixes rendering and gameplay problems found
while getting **EA SPORTS UFC 4 (CUSA14204)** to run on an **Intel Arc B580**, and includes selected fixes from
the official shadPS4.

> [!WARNING]
> **Windows + Intel Arc only.** Everything was written and tested on **Windows 11 with an Intel Arc B580** (a few
> checks on a Radeon 780M) with UFC 4. The Linux and macOS builds are made automatically by CI and are **not tested
> and not supported** (their release files start with `UNTESTED`). Other games were not tested.
>
> **NVIDIA:** the Intel workarounds against black squares are also enabled on NVIDIA GPUs. This is **untested on
> NVIDIA hardware**; please report results with **Collect Debug Logs** enabled (see below).

## How to use

1. Download the Windows zip from [Releases](../../releases):
   - `shadPS4-ufc4-intel-arc-windows-qt-launcher.zip`: with the launcher (recommended);
   - `shadPS4-ufc4-intel-arc-windows-sdl.zip`: emulator only, start a game with `shadPS4.exe -g "<path>\eboot.bin"`.
2. Unpack **the whole zip** into a new folder and run `shadPS4.exe`. All fixes are on by default.

**UFC 4 at native 1080p.** The game renders its 3D scene at 1600x900 and scales it up. To render at 1920x1080,
start the game with `UFC4_1080p.bat` from the zip (in the SDL zip, put your `eboot.bin` path into the file first),
or set the environment variable yourself:

```bat
set SHADPS4_RENDER_RES=1920x1080
shadPS4.exe -g "C:\path\to\CUSA14204\eboot.bin"
```

The first start after an update recompiles shaders, so the game stutters once.

## If something goes wrong

- **"No Qt platform plugin could be initialized":** unpack the whole zip into a new folder (do not run the exe
  from inside the zip or copy it alone over an older version). If it still happens, remove the
  `QT_PLUGIN_PATH` and `QT_QPA_PLATFORM_PLUGIN_PATH` environment variables left by other programs.
- **Any other problem:** in the launcher turn on **Settings → Logger → Collect Debug Logs** (without the launcher:
  `CollectDebugLogs = true` under `[Debug]` in `config.toml`), play until the problem happens, then send the log
  file (**Open Log Location**). Turn the option off again afterwards.

## Fixes

### Release 2026-10-11.2

**UFC 4 status: career mode is fully playable, and the hair bug is fixed.**

| Problem | Fix |
|---|---|
| Black squares on NVIDIA GPUs (reported by users) | The Intel workarounds (NaN guard on shader outputs, 1D compute workgroups) are also enabled on NVIDIA (untested) |
| Created fighter avatars are black | Linear images written by compute shaders are copied back to memory, so the game can save them. Avatars saved with older builds stay black until the fighter is saved again |
| Shaders that use all 256 vector registers break | The register count is stored in 9 bits instead of 8 |

From the official shadPS4 (October 2026): NaN handling, RCP/RSQ/LOG clamps, MP3 parser crash fix, subgroup barriers
for shared memory, Windows memory protection after a region split, MIN/MAX blend factors, EXEC scope after an empty
branch, instance id unbiasing and step rates with base instance, analog stick smoothing fix.

### Release 2026-10-11 (first release)

| Problem | Fix |
|---|---|
| Black squares on models, flickering floor, gaps in shadows (Intel) | Subgroup size 32, 1D compute workgroups, LDS ordering barriers, NaN guard on shader outputs |
| Wrong results of min/max/clamp with NaN | NaN-aware min/max/clamp that matches the PS4 GPU |
| Shadow acne | Depth bias scaled for the emulated D16S8 depth format |
| Flat, hard-edged hair | Alpha to coverage (DB_ALPHA_TO_MASK) is emulated |
| Create Fighter: pieces of other skins after choosing a preset | Fast clears are expanded on FMASK decompress |
| Create Fighter: grey skin | GPU-compressed textures go through memory; GPU images are flushed before CPU writes |
| Stutter and heavy memory traffic while editing a fighter | Lazy write-back of storage image aliases (about 87 % less traffic) |
| Random freeze: the picture stops while sound keeps playing | Self-heal for pages that stay write-protected without an owner |
| Menus invisible at native resolution | Shaders that read CMASK see every tile as written |
| Shader compile stutter on every start | Vulkan pipeline cache kept on disk; shader cache writes are atomic; shader patches checked once per shader |
| UFC 4 renders at 1600x900 | `SHADPS4_RENDER_RES=WxH` changes the UFC 4 render resolution |

Also added: **Collect Debug Logs** option (frame times, performance counters, thread dump on freeze), F1 mutes the
SDL build, `SHADPS4_GPU=<part of the GPU name>` picks the GPU for one run.

### Beta (not released yet)

These changes are in the [`beta`](../../tree/beta) branch. Its builds are on the
[Actions](../../actions?query=branch%3Abeta) page (sign in to GitHub to download) and are not tested in the game yet.

| Problem | Fix |
|---|---|
| Two players: one player's block (or another button) stays held until the game restarts | Inputs from both controllers are kept in one consistently sorted list, so a released button can no longer stay stuck |
| "No Qt platform plugin could be initialized" on some PCs | The launcher ignores Qt plugin paths left by other programs and always loads its own plugins |
| Unclear settings | Every setting in the launcher describes what it does and what it costs in performance |
| Render resolution only through a `.bat` file | **Settings → Graphics → Render Resolution (UFC 4)** |

## More information

- [Shadlix README](https://github.com/diegolix29/shadPS4/blob/Shadlix/README.md): launcher features, Bloodborne
  options and other features of the Shadlix fork.
- [shadPS4 README](https://github.com/shadps4-emu/shadPS4/blob/main/README.md): the original emulator, building
  from source, firmware files, keyboard mappings, the shadPS4 team and contributors.

## License

[GPL-2.0-or-later](LICENSE), like shadPS4.
