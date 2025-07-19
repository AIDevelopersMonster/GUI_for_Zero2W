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
    // Инициализация GTK. Должна быть вызвана первой.
    gtk_init(&argc, &argv);

    // --- Инициализация библиотеки pigpio ---
    // Это должно быть сделано перед любым использованием pigpio функций.
    // Если gpioInitialise() возвращает < 0, это означает ошибку (демон не запущен или недоступен).
    if (gpioInitialise() < 0) {
        g_printerr("Ошибка: Демон pigpiod не запущен или недоступен.\n");
        g_printerr("Пожалуйста, убедитесь, что pigpiod запущен (например, командой 'sudo pigpiod' или 'sudo systemctl start pigpiod').\n");

        // Создаем простое сообщение об ошибке для пользователя
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "Ошибка инициализации pigpio");
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
                                                 "Не удалось подключиться к демону pigpiod.\n"
                                                 "Убедитесь, что демон запущен и работает.\n"
                                                 "Попробуйте запустить 'sudo pigpiod' в терминале.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        return 1; // Завершаем программу с ошибкой
    }

    // Устанавливаем диапазон ШИМ для каждого пина в 255.
    // Это означает, что значения от 0 до 255 будут соответствовать 0% до 100% рабочего цикла.
    gpioSetPWMrange(RED_PIN, 255);
    gpioSetPWMrange(GREEN_PIN, 255);
    gpioSetPWMrange(BLUE_PIN, 255);

    // --- Создание главного окна GTK ---
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "RGB LED Control (pigpio)"); // Заголовок окна
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450); // Увеличиваем высоту окна, чтобы вместить ползунки
    // Подключаем сигнал "destroy" (закрытие окна) к функции gtk_main_quit для завершения приложения.
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // --- Создание основного вертикального контейнера ---
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Вертикальный контейнер с отступом 10px
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20); // Устанавливаем отступ от края окна для vbox
    gtk_container_add(GTK_CONTAINER(window), vbox); // Добавляем vbox в главное окно

    // --- Виджет для отображения цвета ---
    color_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // Создаем контейнер, который будет служить областью цвета
    gtk_widget_set_size_request(color_area, 250, 150); // Увеличиваем размер области цвета
    gtk_widget_set_name(color_area, "color_display"); // Присваиваем CSS-идентификатор для стилизации
    gtk_box_pack_start(GTK_BOX(vbox), color_area, FALSE, FALSE, 0); // Добавляем в vbox
    gtk_widget_set_halign(color_area, GTK_ALIGN_CENTER); // Центрируем по горизонтали

    // --- Сетка для ползунков и меток ---
    GtkWidget *grid = gtk_grid_new(); // Создаем виджет сетки
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15); // Увеличиваем отступы между строками
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15); // Увеличиваем отступы между столбцами
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 20); // Добавляем сетку в vbox, увеличиваем отступ

    // --- Создание ползунков (GtkScale) и присвоение их глобальным переменным ---
    scale_r_global = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
    scale_g_global = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
    scale_b_global = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);

    // Установка минимального размера для ползунков.
    // Ширина 250px для горизонтальных ползунков. Высота обычно регулируется автоматически.
    gtk_widget_set_size_request(scale_r_global, 250, -1);
    gtk_widget_set_size_request(scale_g_global, 250, -1);
    gtk_widget_set_size_request(scale_b_global, 250, -1);


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