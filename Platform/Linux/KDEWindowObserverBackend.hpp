#include "WindowObserverBackend.hpp"
#include <QString>
#include <QTemporaryDir>
#include "ExtensionFile.hpp"

namespace Platform {

class KDEWindowObserverBackend : public WindowObserverBackend {
private:
    static const QString m_kwinScriptName;
    ExtensionFile m_extensionFile;
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