#include "WindowObserverBackend.hpp"
#include <QString>
#include <QTemporaryDir>
#include "ExtensionFile.hpp"

namespace Platform {

class GNOMEWindowObserverBackend : public WindowObserverBackend {
private:
    static const QString m_gnomeScriptUUID;
    static const QString m_gnomeScriptVersion;
    ExtensionFile m_extensionFile;
public:
    explicit GNOMEWindowObserverBackend();
    ~GNOMEWindowObserverBackend();
    bool alive() override;
};

}