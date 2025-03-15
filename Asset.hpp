#pragma once

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

#include <QImage>
#include <QRect>
#include <QBitmap>
#include <QPoint>

class Asset {
private:
    static QRect getRectForImage(QImage const& image);
    QRect m_offset;
    QSize m_originalSize;
    QImage m_image;
    QImage m_mirrored;
#ifdef __linux__
    QBitmap m_mask;
    QBitmap m_mirroredMask;
#endif
public:
    QRect const& offset() const { return m_offset; }
    QSize const& originalSize() const { return m_originalSize; }
    QImage const& image(bool mirrored) const { 
        return mirrored ? m_mirrored : m_image;
    }
#ifdef __linux__
    QBitmap const& mask(bool mirrored) const { 
        return mirrored ? m_mirroredMask : m_mask;
    }
#endif
    Asset() {}
    void setImage(QImage const& image);
};
