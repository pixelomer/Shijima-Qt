#pragma once
#include "ActiveWindow.hpp"
#include "ActiveWindowObserver.hpp"

class QWidget;

namespace Platform {

void initialize(int argc, char **argv);
void showOnAllDesktops(QWidget *widget);

}