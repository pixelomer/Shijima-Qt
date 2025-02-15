#include "../Platform.hpp"
#include <QWidget>
#include <AppKit/AppKit.h>

namespace Platform {

void initialize(int argc, char **argv) {}

void showOnAllDesktops(QWidget *widget) {
    NSView *view = (__bridge NSView *)((void *)widget->winId());
    NSWindow *window = [view window];
    [window setCollectionBehavior:[window collectionBehavior] |
        NSWindowCollectionBehaviorFullScreenAuxiliary |
        NSWindowCollectionBehaviorCanJoinAllSpaces];
}

}
