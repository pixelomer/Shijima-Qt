#pragma once
#include <QString>
#include <QList>
#include <QVariant>

namespace Platform {
namespace GNOME {

QMap<QString, QVariant> getExtensionInfo(QString const& uuid);
bool isExtensionInstalled(QString const& uuid);
bool isExtensionEnabled(QString const& uuid);
void installExtension(QString const& path);
void enableExtension(QString const& uuid);
void disableExtension(QString const& uuid);
bool userExtensionsEnabled();
void setUserExtensionsEnabled(bool enabled);
bool running();

}
}