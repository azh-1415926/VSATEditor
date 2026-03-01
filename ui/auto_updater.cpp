#include "auto_updater.h"
#include "azh/utils/logger.hpp"

#include "utils.hpp"
#include <QJsonObject>
#include <QJsonParseError>

QJsonObject str2json(const QString &str)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError && !doc.isNull())
    {
        aDebug(AZH_ERROR) << "json parse error!";
        return QJsonObject();
    }

    return doc.object();
}

auto_updater::auto_updater(QObject *parent)
    : QObject(parent), m_network_manager(new QNetworkAccessManager)
{
    init();
}

auto_updater::~auto_updater() { delete m_network_manager; }

void auto_updater::requestLatestInfo()
{
    QNetworkRequest request;
    request.setUrl(QUrl(
        "https://api.github.com/repos/azh-1415926/VSATEditor/releases/latest"));
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWe");
    request.setRawHeader("Accept", "application/json");

    m_network_manager->get(request);
}

void auto_updater::handleLatestInfo(QNetworkReply *reply)
{
    disconnect(m_network_manager, &QNetworkAccessManager::finished, this,
               &auto_updater::handleLatestInfo);

    QString str = reply->readAll();
    QJsonObject json = str2json(str);
    /* html_url, latest tag url */
    QString html_url = json.value("html_url").toString();
    /* tag_name, latest tag */
    QString tag_name = json.value("tag_name").toString();
    /* body, latest info */
    QString body = json.value("body").toString();

    latest_release_info info(html_url, tag_name, body);

    if (!html_url.isEmpty() || !tag_name.isEmpty() || !body.isEmpty())
    {
        emit versionUpdated(info);
    }

    reply->deleteLater();
}

void auto_updater::init()
{
    connect(m_network_manager, &QNetworkAccessManager::finished, this,
            &auto_updater::handleLatestInfo);
}
