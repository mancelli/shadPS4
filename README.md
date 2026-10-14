<!--
SPDX-FileCopyrightText: 2024 shadPS4 Emulator Project
SPDX-License-Identifier: GPL-2.0-or-later
-->

# **⚠️ ALL CHANGES IN THIS FORK WERE MADE ENTIRELY WITH AI (CLAUDE BY ANTHROPIC) AND TESTED BY A HUMAN.**

# **⚠️ ВСЕ ИЗМЕНЕНИЯ В ЭТОМ ФОРКЕ ПОЛНОСТЬЮ СДЕЛАНЫ С ПОМОЩЬЮ ИИ (CLAUDE ОТ ANTHROPIC) И ПРОВЕРЕНЫ ЧЕЛОВЕКОМ.**

**[English](#english) · [Русский](#русский)**

## English

A fork of the [shadPS4](https://github.com/shadps4-emu/shadPS4) PS4 emulator (through
[Shadlix](https://github.com/diegolix29/shadPS4)) that makes **EA SPORTS UFC 4** playable on PC.
Made and tested on **Windows 11 with an Intel Arc B580**, with some checks on a Radeon 780M.
Windows only; NVIDIA cards are not tested.

### Games

| Game | Version tested | State |
|---|---|---|
| UFC 4, CUSA14204 (US) | 28.00 | Career, Create Fighter and fights work |
| UFC 4, CUSA14209 (EU) | – | Same game, not tested |
| UFC 3, CUSA06534 (EU) | 1.14 | Career and fights work |

Other versions (for example 1.00) are not tested.

### Main fixes

- Career and Create Fighter in UFC 4 work fully
- Clean picture on Intel Arc
- Fighters look right: skin, hair, shadows
- UFC 4 in 1080p and higher
- Stable and quick: no random freezes, less stutter, faster start

…and much more: [full list of changes](CHANGELOG.md).

### How to use

1. Download `shadPS4-ufc4-intel-arc-windows-qt-launcher.zip` from [Releases](../../releases).
   The newest fixes are in the **Beta** pre-release (it can have new bugs).
2. Unpack **the whole zip** into a new folder and run `shadPS4.exe`. All fixes are on by default, and a new
   install starts with recommended settings.
3. Add your games folder in the launcher and start the game. The first start builds shaders and stutters once.

Useful settings:
- **Settings → Graphics → Render Resolution (UFC 4)**: 1080p and higher.
- **Settings → Graphics → FPS Limiter**: your frame rate, for example your monitor's refresh rate.
- **Settings → General → Skip Intro Video**: on by default.

### If something goes wrong

- **"No Qt platform plugin could be initialized"**: unpack the whole zip into a new folder; do not run the exe
  from inside the zip.
- **Anything else**: turn on **Settings → Logger → Collect Debug Logs**, play until the problem happens, then
  attach the log (**Open Log Location**) to an [issue](../../issues).

### More

- [Shadlix README](https://github.com/diegolix29/shadPS4/blob/Shadlix/README.md): launcher features of Shadlix.
- [shadPS4 README](https://github.com/shadps4-emu/shadPS4/blob/main/README.md): the original emulator, building,
  firmware files, keyboard mapping.
- License: [GPL-2.0-or-later](LICENSE), like shadPS4.

### Support

These fixes are made with a paid AI subscription (Claude). If the builds help you, you can chip in for it on
[Boosty](https://boosty.to/mancelli/donate). Entirely optional: the builds stay free, there are no paid-only
versions, and donations do not buy specific fixes or dates.

## Русский

Форк PS4-эмулятора [shadPS4](https://github.com/shadps4-emu/shadPS4) (через
[Shadlix](https://github.com/diegolix29/shadPS4)), в котором можно играть в **EA SPORTS UFC 4** на ПК.
Сделан и проверен на **Windows 11 с Intel Arc B580**, частично на Radeon 780M.
Только Windows; на видеокартах NVIDIA не проверялся.

### Игры

| Игра | Проверенная версия | Состояние |
|---|---|---|
| UFC 4, CUSA14204 (США) | 28.00 | Карьера, создание бойца и бои работают |
| UFC 4, CUSA14209 (Европа) | – | Та же игра, не проверялась |
| UFC 3, CUSA06534 (Европа) | 1.14 | Карьера и бои работают |

Другие версии (например 1.00) не проверялись.

### Главные исправления

- Карьера и создание бойца в UFC 4 работают полностью
- Чистая картинка на Intel Arc
- Бойцы выглядят как надо: кожа, волосы, тени
- UFC 4 в 1080p и выше
- Стабильно и быстро: без зависаний, меньше подтормаживаний, быстрый запуск

…и многое другое: [полный список изменений](CHANGELOG.md).

### Как пользоваться

1. Скачайте `shadPS4-ufc4-intel-arc-windows-qt-launcher.zip` в [Releases](../../releases).
   Самые новые исправления — в пре-релизе **Beta** (в нём могут быть новые баги).
2. Распакуйте **весь архив** в новую папку и запустите `shadPS4.exe`. Все исправления включены сразу, а новая
   установка начинается с рекомендуемых настроек.
3. Добавьте в лаунчере папку с играми и запустите игру. Первый запуск собирает шейдеры, поэтому один раз будут
   подтормаживания.

Полезные настройки:
- **Settings → Graphics → Render Resolution (UFC 4)**: 1080p и выше.
- **Settings → Graphics → FPS Limiter**: ваша частота кадров, например частота монитора.
- **Settings → General → Skip Intro Video**: включено по умолчанию.

### Если что-то не так

- **«No Qt platform plugin could be initialized»**: распакуйте весь архив в новую папку, не запускайте exe прямо
  из архива.
- **Любая другая проблема**: включите **Settings → Logger → Collect Debug Logs**, играйте до появления проблемы
  и приложите лог (**Open Log Location**) к [issue](../../issues).

### Ещё

- [README Shadlix](https://github.com/diegolix29/shadPS4/blob/Shadlix/README.md): возможности лаунчера Shadlix.
- [README shadPS4](https://github.com/shadps4-emu/shadPS4/blob/main/README.md): оригинальный эмулятор, сборка,
  файлы прошивки, управление с клавиатуры.
- Лицензия: [GPL-2.0-or-later](LICENSE), как у shadPS4.

### Поддержать

Исправления делаются на платной подписке ИИ (Claude). Если сборки вам помогли, можно скинуться на неё на
[Boosty](https://boosty.to/mancelli/donate). Только по желанию: сборки остаются бесплатными, закрытых версий нет,
а за донаты не обещаются конкретные исправления или сроки.
