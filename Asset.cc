#include "Asset.hpp"

QRect Asset::getRectForImage(QImage const& image) {
    int startX=0, endX=image.width(), startY=0, endY=image.height();
    int x, y;

    for (x=0; x<endX; ++x) {
        for (y=0; y<endY; ++y) {
            if (image.pixelColor(x, y).alpha() > 0) break;
        }
        if (y != endY) break;
    }
    startX = x;

    for (y=0; y<endY; ++y) {
        for (x=startX; x<endX; ++x) {
            if (image.pixelColor(x, y).alpha() > 0) break;
        }
        if (x != endX) break;
    }
    startY = y;

    for (x=endX-1; x>startX; --x) {
        for (y=startY; y<endY; ++y) {
            if (image.pixelColor(x, y).alpha() > 0) break;
        }
        if (y != endY) break;
    }
    endX = x+1;

    for (y=endY-1; y>startY; --y) {
        for (x=startX; x<endX; ++x) {
            if (image.pixelColor(x, y).alpha() > 0) break;
        }
        if (x != endX) break;
    }
    endY = y+1;

    return { startX, startY, endX - startX, endY - startY };
}

void Asset::setImage(QImage const& image) {
    m_originalSize = image.size();
    auto rect = getRectForImage(image);
    m_offset = rect;
    m_image = image.copy(rect);
    m_mirrored = m_image.mirrored(true, false);
}