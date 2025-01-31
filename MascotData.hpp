#pragma once
#include <QImage>
#include <QString>

class MascotData {
private:
    QString m_behaviorsXML;
    QString m_actionsXML;
    QString m_path;
    QString m_name;
    QImage m_preview;
    bool m_valid;
public:
    MascotData();
    MascotData(QString const& path);
    void unloadCache() const;
    bool valid() const;
    QString const &behaviorsXML() const;
    QString const &actionsXML() const;
    QString const &path() const;
    QString const &name() const;
    QImage const &preview() const;
};