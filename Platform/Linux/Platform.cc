#include "../Platform.hpp"

namespace Platform {

void initialize() {
    // Wayland does not allow windows to reposition themselves.
    // Set WAYLAND_DISPLAY to an invalid value to prevent its use.
    setenv("WAYLAND_DISPLAY", "", 1);
}

}