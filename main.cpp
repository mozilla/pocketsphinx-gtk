#include <gtk/gtk.h>
#include <iostream>
#include "pocketsphinx_gtk.h"

GtkWidget *progress_bar;
pthread_t mic_thread;

void ClickCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data)
{
    // show which button was clicked
    pthread_create (&mic_thread, NULL, recognize_from_microphone, NULL);
    std::cerr << "button pressed: "  << gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progress_bar));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.01 + gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progress_bar)));
}

int render_gtk(int argc, char *argv[]){
    GtkWidget *window;
    GtkWidget *label;
    GtkWidget *button_start;
    GtkWidget* hbox;

    gtk_init(&argc, &argv);

    /* Create the main, top level window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Give it the title */
    gtk_window_set_title(GTK_WINDOW(window), "Vaani");

    /* Center the window */
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    /* Set the window's default size */
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    /*
    ** Map the destroy signal of the window to gtk_main_quit;
    ** When the window is about to be destroyed, we get a notification and
    ** stop the main GTK+ loop by returning 0
    */
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /*
    ** Assign the variable "label" to a new GTK label,
    ** with the text "Hello, world!"
    */
    label = gtk_label_new("Hello, world!");

    // Add the button onto the main window
    button_start = gtk_button_new_with_label ("Touch to start listening");

    hbox = gtk_hbox_new(true, 0);

    progress_bar = gtk_progress_bar_new ();

    gtk_container_add(GTK_CONTAINER(hbox), button_start);
    /* Plot the label onto the main window */
    gtk_container_add(GTK_CONTAINER(hbox), label);
    gtk_container_add(GTK_CONTAINER(hbox), progress_bar);

    // add the hbox to the window
    gtk_container_add(GTK_CONTAINER(window), hbox);
    g_signal_connect(G_OBJECT(button_start), "button_press_event", G_CALLBACK(ClickCallback), NULL);

    /* Make sure that everything, window and label, are visible */
    gtk_widget_show_all(window);

    /*
    ** Start the main loop, and do nothing (block) until
    ** the application is closed
    */
    gtk_main();
}



int main (int argc, char *argv[])
{
    pocketsphinxstart();
    render_gtk(argc, argv);
    destroy_ps();
    return 0;
}