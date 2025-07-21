#include <gtk/gtk.h>
#include <gpiod.h>
#include <unistd.h>

#define CHIP_NAME "gpiochip0"
#define BUZZER_LINE 17

static struct gpiod_chip *chip;
static struct gpiod_line *line;
static int selected_melody = 1;

void buzzer_on() {
    gpiod_line_set_value(line, 1);
}

void buzzer_off() {
    gpiod_line_set_value(line, 0);
}

void play_melody(int melody) {
    switch (melody) {
        case 1:
            buzzer_on(); usleep(200000);
            buzzer_off(); usleep(100000);
            buzzer_on(); usleep(300000);
            buzzer_off();
            break;
        case 2:
            for (int i = 0; i < 4; ++i) {
                buzzer_on(); usleep(150000);
                buzzer_off(); usleep(150000);
            }
            break;
        case 3:
            buzzer_on(); usleep(500000);
            buzzer_off(); usleep(100000);
            buzzer_on(); usleep(500000);
            buzzer_off();
            break;
        default:
            break;
    }
}

void on_melody_selected(GtkToggleButton *button, gpointer user_data) {
    if (gtk_toggle_button_get_active(button)) {
        selected_melody = GPOINTER_TO_INT(user_data);
    }
}

void on_play_clicked(GtkButton *button, gpointer user_data) {
    play_melody(selected_melody);
}

void on_destroy(GtkWidget *widget, gpointer data) {
    buzzer_off();
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        g_printerr("Failed to open GPIO chip\n");
        return 1;
    }

    line = gpiod_chip_get_line(chip, BUZZER_LINE);
    if (!line || gpiod_line_request_output(line, "buzzer", 0) < 0) {
        g_printerr("Failed to get/request line\n");
        return 1;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Buzzer Melody");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *rb1 = gtk_radio_button_new_with_label(NULL, "Мелодия 1");
    GtkWidget *rb2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb1), "Мелодия 2");
    GtkWidget *rb3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb1), "Мелодия 3");

    g_signal_connect(rb1, "toggled", G_CALLBACK(on_melody_selected), GINT_TO_POINTER(1));
    g_signal_connect(rb2, "toggled", G_CALLBACK(on_melody_selected), GINT_TO_POINTER(2));
    g_signal_connect(rb3, "toggled", G_CALLBACK(on_melody_selected), GINT_TO_POINTER(3));

    GtkWidget *play_btn = gtk_button_new_with_label("Проиграть");
    g_signal_connect(play_btn, "clicked", G_CALLBACK(on_play_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), rb1, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), rb2, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), rb3, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), play_btn, FALSE, FALSE, 10);

    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
