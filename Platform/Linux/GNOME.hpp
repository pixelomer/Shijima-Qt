#pragma once
#include <QString>
#include <QList>
#include <QVariant>

namespace Platform {
namespace GNOME {

QMap<QString, QVariant> getExtensionInfo(QString uuid);
bool isExtensionInstalled(QString uuid);
bool isExtensionEnabled(QString uuid);
void installExtension(QString path);
void enableExtension(QString uuid);
void disableExtension(QString uuid);
bool userExtensionsEnabled();
void setUserExtensionsEnabled(bool enabled);
bool running();

}
}