#include "MascotFinder.hpp"
#include <QDirIterator>
#include <iostream>
#include <QTextStream>
#include <optional>

using namespace shijima;

static std::optional<QString> readFile(QString const& file) {
    QFile f { file };
    if (!f.open(QFile::ReadOnly | QFile::Text)) return {};
    QTextStream in(&f);
    return in.readAll(); 
}

int MascotFinder::findAll(mascot::factory &factory) {
    int found = 0;
    QDir root { "." };
    QDir conf = root, img = root;
    if (!img.cd("img")) {
        std::cerr << "Missing img" << std::endl;
        return 0;
    }
    std::optional<QString> defaultBehaviors, defaultActions;
    if (conf.cd("conf")) {
        defaultBehaviors = readFile(conf.absoluteFilePath("behaviors.xml"));
        defaultActions = readFile(conf.absoluteFilePath("actions.xml"));
    }
    QDirIterator iter { img.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot,
        QDirIterator::NoIteratorFlags };
    while (iter.hasNext()) {
        QDir dir { iter.next() };
        if (!dir.exists()) {
            continue;
        }
        if (dir.dirName() == "unused") {
            continue;
        }
        conf = dir;
        std::optional<QString> behaviors, actions;
        if (conf.cd("conf")) {
            behaviors = readFile(conf.absoluteFilePath("behaviors.xml"));
            actions = readFile(conf.absoluteFilePath("actions.xml"));
        }
        if (!behaviors) {
            behaviors = defaultBehaviors;
        }
        if (!actions) {
            actions = defaultActions;
        }
        if (!behaviors || !actions) {
            std::cerr << "Missing configuration for: " <<
                dir.dirName().toStdString() << std::endl;
            continue;
        }
        mascot::factory::tmpl tmpl;
        tmpl.actions_xml = actions->toStdString();
        tmpl.behaviors_xml = behaviors->toStdString();
        tmpl.name = dir.dirName().toStdString();
        tmpl.path = dir.absolutePath().toStdString();
        factory.register_template(tmpl);
        ++found;
    }
    return found;
}