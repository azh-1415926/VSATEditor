#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

struct latest_release_info
{
    QString url;
    QString version;
    QString info;

    latest_release_info(const QString &latest_url,
                        const QString &latest_version,
                        const QString &latest_info)
        : url(latest_url), version(latest_version), info(latest_info)
    {}

    latest_release_info(const latest_release_info &i)
        : url(i.info), version(i.version), info(i.info)
    {}
};

class auto_updater : public QObject
{
    Q_OBJECT

  public:
    explicit auto_updater(QObject *parent = nullptr);
    ~auto_updater();

  public slots:
    void request_latest_info();
    void handle_latest_info(QNetworkReply *reply);

  signals:
    void version_updated(const latest_release_info &info);

  private:
    QNetworkAccessManager *m_network_manager;

    void init();
};