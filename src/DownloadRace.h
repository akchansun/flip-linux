#pragma once

#include "UpdateFeed.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkReply;

// HEAD/GET the asset URL when present (e.g. amd64 tarball), otherwise the release page.
QString forgeProbeUrl(const QString& page, const QString& asset);

// HTTP 2xx/3xx count as a live mirror. 4xx/5xx/0 do not win the race.
bool probeHttpStatusOk(int status);

// Unique order: race winner, then the other forge, then the product site.
QStringList updateOpenUrlChain(const LinuxRelease& rel, const QString& winner);

int downloadProbeTimeoutMs();

// Parallel TTFB race of Gitee vs GitHub. Emits the first successful probe URL
// (asset preferred). If both fail, emits an empty string; the caller opens
// Gitee, then GitHub, then the site.
class DownloadRacer final : public QObject
{
    Q_OBJECT

public:
    explicit DownloadRacer(QObject* parent = nullptr);
    ~DownloadRacer() override;
    void start(const LinuxRelease& rel);

signals:
    void finished(const QString& url);

private slots:
    void onMetaDataChanged();
    void onReplyFinished();

private:
    struct Probe {
        QString url;
        QNetworkReply* reply = nullptr;
    };

    void armWatchdog();
    void settle(const QString& url);
    void abortAll();

    QNetworkAccessManager* m_nam = nullptr;
    QList<Probe> m_probes;
    QString m_gitee;
    QString m_github;
    bool m_settled = false;
    int m_finishedCount = 0;
};
