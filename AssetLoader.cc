// 
// Shijima-Qt - Cross-platform shimeji simulation app for desktop
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

#include "AssetLoader.hpp"
#include "Asset.hpp"
#include "qdir.h"
#include "DefaultMascot.hpp"
#include <QDir>

AssetLoader::AssetLoader() {}

static AssetLoader *m_defaultLoader = nullptr;

AssetLoader *AssetLoader::defaultLoader() {
    if (m_defaultLoader == nullptr) {
        m_defaultLoader = new AssetLoader;
    }
    return m_defaultLoader;
}

void AssetLoader::finalize() {
    if (m_defaultLoader != nullptr) {
        delete m_defaultLoader;
        m_defaultLoader = nullptr;
    }
}

Asset const& AssetLoader::loadAsset(QString path) {
    path = QDir::cleanPath(path);
    if (!m_assets.contains(path)) {
        Asset &asset = m_assets[path];
        QImage image;
        if (path.startsWith("@")) {
            auto filename = path.sliced(path.lastIndexOf('/') + 1)
                .toStdString();
            if (defaultMascot.count(filename) == 1) {
                auto &file = defaultMascot.at(filename);
                image.loadFromData((const uchar *)file.first,
                    (int)file.second);
            }
        }
        else {
            image.load(path);
        }
        asset.setImage(image);
    }
    return m_assets[path];
}

void AssetLoader::unloadAssets(QString root) {
    root = QDir::cleanPath(root);
    for (auto &path : m_assets.keys()) {
        if (path.startsWith(root)) {
            m_assets.remove(path);
        }
    }
}