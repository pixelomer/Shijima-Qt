#include <QSoundEffect>
#include <QMap>
#include <QString>

class SoundEffectManager {
public:
    SoundEffectManager() {}
    QList<QString> searchPaths;
    void play(QString const& name);
    void stop();
    ~SoundEffectManager();
private:
    QMap<QString, QSoundEffect *> m_loadedEffects;
    QString m_activeName;
};