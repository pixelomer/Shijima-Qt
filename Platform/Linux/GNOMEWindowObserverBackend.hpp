#include "WindowObserverBackend.hpp"
#include <QString>

namespace Platform {

class GNOMEWindowObserverBackend : public WindowObserverBackend {
private:
    static const QString m_gnomeScriptPath;
    static const QString m_gnomeScriptUUID;
    void writeExtensionToDisk();
public:
    explicit GNOMEWindowObserverBackend();
    ~GNOMEWindowObserverBackend();
    bool alive() override;
};

}