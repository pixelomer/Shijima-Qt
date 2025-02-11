#pragma once
#include <QIcon>
#include <QImage>
#include <QString>

class MascotData {
private:
    QString m_behaviorsXML;
    QString m_actionsXML;
    QString m_path;
    QString m_name;
    QIcon m_preview;
    bool m_valid;
    bool m_deletable;
    QImage renderPreview(QImage frame);
public:
    MascotData();
    MascotData(QString const& path);
    void unloadCache() const;
    bool valid() const;
    bool deletable() const;
    QString const &behaviorsXML() const;
    QString const &actionsXML() const;
    QString const &path() const;
    QString const &name() const;
    QIcon const &preview() const;
};