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

#include "ExtensionFile.hpp"
#include <QProcessEnvironment>
#include <unistd.h>

namespace Platform {

ExtensionFile::ExtensionFile(): m_valid(false) {}

ExtensionFile::ExtensionFile(QString const& name, bool text,
    const char *data, size_t len): m_valid(true)
{
    if (!m_dir.isValid()) {
        throw std::runtime_error("failed to create temporary directory for GNOME extension");
    }
    m_path = m_dir.filePath(name);
    QFile file { m_path };
    QFile::OpenMode mode = QFile::WriteOnly;
    if (text) {
        mode |= QFile::Text;
    }
    if (!file.open(mode)) {
        throw std::runtime_error("failed to create temporary file for KDE extension");
    }
    if (text) {
        QTextStream stream { &file };
        stream << QByteArray(data, len);
        stream.flush();
    }
    else {
        QDataStream stream { &file };
        stream << QByteArray(data, len);
    }
    file.flush();
    file.close();
    if (flatpak()) {
        int uid = getuid();
        QString hostPath = QDir::cleanPath("/run/user/" +
            QString::number(uid) + "/.flatpak/com.pixelomer.ShijimaQt/" + m_path);
        m_hostPath = hostPath;
    }
    else {
        m_hostPath = m_path;
    }
}

bool ExtensionFile::flatpak() {
    return QProcessEnvironment::systemEnvironment()
        .value("SHIJIMA_FLATPAK") == "1";
}

QString const& ExtensionFile::path() {
    return m_path;
}

QString const& ExtensionFile::hostPath() {
    return m_hostPath;
}

}