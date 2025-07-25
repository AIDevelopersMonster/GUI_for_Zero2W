# Глава 7. Графический интерфейс для работы с LCD1602 (I²C)

В этой главе мы разработаем и подключим графический интерфейс (GUI) для вывода текстовой информации на дисплей **LCD1602**, подключённый по интерфейсу **I²C** к Raspberry Pi Zero 2W.

## Цель

Создать удобный GTK-интерфейс, позволяющий:

* Вводить произвольный текст в две строки

* Отправлять его на LCD1602 нажатием кнопки

* Управлять очисткой экрана

* Тестировать инициализацию дисплея и отображать статус

## Подключение

LCD1602 должен быть подключён к I²C-шине Raspberry Pi:

* **SCL1** → GPIO3 (Physical pin 5)

* **SDA1** → GPIO2 (Physical pin 3)

* **VCC** → 5V (Physical pin 2)

* **GND** → GND (Physical pin 6)

Убедитесь, что модуль I²C активирован:

```bash
sudo raspi-config  # → Interface Options → I2C → Enable
sudo apt install i2c-tools
i2cdetect -y 1     # Убедитесь, что адрес (обычно 0x27) виден
Исходные файлыlcd1602.h, lcd1602.c — простой драйвер для инициализации и записи текста на LCD1602 по I²Clcd_gui.c — GTK-приложение с двумя строками ввода, кнопками «Отобразить» и «Очистить экран», а также меткой статуса.Makefile — сборка проектаФункциональность GUIПоле ввода для строки 1Поле ввода для строки 2Кнопка Отобразить — отправляет текст на дисплейКнопка Очистить экран — очищает содержимое дисплеяМетка состояния — отображает статус инициализации и отправки текста (успешно/ошибка)Файл: lcd_gui.c#include <gtk/gtk.h> // Включаем библиотеку GTK+ 3 для создания графического интерфейса
#include "lcd1602.h" // Включаем наш заголовочный файл драйвера LCD1602

/**
 * @file lcd_gui.c
 * @brief Пример GTK-приложения для управления дисплеем LCD1602 через I2C.
 *
 * Это приложение предоставляет простой графический интерфейс пользователя (GUI)
 * для ввода двух строк текста и отправки их на дисплей LCD1602.
 * Оно использует драйвер lcd1602.h/lcd1602.c для взаимодействия с аппаратным обеспечением.
 *
 * @note Предназначено для использования на Raspberry Pi OS с Raspberry Pi Zero 2 W.
 */

// Глобальные указатели на виджеты GtkEntry для доступа к ним из разных функций.
// Это необходимо, так как мы будем получать текст из этих полей ввода.
GtkWidget *entry_line1;
GtkWidget *entry_line2;
// Глобальный указатель на метку для отображения статуса
GtkWidget *status_label;

/**
 * @brief Обработчик события нажатия кнопки "Отправить на LCD".
 *
 * Эта функция вызывается, когда пользователь нажимает кнопку "Отправить на LCD".
 * Она считывает текст из полей ввода, очищает дисплей и записывает на него новые строки.
 *
 * @param button Указатель на виджет кнопки, которая вызвала событие (не используется напрямую).
 * @param user_data Пользовательские данные, переданные при подключении сигнала (не используются).
 */
void on_send_clicked(GtkButton *button, gpointer user_data) {
    const char *text1 = gtk_entry_get_text(GTK_ENTRY(entry_line1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(entry_line2));

    // Очищаем дисплей LCD1602
    lcd1602_clear();
    // Записываем полученные строки на дисплей
    lcd1602_write(text1, text2);
    // Обновляем метку статуса
    gtk_label_set_text(GTK_LABEL(status_label), "Текст успешно отправлен на LCD.");
}

/**
 * @brief Обработчик события нажатия кнопки "Очистить экран".
 *
 * Эта функция вызывается, когда пользователь нажимает кнопку "Очистить экран".
 * Она очищает содержимое дисплея LCD1602.
 *
 * @param button Указатель на виджет кнопки, которая вызвала событие.
 * @param user_data Пользовательские данные.
 */
void on_clear_clicked(GtkButton *button, gpointer user_data) {
    lcd1602_clear();
    gtk_label_set_text(GTK_LABEL(status_label), "Экран LCD очищен.");
}

/**
 * @brief Главная функция приложения.
 *
 * Инициализирует GTK, инициализирует LCD, создает и настраивает GUI,
 * запускает главный цикл GTK и очищает ресурсы при завершении.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return 0 при успешном завершении, 1 при ошибке инициализации LCD.
 */
int main(int argc, char *argv[]) {
    // Инициализация GTK. Должна быть вызвана первой в любом GTK-приложении.
    gtk_init(&argc, &argv); // ИСПРАВЛЕНО: argv передается без квадратных скобок.

    // --- Создание главного окна приложения ---
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // Создаем новое окно верхнего уровня
    gtk_window_set_title(GTK_WINDOW(window), "LCD1602 Controller"); // Устанавливаем заголовок окна
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 250); // Устанавливаем размер окна по умолчанию
    // Подключаем сигнал "destroy" (закрытие окна) к функции gtk_main_quit,
    // которая завершает главный цикл GTK и, соответственно, приложение.
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // --- Контейнеры для виджетов ---
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8); // Главный вертикальный контейнер
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // Контейнер для кнопок
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    // --- Создание и добавление поля ввода для первой строки ---
    entry_line1 = gtk_entry_new(); // Создаем новое поле ввода текста
    // Устанавливаем текст-заполнитель, который отображается, когда поле пустое
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_line1), "Текст первой строки (до 16 символов)");
    // Добавляем поле ввода в контейнер vbox.
    // FALSE, FALSE, 0 означает, что виджет не будет расширяться и заполнять доступное пространство.
    gtk_box_pack_start(GTK_BOX(vbox), entry_line1, FALSE, FALSE, 0);

    // --- Создание и добавление поля ввода для второй строки ---
    entry_line2 = gtk_entry_new(); // Создаем новое поле ввода текста
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_line2), "Текст второй строки (до 16 символов)");
    gtk_box_pack_start(GTK_BOX(vbox), entry_line2, FALSE, FALSE, 0);

    // --- Кнопка "Отобразить" ---
    GtkWidget *send_button = gtk_button_new_with_label("Отобразить"); // Создаем кнопку с текстом
    // Подключаем сигнал "clicked" (нажатие кнопки) к нашей функции on_send_clicked
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), send_button, TRUE, TRUE, 0); // Добавляем в контейнер кнопок

    // --- Кнопка "Очистить экран" ---
    GtkWidget *clear_button = gtk_button_new_with_label("Очистить экран");
    g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), clear_button, TRUE, TRUE, 0); // Добавляем в контейнер кнопок

    // --- Метка статуса ---
    status_label = gtk_label_new("Ожидание инициализации LCD...");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);

    // Инициализация LCD1602.
    // Используем "/dev/i2c-1" для I2C-шины на Raspberry Pi и адрес 0x27 для PCF8574.
    if (lcd1602_init("/dev/i2c-1", 0x27) != 0) {
        // Если инициализация LCD не удалась, выводим сообщение об ошибке
        g_printerr("Ошибка инициализации LCD. Убедитесь, что I2C включен и адрес 0x27 корректен.\n");
        gtk_label_set_text(GTK_LABEL(status_label), "Ошибка инициализации LCD!");
        // Не возвращаем 1 сразу, чтобы GUI все равно показался, но с ошибкой.
        // return 1;
    } else {
        gtk_label_set_text(GTK_LABEL(status_label), "LCD успешно инициализирован.");
    }


    // Показываем все виджеты в окне (окно и все его дочерние элементы)
    gtk_widget_show_all(window);

    // Запускаем главный цикл GTK.
    // Приложение будет ожидать событий (нажатия кнопок, ввод текста и т.д.)
    // до тех пор, пока не будет вызвана gtk_main_quit (например, при закрытии окна).
    gtk_main();

    // После завершения главного цикла GTK, закрываем I2C-соединение с LCD.
    lcd1602_close();
    return 0; // Успешное завершение программы
}

Файл: Makefile# @file Makefile
# @brief Makefile для сборки GTK-приложения и драйвера LCD1602.
#
# Этот Makefile автоматизирует процесс компиляции исходных файлов C
# (lcd_gui.c и lcd1602.c) и компоновки их в исполняемый файл 'lcd_gui'.
# Он использует pkg-config для автоматического определения флагов компиляции и
# библиотек GTK+ 3.
#
# @note Предназначен для использования на системах с установленным GTK+ 3 и GCC.

# Компилятор C
CC = gcc

# Флаги компилятора:
# `pkg-config --cflags gtk+-3.0` - автоматически добавляет необходимые флаги для заголовочных файлов GTK+ 3.
# -Wall - включает все предупреждения компилятора, что помогает писать более чистый и безопасный код.
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall

# Библиотеки для компоновки:
# `pkg-config --libs gtk+-3.0` - автоматически добавляет необходимые библиотеки GTK+ 3 для компоновки.
LIBS = `pkg-config --libs gtk+-3.0`

# Цель по умолчанию: 'all'. При вызове 'make' без аргументов, будет выполнена эта цель.
all: lcd_gui

# Цель 'lcd_gui': Компонует объектные файлы в конечный исполняемый файл.
# Зависимости: lcd_gui.o и lcd1602.o (объектные файлы).
lcd_gui: lcd_gui.o lcd1602.o
	$(CC) -o lcd_gui lcd_gui.o lcd1602.o $(LIBS)

# Цель 'lcd_gui.o': Компилирует lcd_gui.c в объектный файл lcd_gui.o.
# Зависимости: lcd_gui.c и lcd1602.h (если lcd1602.h изменится, lcd_gui.c будет перекомпилирован).
lcd_gui.o: lcd_gui.c lcd1602.h
	$(CC) $(CFLAGS) -c lcd_gui.c -o lcd_gui.o

# Цель 'lcd1602.o': Компилирует lcd1602.c в объектный файл lcd1602.o.
# Зависимости: lcd1602.c и lcd1602.h.
lcd1602.o: lcd1602.c lcd1602.h
	$(CC) -c lcd1602.c

# Цель 'clean': Удаляет все сгенерированные объектные файлы и исполняемый файл.
# Полезно для "чистой" пересборки проекта.
clean:
	rm -f *.o lcd_gui

Пример использованияСоберите проект:make
Запустите:./lcd_gui
Введите текст