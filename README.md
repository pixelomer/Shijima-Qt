# Shijima-Qt

Shijima application built with Qt6. Supports macOS, Linux and Windows.

## Platform Notes

### macOS

Shijima-Qt needs the Accessibility permission to access the frontmost window.

### Linux

Shijima-Qt supports KDE Plasma 6 and GNOME 46 in both Wayland and X11. To get the frontmost window, Shijima-Qt automatically installs and enables a shell plugin when started.  
- On KDE, this is transparent to the user.
- On GNOME, the shell needs to be restarted on the first run. This can be done by logging out and logging back in. Shijima-Qt will exit with an appropriate error message if this is required.

### Windows

Shijima-Qt is not optimized for Windows. Platform-specific code is stubbed and activeIE is not available. Use Shimeji-EE instead.