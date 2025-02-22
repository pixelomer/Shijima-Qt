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

#include "SoundEffectManager.hpp"

#if SHIJIMA_USE_QTMULTIMEDIA

#include <QFile>
#include <QDir>
#include <iostream>
#include <QSoundEffect>

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

#else

void SoundEffectManager::play(QString const&) {}
bool SoundEffectManager::playing() const { return true; }
void SoundEffectManager::stop() {}
SoundEffectManager::~SoundEffectManager() {}

#endif