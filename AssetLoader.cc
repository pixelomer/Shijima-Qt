#include "AssetLoader.hpp"

AssetLoader::AssetLoader() {}

AssetLoader *AssetLoader::defaultLoader() {
    static AssetLoader *instance = nullptr;
    if (instance == nullptr) {
        instance = new AssetLoader;
    }
    return instance;
}

QImage const& AssetLoader::loadImage(QString const& path, bool mirrorX) {
    if (!m_originals.contains(path)) {
        QImage &image = m_originals[path];
        image.load(path);
        m_mirrored[path] = image.mirrored(true, false);
    }
    if (mirrorX) {
        return m_mirrored[path];
    }
    else {
        return m_originals[path];
    }
}