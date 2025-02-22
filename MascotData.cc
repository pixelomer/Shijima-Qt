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

#include "MascotData.hpp"
#include "AssetLoader.hpp"
#include "qdiriterator.h"
#include <QPainter>
#include <QDir>
#include "DefaultMascot.hpp"
#include <stdexcept>
#include <shijima/parser.hpp>

static QString readFile(QString const& file) {
    QFile f { file };
    if (!f.open(QFile::ReadOnly | QFile::Text))
        throw std::runtime_error("failed to open file: " + file.toStdString());
    QTextStream in(&f);
    return in.readAll(); 
}

MascotData::MascotData(): m_valid(false) {}

MascotData::MascotData(QString const& path, int id): m_path(path),
    m_valid(true), m_id(id) 
{
    if (path == "@") {
        m_name = "Default Mascot";
        m_behaviorsXML = QString { defaultMascot.at("behaviors.xml").first };
        m_actionsXML = QString { defaultMascot.at("actions.xml").first };
        m_path = "@";
        m_imgRoot = "@/img";
        m_valid = true;
        m_deletable = false;
        QImage frame;
        frame.loadFromData((const uchar *)defaultMascot.at("shime1.png").first,
            (int)defaultMascot.at("shime1.png").second);
        QImage preview = renderPreview(frame);
        m_preview = QPixmap::fromImage(preview);
        return;
    }
    m_deletable = true;
    QDir dir { path };
    auto dirname = dir.dirName();
    if (!dirname.endsWith(".mascot")) {
        throw std::invalid_argument("Mascot folder name must end with .mascot");
    }
    m_name = dirname.sliced(0, dirname.length() - 7);
    m_behaviorsXML = readFile(dir.filePath("behaviors.xml"));
    m_actionsXML = readFile(dir.filePath("actions.xml"));
    {
        // this throws if the XMLs are invalid
        shijima::parser parser;
        parser.parse(m_actionsXML.toStdString(), m_behaviorsXML.toStdString());
    }
    dir.cd("img");
    m_imgRoot = QDir::cleanPath(path + QDir::separator() + "img");
    QDirIterator iter { dir.absolutePath(), QDir::Files,
        QDirIterator::NoIteratorFlags };
    QList<QString> images;
    while (iter.hasNext()) {
        auto entry = iter.nextFileInfo();
        auto basename = entry.fileName();
        if (basename.endsWith(".png")) {
            images.append(basename);
        }
    }
    images.sort(Qt::CaseInsensitive);
    QImage frame;
    frame.load(dir.absoluteFilePath(images[0]));
    QImage preview = renderPreview(frame);
    m_preview = QPixmap::fromImage(preview);
}

QImage MascotData::renderPreview(QImage frame) {
    frame = frame.scaled({ 128, 128 }, Qt::KeepAspectRatio);
    QImage preview { 128, 128, QImage::Format_ARGB32_Premultiplied };
    preview.fill(Qt::transparent);
    QPainter painter { &preview };
    painter.setBackgroundMode(Qt::BGMode::TransparentMode);
    painter.drawImage(QPoint{ (128 - frame.width()) / 2, 0 }, frame);
    return preview;
}

QString const &MascotData::imgRoot() const {
    return m_imgRoot;
}

void MascotData::unloadCache() const {
    AssetLoader::defaultLoader()->unloadAssets(m_path);
}

bool MascotData::deletable() const {
    return m_deletable;
}

bool MascotData::valid() const {
    return m_valid;
}

QString const &MascotData::behaviorsXML() const {
    return m_behaviorsXML;
}

QString const &MascotData::actionsXML() const {
    return m_actionsXML;
}

QString const &MascotData::path() const {
    return m_path;
}

QString const &MascotData::name() const {
    return m_name;
}

QIcon const &MascotData::preview() const {
    return m_preview;
}

int MascotData::id() const {
    return m_id;
}
