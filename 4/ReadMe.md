# RGB LED Controller (GTK + pigpio)

Приложение для управления RGB-светодиодом с помощью графического интерфейса на `GTK+3`
и аппаратного ШИМ через `pigpio`. Работает на Raspberry Pi.

---

## Описание

Программа предоставляет простой GUI с тремя ползунками (R, G, B), которые управляют
яркостью каналов RGB-светодиода. При изменении значений:

-   отправляется ШИМ-сигнал на соответствующий GPIO-пин;
-   обновляется отображаемый цвет в окне;
-   значения яркости отображаются в виде чисел.

Поддерживаются светодиоды с общим катодом (подключение напрямую к GPIO через резисторы).

Используемые пины (Broadcom GPIO):

-   **GPIO17** — красный (R)
-   **GPIO27** — зелёный (G)
-   **GPIO18** — синий (B)

---

## Требования

Для успешной сборки и запуска проекта вам понадобятся:

* **Raspberry Pi:** Любая модель с 40-пиновым разъемом GPIO (например, Raspberry Pi 2, 3, 4, 5).
* **Операционная система:** Raspberry Pi OS (ранее Raspbian) или другая совместимая Linux-система.
* **Библиотеки:**
    * **GTK 3:** Библиотека для создания графического интерфейса.
    * **pigpio:** Библиотека для высокоточного управления GPIO, включая ШИМ (PWM).
* **Аппаратные компоненты:**
    * **RGB-светодиод:** С общим катодом (наиболее распространенный) или общим анодом.
    * **Резисторы:** Три токоограничивающих резистора (например, 220 Ом или 330 Ом, в зависимости от светодиода и напряжения питания) для каждого из трех цветовых каналов RGB-светодиода.
    * **Макетная плата и соединительные провода:** Для сборки схемы.

---

## Подключение компонентов (Схема)

Для **RGB-светодиода с общим катодом**:

1.  **Длинная ножка (общий катод):** Подключите к контакту **GND** (земля) Raspberry Pi.
2.  **Красный канал:** Подключите ножку красного цвета к **GPIO 17** (физический пин 11) через токоограничивающий резистор.
3.  **Зеленый канал:** Подключите ножку зеленого цвета к **GPIO 27** (физический пин 13) через токоограничивающий резистор.
4.  **Синий канал:** Подключите ножку синего цвета к **GPIO 18** (физический пин 12) через токоограничивающий резистор.

Для **RGB-светодиода с общим анодом**:

1.  **Длинная ножка (общий анод):** Подключите к контакту **3.3V** Raspberry Pi.
2.  **Красный канал:** Подключите ножку красного цвета к **GPIO 17** (физический пин 11) через токоограничивающий резистор.
3.  **Зеленый канал:** Подключите ножку зеленого цвета к **GPIO 27** (физический пин 13) через токоограничивающий резистор.
4.  **Синий канал:** Подключите ножку синего цвета к **GPIO 18** (физический пин 12) через токоограничивающий резистор.
    * **Примечание:** Для светодиодов с общим анодом логика ШИМ инвертируется (0% рабочего цикла = максимальная яркость, 100% = выключено). Код написан для общего катода, где более высокое значение соответствует большей яркости. Если цвета будут инвертированы, возможно, потребуется изменить значения `value` на `255 - value` перед вызовом `gpioPWM`.

---

## Установка зависимостей

Убедитесь, что у вас установлены необходимые библиотеки.

**1. Установка GTK 3:**

```bash
sudo apt update
sudo apt install libgtk-3-dev
````

**2. Установка pigpio:**

Библиотека pigpio часто предустановлена на Raspberry Pi OS. Если нет, вы можете установить её:

```bash
sudo apt install pigpio
```

-----

## Запуск pigpio-демона

Для работы `pigpio` требуется фоновый демон `pigpiod`. Его нужно запустить **до запуска программы**.

```bash
sudo pigpiod
```

Если хотите, чтобы он запускался автоматически при загрузке системы:

```bash
sudo systemctl enable pigpiod
sudo systemctl start pigpiod
```

Остановить вручную:

```bash
sudo killall pigpiod
```

-----

## Исходный код

Сохраните следующий код в файл с именем `rgb_pwm_gui.c`:

```c
#include <gtk/gtk.h> // Подключаем библиотеку GTK для создания графического интерфейса
#include <pigpio.h> // Подключаем библиотеку pigpio для работы с GPIO и ШИМ
#include <stdio.h>   // Подключаем стандартную библиотеку ввода/вывода для snprintf

