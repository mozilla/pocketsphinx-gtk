//
// Created by Andre Natal on 5/31/16.
//
#include <gtk/gtk.h>

#ifndef POCKETSPHINX_GTK_MAIN_H_H
#define POCKETSPHINX_GTK_MAIN_H_H

gboolean update_labels(gpointer *data);
gboolean change_btncolor(const gchar *color);
void render_gtk(int argc, char *argv[]);

typedef struct {
    char type;
    char lblvalue[512];
} lbl_s;

#endif //POCKETSPHINX_GTK_MAIN_H_H
