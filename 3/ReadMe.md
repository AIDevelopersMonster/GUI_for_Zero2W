# Игра "Двоичное Число" с GPIO-индикацией для Raspberry Pi

Это интерактивная обучающая игра на языке C с графическим интерфейсом GTK+3, разработанная для Raspberry Pi Zero 2 W. Она помогает пользователям освоить двоичную систему счисления, визуализируя случайные двоичные числа с помощью подключенных к GPIO светодиодов и позволяя вводить десятичные эквиваленты через GUI.

## Описание проекта

Проект реализует простую игру, где Raspberry Pi генерирует случайное 8-битное двоичное число, отображая его на восьми физических светодиодах, подключенных к GPIO. Пользователь вводит свою догадку в графическом приложении, и система проверяет ответ, ведя подсчет правильных и неправильных попыток.

## Функционал

* Генерация случайных 8-битных двоичных чисел (от 0 до 255).
* Визуализация двоичных чисел на 8 GPIO-управляемых светодиодах.
* Графический интерфейс пользователя (GUI) с индикаторами светодиодов, полем ввода и кнопками управления.
* Подсчет и отображение статистики: количество правильных и неправильных ответов.
* Кнопки "Старт", "Ваш ответ" и "Стоп" для управления игрой.

## Требования

### Аппаратные

* **Raspberry Pi Zero 2 W** (или любая другая модель Raspberry Pi с доступными GPIO)
* **8 светодиодов** (любого цвета)
* **8 резисторов 220 Ом** (для каждого светодиода)
* Макетная плата (по желанию, для удобства монтажа)
* Соединительные провода

### Программные

* Операционная система **Raspberry Pi OS** (или совместимая Linux-система).
* **GCC** (GNU Compiler Collection)
* **GTK+3** (библиотека для создания GUI)
* **libgpiod** (библиотека для работы с GPIO)

## Подключение компонентов

Подключите 8 светодиодов к следующим GPIO-пинам Raspberry Pi через резисторы 220 Ом. Другой конец каждого светодиода подключите к контакту GND.

| Индекс бита (в коде) | GPIO-пин (BCM) | Физический пин | Значимость бита   |
| :------------------- | :------------- | :------------- | :---------------- |
| 0                    | GPIO4          | 7              | Младший бит (LSB) |
| 1                    | GPIO25         | 22             |                   |
| 2                    | GPIO24         | 18             |                   |
| 3                    | GPIO23         | 16             |                   |
| 4                    | GPIO22         | 15             |                   |
| 5                    | GPIO27         | 13             |                   |
| 6                    | GPIO18         | 12             |                   |
| 7                    | GPIO17         | 11             | Старший бит (MSB) |

## Установка и запуск

1.  **Клонируйте репозиторий** (или скачайте файл `binary_game.c`):
    ```bash
    git clone [https://github.com/AIDevelopersMonster/GUI_for_Zero2W.git](https://github.com/AIDevelopersMonster/GUI_for_Zero2W.git))
    cd GUI_for_Zero2W/chapter3
    ```


2.  **Установите необходимые библиотеки**, если они еще не установлены:
    ```bash
    sudo apt update
    sudo apt install libgtk-3-dev libgpiod-dev
    ```

3.  **Скомпилируйте исходный код:**
    ```bash
    gcc binary_game.c -o binary_game $(pkg-config --cflags --libs gtk+-3.0 libgpiod) -Wall -Wextra
    ```

4.  **Запустите игру:**
    ```bash
    ./binary_game
    ```

## Создание ярлыка для рабочего стола (Опционально)

Для удобного запуска игры без использования терминала, вы можете создать файл `.desktop`:

1.  Создайте файл `binary_game.desktop` в директории `~/.local/share/applications/`:
    ```bash
    mkdir -p ~/.local/share/applications/
    nano ~/.local/share/applications/binary_game.desktop
    ```

2.  Вставьте следующее содержимое, **обязательно указав правильный полный путь** к вашему исполняемому файлу `binary_game` и иконке (если есть):
    ```ini
    [Desktop Entry]
    Version=1.0
    Type=Application
    Name=Игра "Двоичное Число"
    Comment=Обучающая игра для Raspberry Pi: угадай двоичное число
    Exec=/home/pi/your-repo-name/chapter3/binary_game # Обновите путь!
    Icon=/home/pi/your-repo-name/chapter3/binary_game_icon.png # Обновите путь, если используете иконку
    Terminal=false
    Categories=Game;Education;
    ```
    (Если у вас нет иконки, можете опустить строку `Icon=` или указать путь к стандартной иконке.)

3.  Сохраните файл и сделайте его исполняемым:
    ```bash
    chmod +x ~/.local/share/applications/binary_game.desktop
    ```
    Теперь ярлык должен появиться в меню приложений Raspberry Pi OS.

## Скриншоты

Вы можете найти скриншоты интерфейса и схемы подключения в директории `_static` (или `static` после деплоя на GitHub Pages):
* [Схема подключения](_static/chapter03_schematic.png)
* [Макет GUI игры](_static/chapter03_gui_mockup.png)

---

Приятной игры и обучения!