// Определяем константы для номеров GPIO-пинов, связанных с каждым цветом.
// Эти пины будут использоваться для ШИМ.
#define RED_PIN    17
#define GREEN_PIN  27
#define BLUE_PIN   18

// Глобальные указатели на виджеты GTK.
// color_area: Виджет, который будет отображать текущий смешанный цвет.
// label_r, label_g, label_b: Метки для отображения числовых значений (0-255) каждого цвета.
GtkWidget *color_area;
GtkWidget *label_r, *label_g, *label_b;

// Глобальные указатели на виджеты ползунков для более легкого доступа в on_scale_changed
GtkWidget *scale_r_global;
GtkWidget *scale_g_global;
GtkWidget *scale_b_global;

// --- Функции ---

// Функция для обновления цвета виджета в GUI
void update_color_display(int r, int g, int b) {
    char css[128];
    // Формируем строку CSS, которая задает фоновый цвет виджета
    snprintf(css, sizeof(css), "#color_display { background-color: rgb(%d,%d,%d); }", r, g, b);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(color_area);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

// Функция обратного вызова, вызываемая при изменении значения любого ползунка.
// user_data теперь не используется для получения значений ползунков,
// так как они глобальны.
void on_scale_changed(GtkRange *range, gpointer user_data) {
    // Получаем текущие значения со всех трех ползунков напрямую, так как они глобальны
    int r = (int)gtk_range_get_value(GTK_RANGE(scale_r_global));
    int g = (int)gtk_range_get_value(GTK_RANGE(scale_g_global));
    int b = (int)gtk_range_get_value(GTK_RANGE(scale_b_global));

    // Обновляем текстовые метки рядом с ползунками
    char text[16];
    snprintf(text, sizeof(text), "R: %d", r);
    gtk_label_set_text(GTK_LABEL(label_r), text);

    snprintf(text, sizeof(text), "G: %d", g);
    gtk_label_set_text(GTK_LABEL(label_g), text);

    snprintf(text, sizeof(text), "B: %d", b);
    gtk_label_set_text(GTK_LABEL(label_b), text);

    // Отправляем ШИМ-сигналы на GPIO-пины с помощью pigpio
    gpioPWM(RED_PIN, r);
    gpioPWM(GREEN_PIN, g);
    gpioPWM(BLUE_PIN, b);

    // Обновляем цвет отображаемого виджета в GUI
    update_color_display(r, g, b);
}

// --- Основная функция программы ---

int main(int argc, char *argv[]) {
    // Инициализация библиотеки pigpio. Это должно быть сделано перед любым использованием pigpio функций.
    if (gpioInitialise() < 0) {
        g_printerr("Failed to initialize pigpio\n"); // Выводим сообщение об ошибке, если инициализация не удалась
        return 1; // Завершаем программу с ошибкой
    }

    // Устанавливаем диапазон ШИМ для каждого пина в 255.
    // Это означает, что значения от 0 до 255 будут соответствовать 0% до 100% рабочего цикла.
    gpioSetPWMrange(RED_PIN, 255);
    gpioSetPWMrange(GREEN_PIN, 255);
    gpioSetPWMrange(BLUE_PIN, 255);

    gtk_init(&argc, &argv); // Инициализация GTK. Должна быть вызвана первой.

    // --- Создание главного окна GTK ---
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "RGB LED Control (pigpio)"); // Заголовок окна
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300); // Размер окна по умолчанию
    // Подключаем сигнал "destroy" (закрытие окна) к функции gtk_main_quit для завершения приложения.
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // --- Создание основного вертикального контейнера ---
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Вертикальный контейнер с отступом 10px
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20); // Устанавливаем отступ от края окна для vbox
    gtk_container_add(GTK_CONTAINER(window), vbox); // Добавляем vbox в главное окно

    // --- Виджет для отображения цвета ---
    color_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // Создаем контейнер, который будет служить областью цвета
    gtk_widget_set_size_request(color_area, 200, 100); // Устанавливаем фиксированный размер
    gtk_widget_set_name(color_area, "color_display"); // Присваиваем CSS-идентификатор для стилизации
    gtk_box_pack_start(GTK_BOX(vbox), color_area, FALSE, FALSE, 0); // Добавляем в vbox
    gtk_widget_set_halign(color_area, GTK_ALIGN_CENTER); // Центрируем по горизонтали

    // --- Сетка для ползунков и меток ---
    GtkWidget *grid = gtk_grid_new(); // Создаем виджет сетки
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10); // Отступы между строками
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10); // Отступы между столбцами
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 10); // Добавляем сетку в vbox

    // --- Создание ползунков (GtkScale) и присвоение их глобальным переменным ---
    scale_r_global = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
    scale_g_global = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
    scale_b_global = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);

    // --- Создание меток для отображения значений R, G, B ---
    label_r = gtk_label_new("R: 0");
    label_g = gtk_label_new("G: 0");
    label_b = gtk_label_new("B: 0");

    // --- Размещение виджетов в сетке ---
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Red"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scale_r_global,      1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_r,             2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Green"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scale_g_global,      1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_g,             2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Blue"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scale_b_global,      1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_b,             2, 2, 1, 1);

    // --- Подключение сигналов к ползункам ---
    // Теперь user_data не нужен, можно передать NULL
    g_signal_connect(scale_r_global, "value-changed", G_CALLBACK(on_scale_changed), NULL);
    g_signal_connect(scale_g_global, "value-changed", G_CALLBACK(on_scale_changed), NULL);
    g_signal_connect(scale_b_global, "value-changed", G_CALLBACK(on_scale_changed), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    // --- Очистка ресурсов pigpio после завершения работы GUI ---
    gpioTerminate();
    return 0;
}
```

-----

## Сборка

Скомпилируйте приложение командой:

```bash
gcc rgb_pwm_gui.c -o rgb_pwm_gui $(pkg-config --cflags --libs gtk+-3.0) -lpigpio -lrt -pthread -Wall -Wextra
```

  * `-o rgb_pwm_gui`: Указывает имя выходного исполняемого файла.
  * `$(pkg-config --cflags --libs gtk+-3.0)`: Автоматически добавляет необходимые флаги компилятора (`--cflags`) и компоновщика (`--libs`) для GTK3.
  * `-lpigpio`: Компоновка с библиотекой pigpio.
  * `-lrt`: Компоновка с библиотекой реального времени (требуется для pigpio).
  * `-pthread`: Компоновка с библиотекой POSIX threads (требуется для pigpio).
  * `-Wall -Wextra`: Включает дополнительные предупреждения компилятора, что является хорошей практикой для обнаружения потенциальных проблем.

-----

## Запуск

Запустите скомпилированное приложение:

```bash
sudo ./rgb_pwm_gui
```

Требуется `sudo`, так как `pigpio` напрямую взаимодействует с аппаратными GPIO-пинами и требует соответствующих прав доступа.

-----

## Особенности

  * **Аппаратный ШИМ на 3 пинах:** Используется высокоточный ШИМ от библиотеки `pigpio` для плавного изменения яркости каждого цветового канала.
  * **Отображение цвета в прямоугольнике:** В окне приложения есть специальная область, которая визуально отображает текущий смешанный цвет.
  * **Подписи и числовые метки:** Каждый ползунок снабжен текстовой меткой ("Red", "Green", "Blue") и числовой меткой, показывающей текущее значение яркости (0-255).
  * **Центрированное окно и цветовая область:** Элементы интерфейса аккуратно расположены и центрированы для лучшего визуального восприятия.

-----

## Скриншот

-----

## Автор

**Автор:** *Ваше имя*
**Проект:** RGB LED Control via GTK and pigpio

```
```
