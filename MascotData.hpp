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

#include <QIcon>
#include <QImage>
#include <QString>

class MascotData {
private:
    QString m_behaviorsXML;
    QString m_actionsXML;
    QString m_path;
    QString m_name;
    QString m_imgRoot;
    QIcon m_preview;
    bool m_valid;
    bool m_deletable;
    int m_id;
    QImage renderPreview(QImage frame);
public:
    MascotData();
    MascotData(QString const& path, int id);
    void unloadCache() const;
    bool valid() const;
    bool deletable() const;
    QString const &behaviorsXML() const;
    QString const &actionsXML() const;
    QString const &path() const;
    QString const &name() const;
    QString const &imgRoot() const;
    QIcon const &preview() const;
    int id() const;
};
