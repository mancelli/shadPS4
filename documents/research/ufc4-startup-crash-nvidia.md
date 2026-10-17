<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# UFC 4: intermittent crash at game start after the first Thick1DThick detile

> **Status 2026-10-17.** Unsolved: no log from this fork's build has arrived yet. Done since the
> report: unhandled exceptions are logged on Windows (7587972) and the log is flushed when an
> ASSERT ends the process (ed80f53), so the next crash log from a build of this fork names the
> faulting module, address and thread. Readback Speed is now an On/Off switch (cdfb6fa): "Fast"
> in the text below is "On" today. Line numbers refer to `beta` at 7587972, see the commit
> history for the current lines. Raw player logs are not published; "run 1-5" are the five
> game starts in two player logs (NVIDIA GTX 1660 SUPER and RTX 4060 Ti).

- Date: 2026-10-13
- Game: EA SPORTS UFC 4 (CUSA14204, CUSA14209)
- Repository analysed: this fork, branch `beta` (HEAD 7587972), read only.
- **The logs come from the upstream Shadlix build 11f2e97 (remote diegolix29/shadPS4,
  branch Shadlix), not from this fork.** 11f2e97 differs from our fork point 88bd144 only in
  `externals/CMakeLists.txt` (a clang warning flag): commits 173f7e3 and 78e11db revert and
  re-apply the "face gen hack", so the net source change is zero. Every code path in the logs
  is therefore exactly the 88bd144 code in this repository's history. Line numbers below are
  for `beta` HEAD unless marked `@88bd144`.

Labels used: **[VERIFIED]** = I read the code path (or checked it with a simulation or against
the logs). **[HYPOTHESIS]** = consistent with the evidence but not proven.

---

## 1. Evidence

### 1.1 Runs

| Run | Source file (lines) | GPU / driver | OS | Last lines of the log |
|---|---|---|---|---|
| 1 | c639de30 (8-917) | GTX 1660 SUPER / 576.88 | Win10 | `Thick1DThick_32 detiler`, then one SocketRecv `sceNetEpollCreate` |
| 2 | c639de30 (918-1827) | GTX 1660 SUPER / 576.88 | Win10 | `Compiling compute pipeline 0xfca013e622ea8ab` (no detiler line) |
| 3 | c639de30 (1828-2749) | GTX 1660 SUPER / 617.42 | Win10 | compile, 1 SocketRecv pair, `detiler`, 1 pair, `sceNetEpollCreate` |
| 4 | c639de30 (2750-3667) | GTX 1660 SUPER / 617.42 | Win10 | `detiler`, 1 pair, `sceNetEpollCreate` |
| 5 | c358d0e8 (1-927) | RTX 4060 Ti / 617.14 | Win11 | `detiler`, `sceNetEpollCreate` |

Common to all runs, in order: SimThread creates `/temp0/replayPS4`; the `/app0` thread opens
`/temp0/replayPS4/RenderLowFidelity.replay` with flags 0x602 (O_RDWR|O_CREAT|O_TRUNC);
GpuCommandProcessor logs `Compiling cs shader 0x480cc419`, `Compiling compute pipeline
0xfca013e622ea8ab`, `Creating tiling pipeline Thick1DThick_32 detiler`
(tile_manager.cpp:172). Before that the game finished loading (`scePlayGoClose`,
`sceSysmoduleUnloadModule`). No tiler (`... tiler`) pipeline is created in any run.

Settings in every run: LogType sync, isIdenticalLogGrouped true, readbackSpeed Fast,
readbackLinearImages true, directMemoryAccess false, crashDiagnostics true
(VK_LAYER_LUNARG_crash_diagnostic loaded), vkValidation false, game-specific config present.
dumpShaders true only in run 5. Win10 runs log "Older Windows version detected, reducing user
max to 0x10000000000". The GTX 1660 SUPER user changed driver 576.88 → 617.42 between runs
and still crashed, so the crash does not depend on one driver version.

### 1.2 Reading the end of these logs [VERIFIED]

