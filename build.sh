#!/usr/bin/env bash

shijima=(libshijima/shijima/parser.cc \
    libshijima/shijima/scripting/duktape/duktape.cc \
    libshijima/shijima/scripting/context.cc \
    libshijima/shijima/log.cc \
    libshijima/shijima/translator.cc)

c++() {
    echo c++ "${@}"
    command c++ "${@}"
}

c++ -Ilibshijima `pkg-config --cflags --libs Qt6Widgets` "${shijima[@]}" *.cc