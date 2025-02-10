#include "ExtensionFile.hpp"

namespace Platform {

ExtensionFile::ExtensionFile(): m_valid(false) {}

ExtensionFile::ExtensionFile(QString const& name, const char *data,
    size_t len): m_valid(true)
{
    if (!m_dir.isValid()) {
        throw std::runtime_error("failed to create temporary directory for GNOME extension");
    }
    m_path = m_dir.filePath(name);
    QFile file { m_path };
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error("failed to create temporary file for KDE extension");
    }
    QDataStream stream { &file };
    stream << QByteArray(data, len);
    file.flush();
    file.close();
}

QString const& ExtensionFile::path() {
    return m_path;
}

}