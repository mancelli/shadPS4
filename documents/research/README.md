<!--
SPDX-FileCopyrightText: 2026 mancelli/shadPS4 fork
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Research notes / Исследования

**These notes were written with an AI assistant (Claude) while working on this fork, and checked by a
human only in part. Claims are tied to code (file:line at the commit named in each document);
anything not proven is marked as a hypothesis. Line numbers drift: use the commit named in the
document.**

**Эти заметки написаны с помощью ИИ (Claude) во время работы над форком; человек проверил их лишь
частично. Выводы привязаны к коду (файл:строка на указанном в документе коммите), недоказанное
помечено как гипотеза. Номера строк со временем уезжают: смотрите коммит, указанный в документе.**

Use them to port the fixes to newer shadPS4/Shadlix builds or to continue an investigation. Start
with [fork-changes.md](fork-changes.md). Raw player logs are not published.

Используйте их, чтобы перенести исправления в новые сборки shadPS4/Shadlix или продолжить
расследование. Начинать стоит с [fork-changes.md](fork-changes.md). Сырые логи игроков не
публикуются.

| Document | Language | Topic / Тема | State |
|---|---|---|---|
| [fork-changes.md](fork-changes.md) | EN | Every fork commit by area: what, why, files, A/B switches / все коммиты форка по областям | reference |
| [project-notes.md](project-notes.md) | EN | Branches, CI, releases, updater, open problems, removed files / ветки, сборка, релизы, открытые проблемы | 2026-10-17 |
| [readbacks.md](readbacks.md) | EN | Five Readback Speed modes were two behaviours; On/Off switch; upstream modes / ридбеки | done (cdfb6fa) |
| [two-gamepads-stuck-buttons.md](two-gamepads-stuck-buttons.md), [two-gamepads-sim.cpp](two-gamepads-sim.cpp) | EN | Stuck trigger with two gamepads, with a simulation / залипание с двумя геймпадами | fixed (428ad3b) |
| [launcher-pkg-and-qt.md](launcher-pkg-and-qt.md) | EN | "No Qt platform plugin", PKG install crash and speed, crash on exit / ошибки лаунчера и установки PKG | fixed (6750a4c, a971a34, a3ee89a, 63f4890, d5cc667) |
| [ufc4-startup-crash-nvidia.md](ufc4-startup-crash-nvidia.md) | EN | UFC 4 crash at start on NVIDIA: log analysis, hypotheses, what would settle it / вылет UFC 4 на NVIDIA | open |
| [bbfork-2025-11-05-fps.md](bbfork-2025-11-05-fps.md), [bbfork-2025-11-05-fps-findings.json](bbfork-2025-11-05-fps-findings.json) | RU | Why BBFork 2025-11-05 may give more FPS than release 12739d2 / почему старый BBFork может давать больше FPS | open (candidates) |
| [fsr-xess.md](fsr-xess.md), [fsr-xess-data.json](fsr-xess-data.json) | RU | FSR 1 in the emulator, RCAS bug, why not XeSS, options for weak PCs / FSR, XeSS, слабые ПК | RCAS fixed (d552df8) |

The `.json` files are the raw output of multi-agent reviews: each area's findings with their evidence
and an independent check of them. The `.md` next to them is the summary.

Файлы `.json` — сырые результаты разбора несколькими агентами: находки по каждой области с
доказательствами и их независимая проверка. Рядом лежащий `.md` — итоговый отчёт.
