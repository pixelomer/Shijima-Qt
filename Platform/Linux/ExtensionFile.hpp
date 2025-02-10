#pragma once

#include <QTemporaryDir>
#include <QString>

namespace Platform {

class ExtensionFile {
private:
    QTemporaryDir m_dir;
    QString m_path;
    bool m_valid;
public:
    ExtensionFile();
    ExtensionFile(QString const& filename, const char *data, size_t len);
    QString const& path();
    bool valid();
};

}