#include "AssetLoader.hpp"
#include "Asset.hpp"

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

Asset const& AssetLoader::loadAsset(QString const& path) {
    if (!m_assets.contains(path)) {
        Asset &asset = m_assets[path];
        QImage image;
        image.load(path);
        asset.setImage(image);
    }
    return m_assets[path];
}