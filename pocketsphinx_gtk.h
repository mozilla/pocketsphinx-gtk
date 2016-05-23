//
// Created by Andre Natal on 5/23/16.
//

#include <pocketsphinx/pocketsphinx.h>


#ifndef POCKETSPHINX_GTK_POCKETSPHINX_H_H
#define POCKETSPHINX_GTK_POCKETSPHINX_H_H

int pocketsphinxstart();
void * recognize_from_microphone(void *args);
void destroy_ps();

#endif //POCKETSPHINX_GTK_POCKETSPHINX_H_H
