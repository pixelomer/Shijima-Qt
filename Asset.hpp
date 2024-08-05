#pragma once
#include <QImage>
#include <QRect>
#include <QPoint>

class Asset {
private:
    static QRect getRectForImage(QImage const& image);
    QRect m_offset;
    QSize m_originalSize;
    QImage m_image;
    QImage m_mirrored;
public:
    QRect const& offset() const { return m_offset; }
    QSize const& originalSize() const { return m_originalSize; }
    QImage const& image(bool mirrored) const { 
        return mirrored ? m_mirrored : m_image;
    }
    Asset() {}
    void setImage(QImage const& image);
};