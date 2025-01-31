#include "MascotData.hpp"
#include "AssetLoader.hpp"
#include "qdiriterator.h"
#include <QDir>
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

MascotData::MascotData(QString const& path): m_path(path), m_valid(true) {
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
    images.sort();
    m_preview.load(dir.absoluteFilePath(images[0]));
}

void MascotData::unloadCache() const {
    AssetLoader::defaultLoader()->unloadAssets(m_path);
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

QImage const &MascotData::preview() const {
    return m_preview;
}
