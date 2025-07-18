 #include <gtk/gtk.h>       // Подключаем библиотеку GTK для создания графического интерфейса
    #include <gpiod.h>         // Подключаем библиотеку libgpiod для работы с GPIO
    #include <stdlib.h>        // Подключаем стандартную библиотеку для функций, таких как rand() и atoi()
    #include <time.h>          // Подключаем библиотеку для работы со временем, используемой для инициализации rand()
    #include <stdio.h>         // Подключаем для snprintf и perror

    // Определение констант для количества светодиодов, имени GPIO-чипа и потребителя
    #define NUM_LEDS 8
    #define CHIPNAME "gpiochip0"
    #define CONSUMER "BinaryGame"

    // Массив номеров GPIO-пинов, к которым подключены светодиоды
    // Порядок пинов соответствует порядку битов от LSB (индекс 0) до MSB (индекс 7)
    int gpio_pins[NUM_LEDS] = {4, 25, 24, 23, 22, 27, 18, 17};

    // Глобальные переменные для хранения состояния приложения и указателей на виджеты/GPIO
    // Использование глобальных переменных упрощает передачу данных между функциями в данном примере.
    int current_value = 0;              // Текущее загаданное десятичное число
    int correct = 0;                    // Счетчик правильных ответов
    int incorrect = 0;                  // Счетчик ошибок
    struct gpiod_chip *chip;            // Указатель на GPIO-чип
    struct gpiod_line *lines[NUM_LEDS]; // Массив указателей на линии GPIO для светодиодов
    GtkWidget *circles[NUM_LEDS];       // Массив указателей на виджеты-контейнеры для GUI-индикаторов
    GtkWidget *entry;                   // Указатель на виджет поля ввода (GtkEntry)
    GtkWidget *correct_label;           // Указатель на лейбл для отображения количества правильных ответов (GtkLabel)
    GtkWidget *incorrect_label;         // Указатель на лейбл для отображения количества ошибок (GtkLabel)
    gboolean game_running = FALSE;      // Флаг состояния игры (TRUE - игра активна, FALSE - нет)


    // Функция update_led_circle: Обновляет цвет GUI-индикатора светодиода.
    // i: индекс светодиода (от 0 до 7)
    // state: 0 для выключенного (белый), 1 для включенного (красный)
    void update_led_circle(int i, int state) {
        GdkRGBA color;
        // gdk_rgba_parse преобразует строковое название цвета в структуру GdkRGBA
        // Если state равен 1, цвет будет "red", иначе "white".
        gdk_rgba_parse(&color, state ? "red" : "white");
        // Устанавливаем фоновый цвет для виджета (GtkEventBox), который имитирует светодиод.
        gtk_widget_override_background_color(circles[i], GTK_STATE_FLAG_NORMAL, &color);
    }

    // Функция set_leds: Устанавливает состояние физических светодиодов и их GUI-индикаторов
    // в соответствии с двоичным представлением заданного числа.
    // value: десятичное число, которое нужно отобразить.
    void set_leds(int value) {
        current_value = value; // Сохраняем текущее загаданное число.
        for (int i = 0; i < NUM_LEDS; i++) {
            // Извлекаем i-й бит из числа 'value'.
            // Оператор >> (побитовый сдвиг вправо) сдвигает биты числа на 'i' позиций вправо.
            // Оператор & 1 (побитовое И с 1) извлекает самый младший бит (то есть, текущий i-й бит).
            int bit = (value >> i) & 1;
            // Устанавливаем значение на соответствующей физической GPIO-линии.
            // 0 - выключить светодиод, 1 - включить светодиод.
            gpiod_line_set_value(lines[i], bit);
            // Обновляем цвет GUI-индикатора.
            update_led_circle(i, bit);
        }
    }

    // Функция reset_all_leds: Выключает все физические светодиоды и их GUI-индикаторы.
    void reset_all_leds() {
        set_leds(0); // Вызываем set_leds с значением 0, чтобы выключить все светодиоды.
    }

    // Обработчик кнопки "Старт": Начинает новую игру.
    // Параметр gpointer user_data удален, так как функция использует глобальные переменные.
    void start_game(GtkButton *btn) {
        // Генерируем случайное число от 0 до 255 (для 8 бит).
        // rand() % 256 дает остаток от деления на 256, что гарантирует число в диапазоне [0, 255].
        int value = rand() % 256;
        set_leds(value); // Обновляем светодиоды в соответствии с новым загаданным числом.
        game_running = TRUE; // Устанавливаем флаг, что игра запущена.
        gtk_entry_set_text(GTK_ENTRY(entry), ""); // Очищаем текстовое поле ввода.
    }

    // Обработчик кнопки "Ваш ответ": Проверяет ответ пользователя.
    // Параметр gpointer user_data удален, так как функция использует глобальные переменные.
    void check_answer(GtkButton *btn) {
        // Если игра не запущена, игнорируем нажатие кнопки "Ваш ответ".
        if (!game_running) return;

        // Получаем текст из поля ввода.
        const gchar *input_text = gtk_entry_get_text(GTK_ENTRY(entry));
        // Преобразуем введенную строку в целое число.
        int user_val = atoi(input_text);

        // Сравниваем ответ пользователя с текущим загаданным числом.
        if (user_val == current_value) {
            correct++; // Увеличиваем счетчик правильных ответов.
        } else {
            incorrect++; // Увеличиваем счетчик ошибок.
        }

        // Обновляем текст на лейблах, отображающих статистику.
        char buf[64]; // Буфер для форматирования строк
        snprintf(buf, sizeof(buf), "Правильных: %d", correct);
        gtk_label_set_text(GTK_LABEL(correct_label), buf);

        snprintf(buf, sizeof(buf), "Ошибок: %d", incorrect);
        gtk_label_set_text(GTK_LABEL(incorrect_label), buf);

        // Генерируем новое случайное число для следующего раунда игры.
        int new_value = rand() % 256;
        set_leds(new_value); // Обновляем светодиоды для нового числа.
        gtk_entry_set_text(GTK_ENTRY(entry), ""); // Очищаем поле ввода для нового ответа.
    }

    // Обработчик кнопки "Стоп": Завершает текущую игру.
    // Параметр gpointer user_data удален, так как функция использует глобальные переменные.
    void stop_game(GtkButton *btn) {
        reset_all_leds(); // Выключаем все светодиоды.
        game_running = FALSE; // Устанавливаем флаг, что игра остановлена.
        // Сбрасываем счетчики правильных и неправильных ответов.
        correct = 0;
        incorrect = 0;
        // Обновляем лейблы статистики.
        gtk_label_set_text(GTK_LABEL(correct_label), "Правильных: 0");
        gtk_label_set_text(GTK_LABEL(incorrect_label), "Ошибок: 0");
        gtk_entry_set_text(GTK_ENTRY(entry), ""); // Очищаем поле ввода.
    }

    // Обработчик сигнала "destroy" окна: Освобождает все захваченные ресурсы и завершает приложение.
    // Параметр gpointer data удален, так как функция использует глобальные переменные.
    void on_destroy(GtkWidget *widget) {
        reset_all_leds(); // Убедимся, что все светодиоды выключены перед выходом.

        // Освобождаем каждую GPIO-линию, которая была запрошена.
        for (int i = 0; i < NUM_LEDS; i++) {
            if (lines[i]) { // Проверяем, что указатель на линию не NULL (т.е. линия была успешно получена)
                gpiod_line_release(lines[i]);
            }
        }

        // Закрываем GPIO-чип.
        if (chip) { // Проверяем, что указатель на чип не NULL (т.е. чип был успешно открыт)
            gpiod_chip_close(chip);
        }
        gtk_main_quit(); // Завершаем основной цикл обработки событий GTK, что приводит к завершению приложения.
    }

    // Главная функция приложения, точка входа.
    int main(int argc, char *argv[]) {
        gtk_init(&argc, &argv); // Инициализация библиотеки GTK. Это должно быть вызвано в начале.
        srand(time(NULL)); // Инициализация генератора случайных чисел.
                           // time(NULL) возвращает текущее время, обеспечивая разную последовательность чисел при каждом запуске.

        // Инициализация GPIO
        chip = gpiod_chip_open("/dev/gpiochip0"); // Открываем GPIO-чип по его пути.
        // Альтернативный вариант: chip = gpiod_chip_open_by_name(CHIPNAME);
        if (!chip) {
            perror("Ошибка: не удалось открыть GPIO chip"); // Выводим сообщение об ошибке, если чип не открылся.
            return 1; // Завершаем программу с кодом ошибки.
        }

        // Запрашиваем и настраиваем все 8 GPIO-линий как выходы.
        for (int i = 0; i < NUM_LEDS; i++) {
            // Получаем указатель на конкретную GPIO-линию по ее номеру.
            lines[i] = gpiod_chip_get_line(chip, gpio_pins[i]);
            // Запрашиваем линию как выход с начальным значением 0 (выключено).
            // Проверяем, что линия получена и запрос успешен.
            if (!lines[i] || gpiod_line_request_output(lines[i], CONSUMER, 0) < 0) {
                perror("Ошибка: не удалось настроить пины"); // Сообщение об ошибке.
                // В случае ошибки, освобождаем все линии, которые были успешно запрошены до этого.
                for (int j = 0; j < i; j++) {
                    if (lines[j]) gpiod_line_release(lines[j]);
                }
                gpiod_chip_close(chip); // Закрываем чип.
                return 1; // Завершаем программу.
            }
        }

        // Создание и настройка GTK интерфейса
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // Создаем главное окно приложения.
        gtk_window_set_title(GTK_WINDOW(window), "Игра: Двоичное Число"); // Устанавливаем заголовок окна.
        gtk_window_set_default_size(GTK_WINDOW(window), 400, 300); // Устанавливаем размер окна по умолчанию.
        gtk_container_set_border_width(GTK_CONTAINER(window), 10); // Устанавливаем отступ от краев окна.
        // Подключаем обработчик сигнала "destroy" (закрытие окна) к функции on_destroy.
        // Передаем NULL, так как on_destroy теперь работает с глобальными переменными.
        g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Создаем вертикальный контейнер с отступом 10 пикселей между элементами.
        gtk_container_add(GTK_CONTAINER(window), vbox); // Добавляем контейнер в главное окно.

        // Контейнер для светодиодов (визуальное представление битов)
        GtkWidget *led_grid = gtk_grid_new(); // Используем GtkGrid для размещения индикаторов.
        gtk_grid_set_row_spacing(GTK_GRID(led_grid), 10); // Устанавливаем вертикальный отступ между рядами (на всякий случай).
        gtk_grid_set_column_spacing(GTK_GRID(led_grid), 10); // Устанавливаем ГОРИЗОНТАЛЬНЫЙ ОТСТУП между столбцами.

        // Создаем 8 виджетов-контейнеров (GtkEventBox), которые будут имитировать светодиоды.
        // Идем от 0 к NUM_LEDS-1, чтобы младший бит (LSB) был слева в GUI.
        // Для отображения MSB слева, LSB справа, нужно инвертировать индекс столбца.
        for (int i = 0; i < NUM_LEDS; i++) { // Итерация от LSB к MSB
            circles[i] = gtk_event_box_new(); // Создаем GtkEventBox - это контейнер, который может иметь фон.
            gtk_widget_set_size_request(circles[i], 30, 30); // Устанавливаем фиксированный размер для каждого "светодиода".
            update_led_circle(i, 0); // Изначально все "светодиоды" выключены (белые).
            // Прикрепляем виджет к сетке. (NUM_LEDS - 1) - i используется для того, чтобы circles[7] (MSB) оказался в столбце 0 (самый левый),
            // а circles[0] (LSB) оказался в столбце 7 (самый правый).
            gtk_grid_attach(GTK_GRID(led_grid), circles[i], (NUM_LEDS - 1) - i, 0, 1, 1);
        }
        gtk_box_pack_start(GTK_BOX(vbox), led_grid, FALSE, FALSE, 0); // Добавляем контейнер светодиодов в основной вертикальный контейнер.

        // Поле ввода для ответа пользователя
        entry = gtk_entry_new(); // Создаем новое текстовое поле ввода.
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Введите число"); // Устанавливаем текст-подсказку.
        gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5); // Добавляем поле ввода в основной контейнер с отступом.

        // Контейнер для кнопок управления игрой
        GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // Горизонтальный контейнер для кнопок с отступом 5 пикселей.
        GtkWidget *btn_start = gtk_button_new_with_label("Старт"); // Создаем кнопку "Старт".
        GtkWidget *btn_check = gtk_button_new_with_label("Ваш ответ"); // Создаем кнопку "Ваш ответ".
        GtkWidget *btn_stop = gtk_button_new_with_label("Стоп"); // Создаем кнопку "Стоп".

        // Подключаем обработчики нажатий кнопок.
        // Передаем NULL, так как функции теперь работают с глобальными переменными.
        g_signal_connect(btn_start, "clicked", G_CALLBACK(start_game), NULL);
        g_signal_connect(btn_check, "clicked", G_CALLBACK(check_answer), NULL);
        g_signal_connect(btn_stop, "clicked", G_CALLBACK(stop_game), NULL);

        gtk_box_pack_start(GTK_BOX(button_box), btn_start, TRUE, TRUE, 0); // Добавляем кнопку "Старт".
        gtk_box_pack_start(GTK_BOX(button_box), btn_check, TRUE, TRUE, 0); // Добавляем кнопку "Ваш ответ".
        gtk_box_pack_start(GTK_BOX(button_box), btn_stop, TRUE, TRUE, 0); // Добавляем кнопку "Стоп".

        gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 5); // Добавляем контейнер кнопок в основной контейнер с отступом.

        // Лейблы для отображения статистики (правильных/неправильных ответов)
        correct_label = gtk_label_new("Правильных: 0"); // Создаем лейбл для правильных ответов.
        incorrect_label = gtk_label_new("Ошибок: 0");       // Создаем лейбл для ошибок.
        gtk_box_pack_start(GTK_BOX(vbox), correct_label, FALSE, FALSE, 2); // Добавляем лейбл в основной контейнер с отступом.
        gtk_box_pack_start(GTK_BOX(vbox), incorrect_label, FALSE, FALSE, 2);   // Добавляем лейбл в основной контейнер с отступом.

        gtk_widget_show_all(window); // Отображаем все виджеты, содержащиеся в окне.
        gtk_main(); // Запускаем основной цикл обработки событий GTK. Приложение будет активно до вызова gtk_main_quit().

        return 0; // Успешное завершение программы.
    }