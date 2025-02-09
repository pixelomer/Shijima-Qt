FROM fedora:42

RUN dnf -y group install development-tools
RUN dnf -y install mingw64-qt6-qtbase mingw64-qt6-qtmultimedia mingw64-gcc \
    mingw64-gcc-c++ qt6-qtmultimedia-devel qt6-qtbase-devel yarnpkg \
    mingw64-libarchive

WORKDIR /work
