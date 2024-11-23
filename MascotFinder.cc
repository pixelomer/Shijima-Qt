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

void MascotFinder::findXMLs(QDir conf, std::optional<QString> &behaviors,
    std::optional<QString> &actions, bool &isJapanese)
{
    if (conf.cd("conf")) {
        auto files = conf.entryList(QDir::Filter::Files);
        for (auto &name : files) {
            auto lowercase = name.toLower();
            if (lowercase == "behaviors.xml") {
                behaviors = readFile(conf.absoluteFilePath(name));
            }
            else if (lowercase == "behavior.xml") {
                isJapanese = true;
                behaviors = readFile(conf.absoluteFilePath(name));
            }
            else if (lowercase == "行動.xml") {
                isJapanese = true;
                behaviors = readFile(conf.absoluteFilePath(name));
            }
            if (lowercase == "actions.xml") {
                actions = readFile(conf.absoluteFilePath(name));
            }
            else if (lowercase == "action.xml") {
                isJapanese = true;
                actions = readFile(conf.absoluteFilePath(name));
            }
            else if (lowercase == "動作.xml") {
                isJapanese = true;
                actions = readFile(conf.absoluteFilePath(name));
            }
        }
    }
}

void MascotFinder::findXMLs(QDir conf, std::optional<QString> &behaviors,
    std::optional<QString> &actions)
{
    bool isJapanese;
    findXMLs(conf, behaviors, actions, isJapanese);
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
    bool isJapanese = false;
    findXMLs(conf, defaultBehaviors, defaultActions, isJapanese);
    QDirIterator iter { img.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot,
        QDirIterator::NoIteratorFlags };
    QDir dir = img;
    do {
        if (dir == img && !isJapanese) {
            continue;
        }
        if (!dir.exists()) {
            continue;
        }
        if (dir.dirName() == "unused") {
            continue;
        }
        conf = dir;
        std::optional<QString> behaviors, actions;
        findXMLs(conf, behaviors, actions);
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
    while (iter.hasNext() && (dir.setPath(iter.next()), dir).exists());
    return found;
}