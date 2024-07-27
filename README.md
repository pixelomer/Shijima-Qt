# Shijima-Qt

Shijima application built with Qt6. Supports macOS, Linux and Windows.

Platform notes:  
- **macOS:** Fully supported.
- **Linux:** Only Wayland KDE is supported. Shijima-Qt runs through XWayland and communicates with KWin to interact with other windows.
- **Windows:** Not actively worked on. Known to be laggy. Platform-specific code is stubbed. activeIE is not available. Use Shimeji-EE instead.

## Notes

Qt6 is a heavy framework. It consumes more resources than needed and takes up a lot of disk space. If possible, use a version of Shijima built specifically for your platform.