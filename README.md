# ModCD

**ModCD** — это homebrew-приложение, предоставляющее пользователю следующие возможности (непосредственно со Switch):

- **Скачивание**: Механизм проверки версии установленных игр позволяет скачивать только совместимые с игрой моды.
- **Установка/Удаление**: MCDS (ModCD Script/Scenario) позволяет задавать правила установки и удаления, используя доступный набор инструкций.
- **Просмотр информации**
- **Просмотр скриншотов**
- **Виджет "было-стало"**: Виджет для сравнения скриншотов до и после установки мода, позволяющий наглядно увидеть различия.

## Установка и запуск

1. Скачать файл `ModCD.nro` из доступных релизов
2. Поместить файл `ModCD.nro` по следующему пути -> `sd:/switch/ModCD.nro`
3. Запустить `HB-Menu` в `non-applet` режиме (запустите любую игру с зажатой `R`)
4. Запустить `ModCD.nro` (убедитесь, что есть подключение к интернету)

> Если ModCD зависает после запуска на логотипе, то попробуйте в файле конфигурации, который лежит по пути `sd:/.modcd/config.json` в поле `repositoryUrl` изменить адрес на `http://193.23.219.35:1785/repository.json`

4. Выход из `ModCD` осуществляется по нажатию кнопки `+`
5. На странице мода можно нажать `X` для включения/выключения скроллинг-режима, применяемый к описанию мода

## Доступные моды

Полный список доступных модов можно посмотреть здесь -> [таблица модов](table.md)

## Список авторов модов

Если вам понравился мод, то не забудьте поблагодарить его автора

- ECLIPSE00074
- \_ZolMik\_ ([страница с информацией для донатов](https://github.com/kawaii-flesh/ModCD-Donations/blob/master/_ZolMik_.md))
- Vetal580
- Sun

## FAQ (которые никто не задавал)

### ModCD сразу закрывается после открытия
Что-то из этого может помочь:
1. Убедитесь, что выполнен `3-й` пункт из раздела `Установка`
2. Если не помогло, то удалите с корня карты директорию `.modcd`, перезапустите `HB-Menu` и снова откройте `ModCD`
3. Переустановите `ModCD`
4. Проверьте наличие "битых" игр
Если ничего не помогло, пожалуйста, заведите ишью, прикрепив лог, который можно взять по следующему пути `sd:/.modcd/log.txt` с тегами: `bug`, `версией релиза` и описанием условий запуска

### В ModCD не отображается установленная мной игра
В таком случае может быть одно из:
- на эту игру в `репозитории ModCD` нет ни одного мода
- в репозитории есть мод, но версия контента, с которой он совместим, не соответствует вашей

### Для моей игры есть мод, совместимый с ее версией, от автора, который есть в списке авторов. Можно ли его добавить в репозиторий?
Да, пожалуйста, создайте ишью c тегом `mod request`, в описании укажите:
1. `Название мода`, `Title ID`, `версию контента игры` (можно посмотреть в `dbi`)
2. `Ссылку на мод`

### Для моей игры есть мод, совместимый с ее версией от автора, которого нет в списке авторов. Можно ли его добавить в репозиторий?
Нет. Без разрешения автора я не буду этого делать

### После удаления мода и скриншотов с помощью кнопки `Delete` остаются пустые директории
Это особенность текущей реализации, все можно удалить, удалив директорию `sd:/.modcd`

### Я являюсь автором модов и хочу добавить их в репозиторий ModCD
Пожалуйста, напишите мне в [ТГ](https://t.me/kawaii_flesh)

### У меня есть идея по функционалу ModCD
Пожалуйста, заведите ишью с тегом `feature request` и опишите вашу идею

### Я нашел баг
Пожалуйста, заведите ишью, прикрепив лог, который можно взять по следующему пути `sd:/.modcd/log.txt` с тегами: `bug`, `версией релиза` и описанием условий запуска и сценария

### Проблема с модом, его описанием, скриншотами и тд
Пожалуйста, заведите ишью, с тегом `mod bug` и опишите в чем конкретно проблема, указав дополнительно:
1. Title ID
2. Версию контента
3. Название мода
4. `Description` (если оно есть в плашке мода)
5. Автора мода

### У меня есть вопрос, которого нет в FAQ
Пожалуйста, заведите ишью, с тегом `question` и задайте свой вопрос

### Я хочу поддержать проект. Как это сделать?
1. Поддержать автора модов (ссылка на донаты есть в списке авторов и, обычно, QR'ам в скриншотах к моду)
2. Помочь с добавлением модов в репозиторий (пожалуйста, напишите мне в [ТГ](https://t.me/kawaii_flesh). Объясню и научу как это делать)
3. Задонатить на оплату хостинга -> [здесь](https://github.com/kawaii-flesh/ModCD-Donations/blob/master/HOSTING.md)

## Скриншоты

![Games](/screenshots/games.jpg)

![Mods list](/screenshots/mods_list.jpg)

![Mod page](/screenshots/mod_page.jpg)

![Screenshots](/screenshots/screenshots.jpg)

![Before-after](/screenshots/ba.jpg)

