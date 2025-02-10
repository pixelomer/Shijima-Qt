#include "WindowObserverBackend.hpp"
#include <QString>
#include <QTemporaryDir>

namespace Platform {

class KDEWindowObserverBackend : public WindowObserverBackend {
private:
    static const QString m_kwinScriptName;
    QTemporaryDir m_kwinScriptDir;
    QString m_kwinScriptPath;
    int m_kwinScriptID = -1;
    void loadKWinScript();
    void startKWinScript();
    bool isKWinScriptLoaded();
    void stopKWinScript();
public:
    explicit KDEWindowObserverBackend();
    ~KDEWindowObserverBackend();
    bool alive() override;
};

}