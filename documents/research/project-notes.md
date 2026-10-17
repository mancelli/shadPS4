<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# How this fork is run: branches, builds, releases, open problems

State on 2026-10-17.

## Origin

- Base: Shadlix (diegolix29/shadPS4, branch `Shadlix`, commit `88bd144` of 2026-10-05), itself a
  fork of shadPS4. The launcher, Big Picture, readback settings and most of the UI are Shadlix code;
  about ten fixes were picked from the official shadPS4 on top (see
  [fork-changes.md](fork-changes.md)).
- Shadlix `11f2e97` (seen in player logs) has the same source as `88bd144`; only
  `externals/CMakeLists.txt` differs.
- Target: EA SPORTS UFC 4 (CUSA14204 US, CUSA14209 EU) and UFC 3 (CUSA06534 EU) on Windows. Tested
  on an Intel Arc B580, partly on a Radeon 780M; NVIDIA not tested.
- All changes were made with an AI assistant (Claude) and tested by a human; the README says so at
  the top.

## Branches

| Branch | Role |
|---|---|
| `mancelli-shadPS4-UFC3/4` | Default branch, state of the latest release (2026-10-14, `12739d2`). Formerly `ufc4-intel-arc` |
| `beta` | Everything not released yet. New work goes here |
| `Shadlix` | Mirror of the upstream base, not touched |

## Builds (CI)

`.github/workflows/build.yml`, Windows only:

- `windows-sdl` and `windows-qt` (Qt 6.9.3, `windeployqt`, `dist/qt.conf` copied next to the exe).
- Configure and build exactly as CI does (Visual Studio 2022 build tools, clang-cl, Ninja):

  ```
  cmake --fresh -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_QT_GUI=ON -DENABLE_UPDATER=ON -DCMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE=ON -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
  cmake --build build --config Release
  ```

  Drop `-DENABLE_QT_GUI=ON -DENABLE_UPDATER=ON` for the SDL-only build. Tool setup:
  [building-windows.md](../building-windows.md).
- Pushes that change only `documents/**` or `*.md` do not start CI. A commit with `[skip ci]` in
  its message does not either.
- A push to `beta` (or a manual run on `beta`) replaces the pre-release `beta-latest` with the new
  build (`shadPS4-beta-windows-qt-launcher.zip`); its description is the "Beta" section of
  CHANGELOG.md.

## Releases and the updater

- The launcher's updater (`src/qt_gui/check_update.cpp`) reads only mancelli/shadPS4: channel Main =
  latest release (`/releases/latest`), channel Beta = pre-release `beta-latest`. It compares the
  release's target commit with the build's commit, so create releases **from a commit**, not from a
  branch name.
- The asset name must contain `windows-qt` (for example
  `shadPS4-ufc4-intel-arc-windows-qt-launcher.zip`).
- Release notes: the version's section of CHANGELOG.md (English, then Russian).

## Rules kept in this repository

- README: the AI notice on top, then only usage and the main fixes; full list in CHANGELOG.md;
  links to the Shadlix and shadPS4 READMEs instead of their text.
- Model names are not written into repository files or commit messages.
- Claims in documents are checked against the code; guesses are marked as hypotheses.

## Open problems

1. **UFC 4 crashes at start on NVIDIA** (GTX 1660 SUPER, RTX 4060 Ti; logs from Shadlix 11f2e97).
   Needs a crash log from a build of this fork (it logs `Unhandled exception ...`) or the Windows
   Event Viewer entry (Application, event 1000, `shadPS4.exe`). Analysis:
   [ufc4-startup-crash-nvidia.md](ufc4-startup-crash-nvidia.md).
2. **FPS lower than BBFork 2025-11-05** for one player. Candidates: synchronous image downloads,
   Off still flushing on CPU writes, texture GC eviction never running, depth-as-color conversion
   on every bind: [bbfork-2025-11-05-fps.md](bbfork-2025-11-05-fps.md).
3. **Not tried in the games:** render resolutions below 1280x720 (ab79153), RCAS fix (d552df8),
   readbacks On/Off (cdfb6fa) with Off in UFC 3.
4. **Ideas:** sharpening without upscaling for low resolutions, preemptive flush or upstream
   Relaxed readbacks ([readbacks.md](readbacks.md), [fsr-xess.md](fsr-xess.md)).

## Files removed on 2026-10-17

Packaging and documents for platforms and services this fork does not use. They are still in the
git history and in the official shadPS4 repository:

- Nix (`flake.nix`, `flake.lock`), Flatpak manifests (`net.shadps4.shadPS4.yaml`,
  `net.shadps4.shadPS4.ci.yaml`), Crowdin (`crowdin.yml`);
- `documents/building-linux.md`, `building-macos.md`, `building-docker.md`, `Docker Builder/`;
- `documents/Screenshots/` (upstream screenshots) and `documents/changelog.md` (upstream changelog
  up to v0.4.0, 2024);
- `scripts/` (NID name list and its generator; the generated `aerolib.inl` is kept in
  `src/core/aerolib/`);
- `.github/ISSUE_TEMPLATE/` (upstream forms that sent reports to the shadPS4 Discord) and
  `CONTRIBUTING.md` (upstream's rules for pull requests).

Kept although Windows does not need them: `CMakeLinuxPresets.json`, `CMakeDarwinPresets.json`,
`dist/` and `.github/shadps4.png`. `CMakeLists.txt` and `CMakePresets.json` reference them, so the
source tree still configures on Linux and macOS.
