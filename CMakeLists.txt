cmake_minimum_required(VERSION 3.0)
project(pocketsphinx_gtk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp pocketsphinx_gtk.cpp pocketsphinx_gtk.h main.h)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK30 REQUIRED gtk+-3.0)

include_directories(${GTK30_INCLUDE_DIRS})
include_directories(/Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/include/ /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/include/sphinxbase/ /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/include/pocketsphinx/)
link_directories(${GTK30_LIBRARY_DIRS})

add_definitions(${GTK30_CFLAGS_OTHER})
add_executable(pocketsphinx_gtk ${SOURCE_FILES})

target_link_libraries(pocketsphinx_gtk ${GTK30_LIBRARIES} /System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/lib/libsphinxad.a /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/lib/libsphinxbase.a /Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/lib/libpocketsphinx.a )