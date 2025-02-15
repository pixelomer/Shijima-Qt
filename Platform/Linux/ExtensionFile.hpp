#pragma once

#include <QTemporaryDir>
#include <QString>

namespace Platform {

class ExtensionFile {
private:
    QTemporaryDir m_dir;
    QString m_path;
    QString m_hostPath;
    bool m_valid;
public:
    static bool flatpak();
    ExtensionFile();
    ExtensionFile(QString const& filename, bool text, const char *data, size_t len);
    QString const& path();
    QString const& hostPath();
    bool valid();
};

}