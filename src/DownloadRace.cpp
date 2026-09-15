#include "DownloadRace.h"

#include "AppInfo.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QList>

namespace {
constexpr int kProbeTimeoutMs = 2500;
}

QString forgeProbeUrl(const QString& page, const QString& asset)
{
    if (!asset.trimmed().isEmpty())
        return asset.trimmed();
    return page.trimmed();
}

bool probeHttpStatusOk(int status)
{
    return status >= 200 && status < 400;
}

QStringList updateOpenUrlChain(const LinuxRelease& rel, const QString& winner)
{
    QStringList out;
    const QString gitee = forgeProbeUrl(rel.downloadGitee, rel.giteeAsset);
    const QString github = forgeProbeUrl(rel.downloadGithub, rel.githubAsset);
    const auto add = [&out](const QString& url) {
        if (url.isEmpty() || out.contains(url))
            return;
        out << url;
    };
    add(winner.trimmed());
    add(gitee);
    add(github);
    add(rel.downloadSite.trimmed());
    return out;
}

int downloadProbeTimeoutMs()
{
    return kProbeTimeoutMs;
}

DownloadRacer::DownloadRacer(QObject* parent)
    : QObject(parent)
{
}

DownloadRacer::~DownloadRacer()
{
    m_settled = true;
    abortAll();
}

void DownloadRacer::start(const LinuxRelease& rel)
{
    m_gitee = forgeProbeUrl(rel.downloadGitee, rel.giteeAsset);
    m_github = forgeProbeUrl(rel.downloadGithub, rel.githubAsset);

    QStringList race;
    if (!m_gitee.isEmpty())
        race << m_gitee;
    if (!m_github.isEmpty() && m_github != m_gitee)
        race << m_github;

    if (race.isEmpty()) {
        settle(QString());
        return;
    }
    if (race.size() == 1) {
        settle(race.first());
        return;
    }

    m_nam = new QNetworkAccessManager(this);
    for (const QString& url : race) {
        QNetworkRequest req{QUrl(url)};
        req.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Flip/%1").arg(QStringLiteral(FLIP_VERSION)));
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        req.setTransferTimeout(kProbeTimeoutMs);
#endif
        // GET + abort after headers: measures TTFB. HEAD is often blocked on CDNs.
        QNetworkReply* reply = m_nam->get(req);
        m_probes.push_back({url, reply});
        connect(reply, &QNetworkReply::metaDataChanged, this, &DownloadRacer::onMetaDataChanged);
        connect(reply, &QNetworkReply::finished, this, &DownloadRacer::onReplyFinished);
    }
    armWatchdog();
}

void DownloadRacer::armWatchdog()
{
    auto* watchdog = new QTimer(this);
    watchdog->setSingleShot(true);
    connect(watchdog, &QTimer::timeout, this, [this] {
        if (m_settled)
            return;
        abortAll();
        settle(QString());
    });
    watchdog->start(kProbeTimeoutMs);
}

void DownloadRacer::onMetaDataChanged()
{
    if (m_settled)
        return;
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (!probeHttpStatusOk(status))
        return;

    QString url;
    for (const Probe& p : m_probes) {
        if (p.reply == reply) {
            url = p.url;
            break;
        }
    }
    if (url.isEmpty())
        url = reply->url().toString();
    settle(url);
}

void DownloadRacer::onReplyFinished()
{
    if (m_settled)
        return;
    ++m_finishedCount;
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() == QNetworkReply::NoError && probeHttpStatusOk(status)) {
            QString url;
            for (const Probe& p : m_probes) {
                if (p.reply == reply) {
                    url = p.url;
                    break;
                }
            }
            settle(url.isEmpty() ? reply->url().toString() : url);
            return;
        }
    }
    if (m_finishedCount >= m_probes.size())
        settle(QString());
}

void DownloadRacer::settle(const QString& url)
{
    if (m_settled)
        return;
    m_settled = true;
    abortAll();
    emit finished(url);
    deleteLater();
}

void DownloadRacer::abortAll()
{
    for (Probe& p : m_probes) {
        if (!p.reply)
            continue;
        p.reply->disconnect(this);
        if (p.reply->isRunning())
            p.reply->abort();
        p.reply->deleteLater();
        p.reply = nullptr;
    }
}
