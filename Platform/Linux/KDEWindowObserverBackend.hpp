#include "WindowObserverBackend.hpp"
#include <QString>

namespace Platform {

class KDEWindowObserverBackend : public WindowObserverBackend {
private:
    static const QString m_kwinScriptPath;
    static const QString m_kwinScriptName;
    int m_kwinScriptID = -1;
    void createKWinScript();
    void startKWinScript();
    bool isKWinScriptLoaded();
    void stopKWinScript();
public:
    explicit KDEWindowObserverBackend();
    ~KDEWindowObserverBackend();
    bool alive() override;
};

}