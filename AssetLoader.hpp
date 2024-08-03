#pragma once

#include <QImage>
#include <QMap>

class AssetLoader
{
private:
    QMap<QString, QImage> m_originals;
    QMap<QString, QImage> m_mirrored;
    AssetLoader();
public:
    static AssetLoader *defaultLoader();
    static void finalize();
    QImage const& loadImage(QString const& path, bool mirrorX);
};