- `isIdenticalLogGrouped=true` makes the logger hold the newest entry in `_last_entry` and
  write it only when a *different* message arrives (src/common/logging/backend.cpp:216-246).
  **The final message before a crash is always missing from the log.**
- `ASSERT`/`ASSERT_MSG` end in `assert_fail_impl()` (src/common/assert.cpp:18-22), which does
  not call `Common::Log::Stop()` (unlike `unreachable_impl()` at :24-27). With grouping on,
  the "Assertion Failed!" text is exactly the held entry, so **an assert also leaves a log
  that just stops.** Our fork's new unhandled-exception filter (src/core/signals.cpp:71-104,
  installed at :177) returns early for `EXCEPTION_BREAKPOINT` (:76-79) without
  `Common::Log::Stop()`, so asserts are still invisible in this fork.
- The log file backend flushes only for level >= Error (backend.cpp:82-89). The posted logs
  include the `;#IPC_...` stdout lines, so they are the stdout capture, which is flushed after
  every sync write (backend.cpp:229-233, 261-265).
- The SocketRecv thread always logs `sceNetEpollCreate` (Info) immediately followed by
  `sceNetEpollControl` (Warning). In runs 1, 3, 4 and 5 the last visible line is `Create`, so
  the held entry is its `Control` pair and **no thread logged anything after that** — no
  "Assertion Failed!", no "Device lost", no Unreachable, no new shader. The process died
  silently: an unhandled access violation, a fast-fail (/GS cookie, heap corruption,
  abort/terminate) or a crash inside a DLL.
- In run 2 the held entry is most likely the detiler line itself (crash before the next
  message, with no SocketRecv iteration in between).
- Timing: about 12 GPU compile events fit into one SocketRecv period earlier in run 1
  (log lines ~857-893), so one period is long (roughly 0.1-0.6 s or more). In runs 3 and 4 at
  least one full period passed after the detiler line with no GPU-thread log at all: the GPU
  thread was alive but silent (busy or blocked) before the process died.
- No `Kernel.Vmm` lines (map, munmap, mprotect and release all log at Info:
  src/core/libraries/kernel/memory.cpp:114, :399, :843) appear in the last ~350-900 lines of
  any run. Unmap-driven paths (texture cache `UnmapMemory` from guest threads, the Windows
  region split) were not active when it crashed.
- No `file read errored` (src/common/io_file.h:185-190) and no
  `Coercing copy ...` warnings (vk_runtime.cpp:40-74) appear.

---

## 2. Code path: first use of a Thick1DThick 32-bit image by compute shader 0x480cc419

1. `Rasterizer::DispatchDirect` (vk_rasterizer.cpp:395) → `PipelineCache::GetComputePipeline`
   logs "Compiling compute pipeline" (vk_pipeline_cache.cpp:376 @88bd144) **before** the
   driver compile. Then `BindResources` (vk_rasterizer.cpp:520) runs **`BindBuffers` first, then
   `BindTextures`** (:538-546).
2. `BindTextures` (:953): checks only `IsValidGpuMapping(tsharp.Address(), 0)` (:973, start
   address only) → `ImageDesc` → `ImageInfo(const AmdGpu::Image&, ...)`
   (image_info.cpp:118-145): Color3D → `is_volume`, `size.depth = depth+1` (:133),
   `resources.layers = NumLayers()` = 1 for 3D (resource.h:225-240),
   `levels = last_level+1` ≤ 16 (resource.h:242-247), then `UpdateSize()`.
3. `UpdateSize` (image_info.cpp:153-203): Array1DTiledThick is not macro-tiled, so the micro
   path is used with `mip_thickness = min(4, 4)` (:186) and
   `ImageSizeMicroTiled(mip_w, mip_h, 4, 32, 1)` (tile.h:315-326: pitch/height aligned to 8,
   pitch grown until `slice_bytes*4 % 256 == 0`). Slices are padded to a multiple of 4
   (:197-199), `size *= num_slices`, offsets accumulate into `guest_size`.
