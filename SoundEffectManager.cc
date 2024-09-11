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
    QSoundEffect *effect = m_loadedEffects[name];
    effect->play();
    m_activeEffect = effect;
}

bool SoundEffectManager::playing() const {
    if (m_activeEffect != nullptr) {
        return m_activeEffect->isPlaying();
    }
    return false;
}

void SoundEffectManager::stop() {
    if (m_activeEffect != nullptr) {
        m_activeEffect->stop();
        m_activeEffect = nullptr;
    }
}

SoundEffectManager::~SoundEffectManager() {
    for (QSoundEffect *effect : m_loadedEffects) {
        delete effect;
    }
}