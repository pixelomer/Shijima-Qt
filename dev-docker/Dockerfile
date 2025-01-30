FROM fedora:41

RUN dnf -y group install development-tools
RUN dnf -y install 'mingw32-qt6-*' 'mingw64-qt6-*' mingw32-gcc mingw32-gcc-c++ \
    mingw64-gcc mingw64-gcc-c++ 'qt6-*-devel'

WORKDIR /work