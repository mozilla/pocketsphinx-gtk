cmake_minimum_required(VERSION 3.5)
project(pocketsphinx_gtk)

#include_directories(/usr/local/include/gtk-2.0/ /usr/local/Cellar/glib/2.46.2/include/glib-2.0/ /usr/local/Cellar/glib/2.46.2/lib/glib-2.0/include/ /usr/local/Cellar/cairo/1.14.6_1/include/cairo/ /usr/local/Cellar/pango/1.38.1/include/pango-1.0/ /usr/local/Cellar/gtk+/2.24.30/lib/gtk-2.0/include/ /usr/local/Cellar/gdk-pixbuf/2.32.3/include/gdk-pixbuf-2.0/ /usr/local/Cellar/atk/2.18.0_1/include/atk-1.0/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-2.0)

include_directories(${GTK3_INCLUDE_DIRS})
include_directories(/Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/include/ /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/include/sphinxbase/ /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/include/pocketsphinx/)
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})
add_executable(pocketsphinx_gtk ${SOURCE_FILES})

target_link_libraries(pocketsphinx_gtk ${GTK3_LIBRARIES} /System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/lib/libsphinxad.a /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/lib/libsphinxbase.a /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/lib/libpocketsphinx.a )