4. `TextureCache::FindImage` (texture_cache.cpp:828): new image →
   `Image::Image` (image.cpp:128-205): VK 3D image, flags MutableFormat | ExtendedUsage |
   2DArrayCompatible | 2DViewCompatibleEXT, usage includes storage and color attachment. Initial
   flags = `Dirty` = MaybeCpuDirty | CpuDirty | **GpuDirty** (image.h:143).
   Shadlix-only: `rt_sync_.CopyFromLastRt(...)` right after `FindImage` (vk_rasterizer.cpp:1002-1005).
5. Second pass → `FindTexture` (texture_cache.cpp:963) → `UpdateImage` (texture_cache.h:158-163,
   under `mutex`) → `RefreshImage` (texture_cache.cpp:1054). For a new image the mip hashing at
   :1098-1101 is skipped because GpuDirty is set. Copies are built per mip: bufferOffset
   = mip offset, bufferRowLength = mip pitch, bufferImageHeight = mip height,
   layerCount = 1, imageExtent = (w>>m, h>>m, d>>m) (:1088-1118).
6. `BufferCache::ObtainBufferForImage` (buffer_cache.cpp:193-202): not GPU-modified, so the guest
   memory is copied into a HostUncached staging block with `CopySparseMemory`
   (memory.cpp:184-199). It reads through the backing pages and zero-fills unmapped pages, so
   it cannot fault.
7. `TileManager::DetileImage` (tile_manager.cpp:196-278): `TilingInfo params` (16 mips) filled
   from `mips_layout`; DeviceLocal staging `Request(guest_size, DeviceLocal, 256, false,
   unsynchronized=true)` (:221-222); `GetTilingPipeline` (:89-194) logs at :172, then compiles
   with `createComputePipelineUnique` (:186-189) and destroys the shader module (:192); the
   tiled input descriptor uses `range = VK_WHOLE_SIZE` (:233); dispatch
   `(guest_size/4)/64` groups (:270-271).
8. tiling.comp thick path: `ComputePixelIndexWithinMicroTile` thick branch (tiling.comp:175-204),
   `ComputeSurfaceAddrFromCoordMicroTiled` (:213-239), `GetMipLevel` (:416-425), `main`
   (:427-452).
9. `Runtime::UploadImage` (vk_runtime.cpp:170-197) → `copyBufferToImage`. Then `FindView`
   (image.cpp:209-235) creates the 3D view (image_view.cpp:92-153), descriptors are pushed and
   the dispatch is recorded (vk_rasterizer.cpp:419-426). Shadlix-only: `storage_sync_.Sync()`
   for every storage image after the dispatch (:431-434).

### 2.1 What was checked and found correct [VERIFIED]

- **Detile and upload math for Thick1DThick 32 bpp is in bounds.** I reimplemented
  `UpdateSize` (micro path) and the shader's micro-thick addressing (a standalone
  Python script, not kept; see Appendix A). For 32³, 16³, 64³, 32³ with 6 mips, 64×64×16 with 7 mips, 8³
  with 4 mips, 256×256×4 with 9 mips, 33³ and 17×9×3 with 3 mips: the highest tiled read
  end equals `guest_size` exactly, the dispatch covers exactly `guest_size/4` texels, no
  invocation runs past the last mip, and every upload region stays inside the staging
  allocation.
- `TilingInfo::mips[16]` and `mips_layout[16]` cannot overflow through `levels` (≤ 16).
- Shader buffer accesses are bounds-checked anyway: robustBufferAccess2, robustImageAccess2 and
  nullDescriptor are required and enabled (vk_instance.cpp:281-286, 510-512).
- Staging pool: no size overflow. The `unsynchronized` fallback reuses the next ring block at
  offset 0 without waiting (vk_staging_buffer_pool.cpp:62-66). That can corrupt data, but it
  cannot crash the CPU.
- `RunGarbageCollector()` is never called, at 88bd144 or in `beta`. `gc_tick` stays 0, so
  LRU `Touch`-after-`Free` and the `FreeDeferred` race on the priority-ops thread are dead code.
