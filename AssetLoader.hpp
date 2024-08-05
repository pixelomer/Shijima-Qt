#pragma once
#include "Asset.hpp"
#include <QImage>
#include <QMap>

class AssetLoader
{
private:
    QMap<QString, Asset> m_assets;
    AssetLoader();
public:
    static AssetLoader *defaultLoader();
    static void finalize();
    Asset const& loadAsset(QString const& path);
};