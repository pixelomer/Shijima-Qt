#include "AssetLoader.hpp"
#include "Asset.hpp"
#include "qdir.h"
#include "ShijimaManager.hpp"
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
        image.load(path);
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