- `TileLinearBuffer` has no callers. `TileImage` is reached only through StorageImageSync
  (@88bd144), `TileImageToBuffer` (fork) and `SynchronizeMemoryFromImage`. No tiler pipeline
  was ever created in these runs, so no `TileImage` call ran.

---

## 3. Hypotheses, ranked

### H1 — Null `memset` in the LDS-emulation path (GPU thread; NVIDIA-specific trigger) — medium
[VERIFIED code defect / HYPOTHESIS as the cause]

- vk_rasterizer.cpp:880-887 (@88bd144 :843-846, identical upstream):
  `auto [data, offset] = lds_buffer.Map(lds_size, alignment); std::memset(data, 0, lds_size);`
- `StreamBuffer::Map` returns `{nullptr, 0}` when `size > size_bytes` (buffer.cpp:199-200,
  227-233). The stream buffer is 128 MiB (buffer_cache.cpp:26). The result is an access
  violation writing near address 0 on GpuCommandProcessor, which nothing handles and which
  writes no log. `StreamBuffer::Copy` asserts on this case (buffer.h:152-155); this path does
  not check it.
- When it runs: `SharedMemoryToStoragePass` (shared_memory_to_storage_pass.cpp:114-117) emulates
  LDS with a buffer when `shared_memory_size > maxComputeSharedMemorySize`. NVIDIA reports
  48 KiB in Vulkan (to be confirmed in the users' vulkaninfo). GCN shaders may use up to
  64 KiB, so this path is effectively NVIDIA-only. `lds_size = SharedMemSize() *
  NumWorkgroups()` is a u32 product (regs_shader.h:217-224), so a 64 KiB shader with more than
  2048 workgroups fails the `Map`.
- For `DispatchIndirect`, `dim_x/y/z` are never updated from the indirect arguments
  (liverpool.cpp:677-698, 1227-1250), so `lds_size` comes from stale registers.
- Fits the evidence: silent, GPU thread, at a compute dispatch, both users on NVIDIA, and the
  same code exists in this fork. Weak points: it is unproven that 0x480cc419 or a following
  dispatch uses more than 48 KiB of LDS. The crash would hit a *later* dispatch, because
  `BindBuffers` runs before `BindTextures` and the first dispatch reached the detile.
  Intermittency needs dispatch sizes that vary between runs.

### H2 — RenderTargetSync uses stale or freed image ids (Shadlix-only, still in this fork) — medium-low
[VERIFIED code defect / HYPOTHESIS as the cause]

- `pending_rt_writes_` (render_target_sync.h:87) maps a guest address to an `ImageId`. It is
  recorded at every RT bind (vk_rasterizer.cpp:1172-1179) and cleared only at the end of a guest
  submission (`OnSubmit`, vk_rasterizer.cpp:492-498, called from liverpool.cpp:171-174). Nothing
  removes an entry when the image is freed.
- During the same submission, images are freed by `ResolveOverlap` and `ExpandImage`. The
  equal-address, equal-size, 2D↔3D branch (texture_cache.cpp:577-581) is exactly a "first 3D use
  of memory that held another image" path, and `ExpandImage` frees the old image
  (texture_cache.cpp:802-826). The slot is erased by a deferred operation
  (texture_cache.cpp:1382-1415) as soon as any mid-submission `scheduler.Finish()` runs
  `PopPendingOperations` (vk_scheduler.cpp:117-143). Readbacks at EOP/EOS cause such Finishes
  (liverpool.cpp:770, 791 → texture_cache.cpp:102-149).
- After that, `CopyFromLastRt` (render_target_sync.cpp:26-53) reads the destroyed `Image`.
  `backing` points into a freed `std::deque` (image.h:164-165). `CopyRtToAlias`
  (render_target_sync.cpp:102-224) then calls `Transit`, which writes freed heap memory, and
  records `vkCmdCopyImage` with a destroyed `VkImage`, which can crash inside the driver. If the
  slot was reused, it copies from an unrelated image.
- Also: the bounds check uses the caller's T# size (render_target_sync.cpp:37) while the copy
  uses `alias_image.info.size` (:103-104). There is no texel-size, format or aspect check, and
  the regions always use aspect eColor (:145-147, 176-181, 206-212).
- Fits "intermittent" because it depends on whether a Finish happened between the free and the
  reuse. More readback traffic (readbackLinearImages, Fast) makes that more likely.

### H3 — Guest-side failure after kernel I/O on protected guest pages (readbackSpeed Fast) — low
[VERIFIED mechanism / HYPOTHESIS as the cause]

- With Fast, GPU-modified buffer pages get read and write watchers, so `Perms()` is None and
  the page becomes PAGE_NOACCESS (region_manager.h:101-102, 149-150; page_manager.cpp:46-112).
  User-mode accesses fault into `GuestFaultSignalHandler` (page_manager.cpp:220-228) and are
  downloaded. Kernel-mode accesses do not fault: `WriteFile` from such a buffer fails with
  ERROR_NOACCESS.
- `HostFile::Write` → `IOFile::WriteRaw` (host_fs.cpp:31-36; io_file.h:222-228) ignores short
  writes, so the game silently gets a partial write. Reads log `file read errored`, which never
  appears.
- In every run the `/app0` thread had just opened `RenderLowFidelity.replay` for writing. If the
  replay data comes from GPU-written memory, a short write could make the game abort (a guest
  trap is not handled, so the process dies silently).

### H4 — Crash Diagnostic Layer — unknown (confounder)
`crashDiagnostics=true` loads VK_LAYER_LUNARG_crash_diagnostic with `sync_after_commands=true`
and `output_path` = the log directory (vk_platform.cpp:393-405). It intercepts every call, so a
layer bug would crash silently. A real device loss is unlikely here: the next
`Semaphore::Refresh` or submit would assert (vk_semaphore.cpp:31-45, vk_scheduler.cpp:242), and
that message would have pushed the held `Control` line out. A wait already in progress would
spin forever in `Semaphore::Wait` (vk_semaphore.cpp:65-66), which looks like a freeze, not an
exit. If CDL wrote dumps, they are in the log folder.

### H5 — NVIDIA driver crash compiling the Thick detiler — low
The detiler log line is written before `createComputePipelineUnique` (tile_manager.cpp:172 vs
:186). Against it: the same crash on two driver branches, and the crash is intermittent.

### Not implicated at this moment (latent defects worth fixing)
- @88bd144 `StorageImageSync::Sync` (storage_image_sync.cpp:72-122 @88bd144) passes one region
  **per layer** into `TileImage`, which treats regions as **mips**
  (`params.num_mips = buffer_copies.size()`, tile_manager.cpp:379-382). With more than 16 layers
  this overflows the stack array (a /GS fast-fail, which bypasses every handler). For 3D images
  it downloads only slice z=0 and writes back the full `guest_size`. It is not reached in these
  runs (no tiler was logged), and this fork replaced it
  (storage_image_sync.cpp:59-78 → `WriteBackImage` / `TileImageToBuffer`, per mip).
- Unlocked `download_images.emplace`: `FindTexture` volume branch (texture_cache.cpp:968-970,
  Shadlix-only) and `AddDownload` (texture_cache.h:138-140, used for every 1×1 RT,
  render_target_sync.cpp:67-69). `DeleteImage` erases under the lock (texture_cache.cpp:1398-1404).
  Guest-thread munmap reaches it directly (memory.cpp:1008-1009 → vk_rasterizer.cpp:1468-1470 →
  texture_cache.cpp:451-461). No munmap happened near these crashes.
- Windows `SplitRegion` (address_space.cpp:351-436) unmaps the whole view while splitting.
  Before upstream #5317 (cherry-picked here as 914c5a1) it also dropped the page manager's
  protections. No map or unmap was logged near the crash.
- tiling.comp:419-422 reads `info.mips[num_mips]` (index 16 is out of bounds) when the loop
  exits; this is harmless in practice.
- `VK_WHOLE_SIZE` tiled-buffer range (tile_manager.cpp:233, :410; upstream used `guest_size`):
  on 4-8 GiB sparse arenas the effective range can exceed maxStorageBufferRange, which is an
  invalid descriptor.
- 1D-thick sizing uses thickness 4 for every mip (image_info.cpp:185-191). AddrLib switches
  mips with fewer than 4 slices to thin tiling, so `guest_size` is overestimated (correctness
  only; reads zero-fill).

---

## 4. Shadlix / this fork / official in these paths

- **Shadlix 88bd144 vs official base 3912336** [VERIFIED by diff]:
  - tile_manager.cpp: tiled descriptor `range` changed from `info.guest_size` to `VK_WHOLE_SIZE`
    (DetileImage :233, TileImage :410); `TileLinearBuffer` added but unused.
  - image_info.cpp: removed the `ArrayLinearGeneral` UNREACHABLE.
  - texture_cache.cpp `FindTexture`: the storage readback changed from
    `readback_linear_images && (!tiled || w<=8)` under the lock to an unconditional
    `!tiled && is_volume` branch without the lock (@88bd144 :640-656).
  - New Shadlix-only systems in the bind and dispatch path: RenderTargetSync, StorageImageSync,
    readback speed modes in region_manager/page_manager, and vk_predication.
- **Official commits after 3912336 that Shadlix lacks**: #5317 (Windows protections after
  split; in this fork), #5316 (TryWriteBacking source advance; Shadlix has its own fix),
  #5307, #5229, #5219 (per-image locking, atomic untrack), #5255, #5233, #5240, #5277.
