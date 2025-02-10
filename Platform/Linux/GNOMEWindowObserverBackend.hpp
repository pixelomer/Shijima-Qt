#include "WindowObserverBackend.hpp"
#include <QString>
#include <QTemporaryDir>

namespace Platform {

class GNOMEWindowObserverBackend : public WindowObserverBackend {
private:
    static const QString m_gnomeScriptUUID;
    static const QString m_gnomeScriptVersion;
    QTemporaryDir m_gnomeScriptDir;
    QString m_gnomeScriptPath;
public:
    explicit GNOMEWindowObserverBackend();
    ~GNOMEWindowObserverBackend();
    bool alive() override;
};

}