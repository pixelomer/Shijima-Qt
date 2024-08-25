#include "SoundEffectManager.hpp"
#include <QFile>
#include <QDir>
#include <iostream>

void SoundEffectManager::play(QString const& name) {
    if (!m_loadedEffects.contains(name)) {
        QUrl url;
        for (QString &searchPath : searchPaths) {
            QString file = QDir::cleanPath(searchPath + QDir::separator() + name);
            if (QFile::exists(file)) {
                url = QUrl::fromLocalFile(file);
                break;
            }
        }
        if (url.isEmpty()) {
            std::cerr << "Could not load effect: " << name.toStdString() << std::endl;
            return;
        }
        QSoundEffect *effect = m_loadedEffects[name] = new QSoundEffect;
        effect->setSource(url);
        effect->setLoopCount(1);
        effect->setVolume(1.f);
    }
    stop();
    m_loadedEffects[name]->play();
    m_activeName = name;
}

void SoundEffectManager::stop() {
    if (!m_activeName.isEmpty()) {
        m_loadedEffects[m_activeName]->stop();
        m_activeName.clear();
    }
}

SoundEffectManager::~SoundEffectManager() {
    for (QSoundEffect *effect : m_loadedEffects) {
        delete effect;
    }
}