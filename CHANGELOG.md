# Changelog / Список изменений

Each version: English first, then Russian. The text of a version is also its release description.

Для каждой версии: сначала английский, потом русский. Текст версии — это и описание её релиза.

## Beta (not released yet)

### English

- UFC 3 works: career and fights tested
- Fighter shadows stay under the fighters (UFC 3 and UFC 4), the floor is lit again on Radeon iGPUs
- Faster start: the intro video is skipped (Settings → General → Skip Intro Video)
- FPS Limiter sets the real frame rate in UFC (it gave half of it)
- A new install starts with recommended settings (shaders kept on disk, Readback Speed Low, FPS Limiter value = monitor refresh rate, the limiter itself off); an update keeps your settings
- Render Resolution for UFC 4 is in the launcher (Settings → Graphics), also for the EU release
- UFC 4 fighters always have heads, whatever Readback Speed is set to
- Two controllers: buttons no longer get stuck
- Launcher: updates from this fork (Main and Beta channels), every setting explains what it does
- Launcher: PKG install works and is about 2.5x faster, with progress and Cancel; DLC goes to the DLC folder
- Launcher: no more crashes on close or "No Qt platform plugin" errors
- Crashes leave their reason in the log

### Русский

- UFC 3 работает: проверены карьера и бои
- Тени бойцов под бойцами (UFC 3 и UFC 4), на встроенной графике Radeon снова освещён пол
- Быстрый запуск: интро-ролик пропускается (Settings → General → Skip Intro Video)
- FPS Limiter задаёт настоящую частоту кадров в UFC (раньше давал половину)
- Новая установка сразу с рекомендуемыми настройками (шейдеры сохраняются на диск, Readback Speed Low, значение FPS Limiter = частота монитора, сам лимитер выключен); при обновлении ваши настройки остаются
- Render Resolution для UFC 4 есть в лаунчере (Settings → Graphics), и для европейской версии тоже
- У бойцов UFC 4 всегда есть голова, при любом Readback Speed
- Два геймпада: кнопки больше не залипают
- Лаунчер: обновления из этого форка (каналы Main и Beta), у каждой настройки есть описание
- Лаунчер: установка PKG работает и идёт примерно в 2,5 раза быстрее, с прогрессом и отменой; DLC ставятся в папку DLC
- Лаунчер: больше не вылетает при закрытии и не выдаёт ошибку «No Qt platform plugin»
- Вылеты оставляют причину в логе

## 2026-10-11.2

### English

- UFC 4 career is fully playable
- Created fighters keep their avatars (re-save fighters made with older builds)
- Hair looks right
- Fixes from the official shadPS4 (October 2026), including a crash in game audio
- NVIDIA: the fixes against black squares are on as well (not tested on NVIDIA)

### Русский

- Карьера UFC 4 полностью проходится
- У созданных бойцов сохраняются аватарки (бойцов из старых сборок пересохраните)
- Волосы выглядят как надо
- Исправления из официального shadPS4 (октябрь 2026), в том числе вылет в звуке игры
- NVIDIA: исправления чёрных квадратов включены и там (на NVIDIA не проверялось)

## 2026-10-11 (first release / первый релиз)

### English

- Clean picture on Intel Arc: no black squares, flickering floor or gaps in shadows
- Create Fighter works: right skin after choosing a preset, much less stutter while editing
- Natural hair edges
- No more random freezes where the picture stops and the sound goes on
- Less shader stutter: shaders are kept on disk
- UFC 4 at native 1080p (`UFC4_1080p.bat`)
- Collect Debug Logs option for bug reports

### Русский

- Чистая картинка на Intel Arc: без чёрных квадратов, мерцающего пола и дыр в тенях
- Создание бойца работает: правильная кожа после выбора пресета, гораздо меньше подтормаживаний при редактировании
- Естественные края волос
- Больше нет случайных зависаний, когда картинка стоит, а звук идёт
- Меньше подтормаживаний из-за шейдеров: шейдеры сохраняются на диск
- UFC 4 в родном 1080p (`UFC4_1080p.bat`)
- Опция Collect Debug Logs для отчётов об ошибках
