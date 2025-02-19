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
    QString m_imgRoot;
    QIcon m_preview;
    bool m_valid;
    bool m_deletable;
    int m_id;
    QImage renderPreview(QImage frame);
public:
    MascotData();
    MascotData(QString const& path, int id);
    void unloadCache() const;
    bool valid() const;
    bool deletable() const;
    QString const &behaviorsXML() const;
    QString const &actionsXML() const;
    QString const &path() const;
    QString const &name() const;
    QString const &imgRoot() const;
    QIcon const &preview() const;
    int id() const;
};
