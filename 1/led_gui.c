#include <gtk/gtk.h> // Библиотека GTK для создания GUI
#include <gpiod.h>   // Библиотека libgpiod для работы с GPIO
#include <stdio.h>   // Стандартная библиотека ввода/вывода

#define CONSUMER "GUI_for_Zero2W" // Имя потребителя для линии GPIO
#define CHIPNAME "gpiochip0"      // Имя GPIO-чипа (стандартно для RPi)
#define LED_LINE 17               // Номер GPIO-пина для светодиода (GPIO17, пин 11 на RPi)

// Структура для хранения указателей на виджеты и состояние светодиода
struct app_widgets {
    GtkWidget *button;          // Указатель на кнопку
    struct gpiod_line *line;    // Указатель на линию GPIO
    int led_on;                 // Переменная для отслеживания состояния светодиода (0 = выключен, 1 = включен)
};

// Функция, вызываемая при нажатии на кнопку
static void toggle_led(GtkButton *button, gpointer user_data) {
    // Приводим user_data к типу нашей структуры
    struct app_widgets *widgets = (struct app_widgets *)user_data;

    // Инвертируем состояние светодиода
    widgets->led_on = !widgets->led_on;
    // Устанавливаем значение на линии GPIO (включаем/выключаем светодиод)
    gpiod_line_set_value(widgets->line, widgets->led_on);

    // Меняем текст на кнопке в зависимости от состояния светодиода
    if (widgets->led_on)
        gtk_button_set_label(button, "Выключить LED");
    else
        gtk_button_set_label(button, "Включить LED");
}

int main(int argc, char *argv[]) {
    // Инициализация GTK. Это всегда должно быть первой строкой в GTK-приложении.
    gtk_init(&argc, &argv);

    struct gpiod_chip *chip; // Указатель на GPIO-чип
    struct gpiod_line *line; // Указатель на конкретную линию GPIO

    // Открываем GPIO-чип по его имени
    chip = gpiod_chip_open_by_name(CHIPNAME);
    if (!chip) {
        perror("Не удалось открыть gpiochip"); // Выводим сообщение об ошибке, если не удалось открыть чип
        return 1;
    }

    // Получаем конкретную линию GPIO по ее номеру
    line = gpiod_chip_get_line(chip, LED_LINE);
    if (!line) {
        perror("Не удалось получить линию GPIO"); // Выводим сообщение об ошибке
        gpiod_chip_close(chip); // Закрываем чип перед выходом
        return 1;
    }

    // Запрашиваем линию GPIO как выходную с начальным значением 0 (выключено)
    if (gpiod_line_request_output(line, CONSUMER, 0) < 0) {
        perror("Не удалось запросить линию для вывода"); // Выводим сообщение об ошибке
        gpiod_chip_close(chip); // Закрываем чип перед выходом
        return 1;
    }

    // Инициализируем структуру app_widgets
    struct app_widgets widgets = {0}; // Обнуляем структуру
    widgets.line = line;             // Сохраняем указатель на линию GPIO
    widgets.led_on = 0;              // Изначально светодиод выключен

    // Создаем новое окно верхнего уровня
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // Устанавливаем заголовок окна
    gtk_window_set_title(GTK_WINDOW(window), "LED Toggle");
    // Устанавливаем размер окна по умолчанию
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    // Подключаем сигнал "destroy" окна к функции gtk_main_quit для завершения приложения при закрытии окна
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Создаем кнопку с начальным текстом
    widgets.button = gtk_button_new_with_label("Включить LED");
    // Подключаем сигнал "clicked" кнопки к нашей функции toggle_led
    g_signal_connect(widgets.button, "clicked", G_CALLBACK(toggle_led), &widgets);
    // Добавляем кнопку в окно
    gtk_container_add(GTK_CONTAINER(window), widgets.button);

    // Показываем все виджеты в окне
    gtk_widget_show_all(window);

    // Запускаем основной цикл GTK. Здесь приложение будет ожидать событий (нажатий кнопок, закрытия окна и т.д.).
    gtk_main();

    // --- Очистка ресурсов после выхода из gtk_main ---

    // Выключаем LED перед выходом из программы, чтобы он не остался включенным
    gpiod_line_set_value(line, 0);

    // Освобождаем линию GPIO
    gpiod_line_release(line);
    // Закрываем GPIO-чип
    gpiod_chip_close(chip);

    return 0; // Успешное завершение программы
}