- **This fork after 88bd144**: tile_manager.cpp, tiling.comp, image_info.cpp, image.cpp,
  buffer_cache.cpp, render_target_sync.*, memory.cpp, assert.cpp and logging are unchanged.
  Changed: storage_image_sync.cpp (GPU write-back), texture_cache.cpp (TileImageToBuffer,
  FlushBeforeCpuWrite, lazy write-back, linear storage readback e565b66), vk_rasterizer.cpp
  (FMASK, CMASK fill, fault-storm self-heal), address_space.cpp (#5317), and signals.cpp
  (unhandled-exception logger).

## 5. Settings (Q3)

- **readbackLinearImages**: @88bd144 it only affects render targets that are linear or tiled
  with width ≤ 8 (texture_cache.cpp:988-991). These are downloaded at EOP/EOS with
  `Finish` + `TryWriteBacking`; a failure there asserts, and asserts were not what happened
  here. It never touches the Thick image. It also writes linear data over tiled memory for
  8-wide tiled render targets (correctness bug). Indirectly, it adds mid-submission Finishes,
  which widens H2.
- **readbackSpeed Fast**: makes GPU-written buffer pages NOACCESS. CPU reads trigger a
  synchronous download on the GPU thread (`ReadMemory`, vk_rasterizer.cpp:1431-1438 →
  buffer_cache.cpp:104-128). Kernel I/O on such pages fails silently (H3). GPU-thread faults
  are serviced inline and call `scheduler.Finish()` from inside the handler.
- **crashDiagnostics**: H4.

## 6. Fixes, ranked by confidence

1. **Make the next crash visible** (certain value):
   - assert.cpp:18-22: call `Common::Log::Stop();` before `Crash();`.
   - signals.cpp:76-79: call `Common::Log::Stop();` before returning for `EXCEPTION_BREAKPOINT`.
   - backend.cpp:216-246: never hold entries with level ≥ Error. After setting `_last_entry`,
     write it immediately and reset it.
   - Add `std::set_terminate`, `signal(SIGABRT)` and `_set_invalid_parameter_handler` handlers
     that log and call `Log::Stop()`, plus a minidump (`MiniDumpWriteDump`).
     `SetUnhandledExceptionFilter` does not see `__fastfail` or abort.
2. **H1** (vk_rasterizer.cpp:880-887):
   ```cpp
   const u64 lds_size = u64(cs_program.SharedMemSize()) * cs_program.NumWorkgroups();
   VideoCore::StreamBufferMapping lds{lds_buffer, lds_size, alignment}; // temp buffer if > 128 MiB
   std::memset(lds.Data(), 0, lds_size);
   if (!lds.TemporaryBuffer()) {
       lds_buffer.Commit();
   }
   buffer_infos.emplace_back(lds.Buffer()->Handle(), lds.Offset(), lds_size);
   ```
   (buffer.cpp:294-319; the temporary buffer is deleted after the current tick and uses
   AllFlags including storage, buffer.h:44-45.) Log a warning when it exceeds 128 MiB. For
   `DispatchIndirect`, read the group counts with `CopySparseMemory` when the arguments are not
   GPU-modified, otherwise cap the size and warn.
3. **H2** (render_target_sync.*): store `{ImageId, image_uid}`. In `CopyFromLastRt`, drop the
   entry and return if `!(rt.flags & Registered) || rt.image_uid != uid`. Check the alias's own
   size against the RT, require equal `num_bits`, and skip depth aliases.
4. Lock `download_images_mutex` at texture_cache.cpp:968-970 and in `AddDownload`
   (texture_cache.h:138-140).
5. **H3**: in `HostFile::Write` and `Read` (host_fs.cpp:24-36), go through a bounce buffer so
   guest pages are touched in user mode, and log short writes and reads with `GetLastError()`.
6. `TileImage` (tile_manager.cpp:379): `ASSERT_MSG(buffer_copies.size() <= params.mips.size(), ...)`.
   tiling.comp:419: `while (mip + 1 < info.num_mips && texel >= mip_size)`.
7. tile_manager.cpp:233 and :410:
   `.range = std::min<u64>(info.guest_size, buffer->SizeBytes() - offset)`.
8. Model the AddrLib thick→thin switch for small mips (image_info.cpp:185-191 plus a per-mip
   thickness in `TilingInfo`). Correctness only.

## 7. Cheap logging at this spot

- `DetileImage`: log once per image (address, `guest_size`, w×h×d, levels, tile mode, staging
  offset).
- LDS path: log an error with `pgm_hash`, dims and size when the 128 MiB stream buffer is
  exceeded.
- `CopyFromLastRt`: warn once when an id is stale or the size/format do not match.
- `FindImage`: log new 3D images at INFO, once each.
- `HostFile::Write`: log an error on a short write.

## 8. Open questions and data that would decide

| Data | Confirms or rejects |
|---|---|
| Event Viewer → Application → Event 1000 for shadPS4.exe (module, code, offset) | 0xc0000005 in shadPS4.exe writing near 0 → H1. In nvoglv64.dll → H2/H5. In the CDL dll → H4. 0xc000001d or guest code → H3. 0xc0000409 → fast-fail |
| Run this fork's build (unhandled-exception logger) | Thread name and module; asserts stay hidden until fix 1 |
| A/B: crashDiagnostics off; readbackSpeed Disable; readbackLinearImages off; isIdenticalLogGrouped off | H4; H3/H2; H2; shows the real last message |
| CDL dump files in the log folder | Device loss or hang |
| Dumped shader 0x480cc419 (run 5): LDS size, indirect dispatch? T# of the 3D texture | H1 |
| Size of `temp0/replayPS4/RenderLowFidelity.replay` after a crash | H3 |
| Do AMD or Intel users crash at the same point? | NVIDIA-only supports H1 |

---

## Appendix A — Simulation

The script (Python, standalone) was not kept with these notes. It mirrored `UpdateSize`
(micro path), `ImageSizeMicroTiled`, `GetMipLevel` and the micro-thick address function, and
checks each read against `guest_size`, the dispatch coverage and the upload copy extents. Every
tested case reported max read end == `guest_size`, dispatched bytes == `guest_size`, no
invocation past the last mip, and copies inside staging.
