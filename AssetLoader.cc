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

        // Trim transparent lines from the image
        int y, width = image.width();
        for (y=image.height()-1; y>=128; --y) {
            int x;
            for (x=0; x<width; ++x) {
                if (image.pixelColor(x, y).alpha() > 0) {
                    break;
                }
            }
            if (x != width) {
                break;
            }
        }
        image = image.copy(0, 0, width, y+1);

        // Create a mirrored version of the image
        m_mirrored[path] = image.mirrored(true, false);
    }
    if (mirrorX) {
        return m_mirrored[path];
    }
    else {
        return m_originals[path];
    }
}