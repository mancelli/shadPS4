<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# GPU readbacks: the five Readback Speed modes and the On/Off switch

- Date: 2026-10-15. Code read: `cdfb6fa^` (five modes, release 2026-10-14 = 12739d2) and
  `cdfb6fa` (On/Off, branch `beta`). Line numbers are for `beta` at d552df8.
- Method: code reading only, nothing measured. **[VERIFIED]** = traced in the code.
  **[HYPOTHESIS]** = not proven.

## 1. What a readback is

The PS4 has one memory for the CPU and the GPU: what the GPU writes, the CPU can read at once. On a
PC the emulator keeps guest memory in RAM and GPU copies of it in VRAM. When the game reads on the
CPU something the GPU wrote (compute results such as skinning or physics, pictures it saves), the
emulator must copy it back first. That copy is a readback.

How it works [VERIFIED]: the emulator protects the pages the GPU wrote. A CPU access faults; the
fault handler waits for the GPU (`scheduler.Finish()`), copies the data down and lifts the
protection, and the game reads correct data. Each such fault stops the game thread until the GPU is
idle, so many of them mean stutter and lower FPS.

## 2. What the five modes did (before cdfb6fa)

The mode was read in many places (launcher, ImGui settings, overlay, log, config, the UFC 4
override), but it changed emulation in one file only:
`src/video_core/buffer_cache/region_manager.h`, in `ChangeRegionState` and
`ForEachModifiedRange` [VERIFIED].

| Mode | `ChangeRegionState` (GPU wrote pages) | `ForEachModifiedRange` (clear) | `flushes[page]` counter |
|---|---|---|---|
| Disable | `UpdateProtection<!enable, false>` | `UpdateProtection<true, false>` | counts |
| Unsafe | `UpdateProtection<!enable, false>` | `UpdateProtection<false, false>` | counts |
| Low | `UpdateProtection<enable, true>` | `UpdateProtection<false, true>` | no |
| Default | same as Low | same as Low | counts |
| Fast | same as Low | same as Low | counts |

So there were two behaviours:

- **Disable / Unsafe:** GPU-written pages are not read-protected; nothing is copied back. The game
  reads stale memory when it reads GPU results.
- **Low / Default / Fast:** pages are read-protected and copied back on CPU access.

**Why Low, Default and Fast were the same [VERIFIED].** The only difference is the `flushes`
counter (counted for every mode except Low, `region_manager.h:113-116` at `cdfb6fa^`). Its value is returned only by `NumFlushes()`, which is called only by
`ForEachPreemptiveFlushPage()` (`memory_tracker.h:77`). That function selects pages flushed 16 or
more times (`PREEMPTIVE_FLUSH_THRESHOLD`, `memory_tracker.h:21`) to copy them before the CPU asks.
Nothing calls it: Shadlix added the counter but never wired up the preemptive flush. Default and
Fast are identical even in that design.

**Disable vs Unsafe.** Only the clear path differs: Disable puts write protection back
(`<true, false>`), Unsafe removes it (`<false, false>`). That protection tracks CPU writes; neither
mode copies anything back. No benefit of Unsafe is visible in the code.

## 3. The official shadPS4 for comparison

Upstream `main` (checked 2026-10-15) has three modes in the same files:

- **Precise:** like our On: GPU-written pages read-protected, copied on CPU read.
- **Relaxed:** copies only when the CPU *writes* to memory the GPU modified; CPU reads are not
  caught. Cheaper, catches less.
- **Disabled:** never copies.

Relaxed is a real middle ground that this fork does not have.

## 4. What the fork does now (cdfb6fa)

- One switch, **Off / On**, in Settings → Experimental, the per-game settings, the ImGui settings
  and the in-game overlay. On is the default for a new install.
- On = old Low (`region_manager.h:101-103`, `:135-136`). Off = old Disable (`:105`, `:138`).
- Config stays compatible: key `GPU.readbackSpeedMode`; `NormalizeReadbackSpeed()`
  (`config.cpp:1232`) reads 0/1 (Disable/Unsafe) as Off and 2/3/4 as On; the launcher writes 0 or
  2. `Config::readbacksEnabled()` (`config.cpp:1237`) is what the region manager asks.
- No hidden override any more (ef844c9 ran Disable/Unsafe as Low for UFC 4 without telling the
  player). Choosing Off shows a warning dialog; the log says `GPU readbacks: on/off`.

## 5. UFC 3 and UFC 4 need readbacks

- UFC 4: with Off the fighters have no heads (only hair, eyes and beard): the game reads results of
  its compute shaders for the heads on the CPU (found for ef844c9).
- UFC 3: with Off created fighters' heads break, polygons stick out of the head (reported by the
  tester, 2026-10-15).

## 6. What Off still does not skip

`MemoryTracker::InvalidateRegion` (`memory_tracker.h:91`) downloads GPU-modified pages and waits
for the GPU whenever the **CPU writes** to them, whatever the switch says [VERIFIED]. The old
BBFork build (2025-11-05) skipped that with Disable. So Off is not a full off. See
[bbfork-2025-11-05-fps.md](bbfork-2025-11-05-fps.md), item 4.

## 7. LinearImage Readbacks (separate setting)

`readbackLinearImages` downloads render targets that are linear or at most 8 pixels wide after they
are drawn (`texture_cache.cpp`, see [ufc4-startup-crash-nvidia.md](ufc4-startup-crash-nvidia.md),
"Settings"). It costs a GPU wait per download. UFC does not need it: the avatar fix (e565b66)
downloads only the linear storage images it needs and works with this setting off.

## 8. Per GPU

The code has no vendor-specific readback logic; only the cost differs [HYPOTHESIS for the size of
the cost]:

- discrete NVIDIA, AMD, Intel Arc: copies go over PCIe; on Arc, Resizable BAR matters for CPU↔GPU
  transfers (general driver advice, not measured here);
- integrated GPUs (Radeon 780M): shared RAM makes the copy cheaper, but the wait for the GPU stays.

## 9. Ideas, not done

Each needs checks in UFC 3 and UFC 4, where readbacks keep the heads intact.

1. Wire up the preemptive flush (call `ForEachPreemptiveFlushPage`), the original idea behind
   Default/Fast: fewer waits, more bus traffic.
2. Port upstream Relaxed as a middle mode.
3. A real Off: skip the flush on CPU writes in `InvalidateRegion` when readbacks are off, as BBFork
   did.
4. Readbacks only for the ranges UFC needs (the head compute results) instead of globally: most
   work, biggest gain.
