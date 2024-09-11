#include <QSoundEffect>
#include <QMap>
#include <QString>

class SoundEffectManager {
public:
    SoundEffectManager() {}
    QList<QString> searchPaths;
    void play(QString const& name);
    bool playing() const;
    void stop();
    ~SoundEffectManager();
private:
    QMap<QString, QSoundEffect *> m_loadedEffects;
    QSoundEffect *m_activeEffect = nullptr;
};