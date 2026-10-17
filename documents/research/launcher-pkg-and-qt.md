<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Launcher: "No Qt platform plugin", PKG install, crash on exit

Dates: 2026-10-11 to 2026-10-14. All three problems are fixed in the release of 2026-10-14.

## 1. "This application failed to start because no Qt platform plugin could be initialized"

**Report (2026-10-11):** the launcher of the 2026-10-11.2 release does not start on some PCs.

**The zip is fine [VERIFIED].** `shadPS4-ufc4-intel-arc-windows-qt-launcher.zip` contains
`qtplugins/platforms/qwindows.dll`, a `qt.conf` pointing to `./qtplugins` and Qt 6.9.3 DLLs of the
same version; its file list matched the earlier release. So the cause is on the player's PC.

**Usual causes, most likely first:**
1. The exe is run from inside the zip, or only the exe was copied over an older install. Fix:
   unpack the whole zip into a new folder (the README says so).
2. `QT_PLUGIN_PATH` or `QT_QPA_PLATFORM_PLUGIN_PATH` set by another Qt program sends Qt to the
   wrong plugin folder.
3. An antivirus removed `qtplugins\platforms\qwindows.dll`.

`QT_DEBUG_PLUGINS=1` in a command prompt, then `shadPS4.exe` from the same prompt, prints which
plugin Qt tried and why it failed.

**Fix (6750a4c, `src/qt_gui/main.cpp:54-67`):** before QApplication is created the launcher clears
both variables and adds `<exe folder>/qtplugins` to the library paths, which removes cause 2. Also
fixed: the converted argv was freed while QApplication still used it.

## 2. PKG install

**Reports (2026-10-13):** game PKGs install only with shadPS4 0.7.0, not with this fork; later
"the emulator just closed" while installing.

### Extraction code compared with shadPS4 0.7.0 [VERIFIED]

`src/core/file_format/pkg.cpp` (decryption, PFS) matches 0.7.0: same keys, same file system
format. Three small differences, none of them break a game install:
- `aesCbcCfb128DecryptEntry` (`src/core/crypto/crypto.cpp`) throws `std::length_error` when its
  buffers differ in size;
- the extraction root is the folder chosen by the caller;
- the file write mode has a different name with the same effect.

### The launcher's installer (Shadlix rewrote it)

Shadlix's Install PKG (`src/qt_gui/version_dialog.cpp` `InstallPkgFiles`,
`src/pkg_installer/pkg_installer.cpp`) behaved differently from 0.7.0:

- **DLC went into the game folder** and overwrote game files; the emulator looks for DLC in
  `<DLC folder>/<title id>/<entitlement label>` (`app_content.cpp`). Fixed in a971a34: CATEGORY
  `ac` goes to the DLC folder, `gp` counts as an update.
- **std::filesystem exceptions** (no access, path over 260 characters) ended the launcher
  (`std::terminate`). Fixed in a971a34: a message box with the reason.
- **No install folder prompt:** the game goes into the first game folder from Settings → Paths.
  Unchanged.

### The real cause of the closing launcher (a3ee89a)

The launcher died with `STATUS_HEAP_CORRUPTION` (0xC0000374) in ntdll right after
`sce_sys/digests` was written, in the RSA decryption of the entry keys. cryptopp-cmake applies
`CRYPTOPP_DISABLE_ASM` (set on Windows since the built-in installer was added) and the AESNI/AVX2
switches to the library's own sources only. In the emulator's sources `CRYPTOPP_BOOL_ALIGN16`
stayed 1 while the library was built with 0, so memory allocated with `AlignedAllocate` was
released with the unaligned deallocator. shadPS4 0.7.0 did not disable ASM, which is why it
worked there.

Fix: the definitions are also INTERFACE definitions of the `cryptopp` target
(`externals/CMakeLists.txt`). Checked with a local test running the installer's RSA key decryption
on a game PKG: 0xC0000374 before, decrypts after.

### Speed (63f4890)

Extraction ran on the GUI thread, one file at a time (UFC 3 starts with a 4.3 GB audio archive: the
window froze for about 20 s at 0 %). Now: a worker thread with up to 8 extraction threads like 0.7.0,
progress in bytes, Cancel, AES key schedules set up once per call. UFC 3 game PKG, first 300 files
(17.7 GB): 94 s before, 38 s after; files identical to the installed game.

### Left open

`PKG::ExtractFiles` keeps the file size in `int bsize` (`pkg.cpp:426`), which looks wrong for files
over 2 GiB. The 4.3 GB UFC 3 archive still extracted byte for byte, so it is not a practical problem
today; not investigated further.

## 3. Launcher crash on exit (d5cc667)

The launcher crashed with `STATUS_HEAP_CORRUPTION` every time it closed (release, beta and local
builds; 0.7.0 exits cleanly). `MainWindow` created the IPC client with
`std::make_shared<IpcClient>(this)`, so it had two owners: the shared_ptrs and Qt (as a child of
the window). `~QWidget` deleted it while it lived inside the make_shared block. Fix: no Qt parent.
