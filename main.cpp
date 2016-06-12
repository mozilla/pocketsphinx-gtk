#include <gtk/gtk.h>
#include "pocketsphinx_gtk.h"
#include <glib/gprintf.h>
#include <sphinxbase/err.h>
#include "main.h"

pthread_t mic_thread;
GtkWidget *button_start;
GdkScreen *screen;
GtkCssProvider *provider;
GdkDisplay *display;
GtkWidget *label_kaldi, *label_ws, *label_off;
GtkWidget *grid;

int main (int argc, char *argv[])
{
    pocketsphinxstart();
    render_gtk(argc, argv);
    destroy_ps();
    return 0;
}


gboolean update_labels(gpointer *data) {
    E_INFO("UPDATE LABELS TIPO: %c \n",  ((lbl_s *)data)->type );
    E_INFO("UPDATE LABELS VALUE: %s \n",  ((lbl_s *)data)->lblvalue );

    switch(((lbl_s *)data)->type ) {
        // update lbl kaldi
        case 'k'  :
            gtk_label_set_label(GTK_LABEL(label_kaldi), ((lbl_s *)data)->lblvalue);
            break;
        // update lbl ws
        case 'w'  :
            gtk_label_set_label(GTK_LABEL(label_ws), ((lbl_s *)data)->lblvalue);
            break;
        // update lbl offline
        case 'o'  :
            gtk_label_set_label(GTK_LABEL(label_off), ((lbl_s *)data)->lblvalue);
            break;

        default :
            break;
    }
    return FALSE;
}


gboolean change_btncolor(const gchar *color){
    E_INFO("ADICIONANDO COR: %s \n", color);
    provider = gtk_css_provider_new ();
    gtk_style_context_add_provider_for_screen (screen,
                                               GTK_STYLE_PROVIDER (provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);
    gchar css[1024];
    g_sprintf(css,  " GtkWindow {\n"
                      "   background-color: %s;\n"
                      "}\n"
                      " #mybutton {\n"
                      "   -GtkWidget-focus-line-width: 0;\n"
                      "   border-radius: 15px;\n"
                      "   font: Sans 16;\n"
                      "   color: blue;\n"
                      "   border-style: outset;\n"
                      "   border-width: 2px;\n"
                      "   padding: 20px;\n"
                      "}\n"
                      " #label_kaldi {\n"
                      "   font-size: 30px;\n"
                      "   font-weight: bold;\n"
                      "}\n"
                      " #label_ws {\n"
                      "   font-size: 30px;\n"
                      "   font-weight: bold;\n"
                      "}\n"
            , color);
    gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider), css, -1, NULL);
    g_object_unref (provider);

    // we return false to remove the
    return FALSE;
}

void ClickCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data)
{
    // show which button was clicked
    if (change_decoder_state()){
        gdk_threads_add_idle((GSourceFunc)change_btncolor,(gpointer)"red");
        //change_btncolor("red");
        gtk_button_set_label(GTK_BUTTON(button_start), "Touch to start listening.");
    } else {
        gdk_threads_add_idle((GSourceFunc)change_btncolor,(gpointer)"yellow");
        //change_btncolor("yellow");
        gtk_button_set_label(GTK_BUTTON(button_start), "Listening...");
    }
}

void render_gtk(int argc, char *argv[]){
    GtkWidget *window;

    gtk_init(&argc, &argv);

    /* Create the main, top level window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Give it the title */
    gtk_window_set_title(GTK_WINDOW(window), "Vaani");

    /* Center the window */
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    /* Set the window's default size */
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    /* Create the main grid */
    grid = gtk_grid_new ();

    gtk_grid_set_row_spacing (GTK_GRID (grid), 20);

    // attach the grid to the main window
    gtk_container_add (GTK_CONTAINER (window), grid);

    // align the button on horizontal center
    gtk_widget_set_halign (GTK_WIDGET(grid),
                           GTK_ALIGN_CENTER);
    // align the button on vertical center
    gtk_widget_set_valign (GTK_WIDGET(grid),
                           GTK_ALIGN_CENTER);

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

    // label for ws results
    label_ws = gtk_label_new ("WS Result" );
    // add the label to the container
    gtk_widget_set_name (GTK_WIDGET(label_ws),
                         "label_ws");        /* name button so we can apply css to it later */
    gtk_grid_attach (GTK_GRID (grid), label_ws, 0, 1, 1, 1);

    // label for offline results
    label_off = gtk_label_new ("Offline Result" );
    // add the label to the container
    gtk_widget_set_name (GTK_WIDGET(label_off),
                         "label_ws");        /* name button so we can apply css to it later */
    gtk_grid_attach (GTK_GRID (grid), label_off, 0, 2, 1, 1);

    // label for kaldi results
    label_kaldi = gtk_label_new ("Kaldi Result" );
    // add the label to the container
    gtk_widget_set_name (GTK_WIDGET(label_kaldi),
                         "label_kaldi");        /* name button so we can apply css to it later */
    gtk_grid_attach (GTK_GRID (grid), label_kaldi, 0, 3, 1, 1);


    // create the main button
    button_start = gtk_button_new_with_label ("Touch to start listening.");
    gtk_widget_set_name (GTK_WIDGET(button_start),
                         "mybutton");        /* name button so we can apply css to it later */
    // set the button size
    gtk_widget_set_size_request (GTK_WIDGET(button_start),
                                 100, 75);
    // add the button to the grid
    gtk_grid_attach (GTK_GRID (grid), button_start, 0, 0, 2, 1);


    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    change_btncolor("red");
    g_signal_connect(G_OBJECT(button_start), "button_press_event", G_CALLBACK(ClickCallback), NULL);

    // start mike thread
    pthread_create(&mic_thread, NULL, recognize_from_microphone, NULL);

    /* Make sure that everything, window and label, are visible */
    gtk_widget_show_all(window);

    // start the gtk's main loop (block)
    gtk_main();
}

