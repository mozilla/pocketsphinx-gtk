gcc `pkg-config --cflags --libs gtk+-3.0` -I/usr/include/gtk-3.0/ -I/usr/include/glib-2.0/ -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include/ -I/usr/include/pango-1.0/ -I/usr/include/cairo/ -I/usr/include/gdk-pixbuf-2.0/ -I/usr/include/atk-1.0/ `pkg-config --cflags --libs pocketsphinx sphinxbase` -lstdc++ -Llibiostream main.cpp pocketsphinx_gtk.cpp -o pocketsphinx_gtk