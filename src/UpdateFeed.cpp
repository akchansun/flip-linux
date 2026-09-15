#include "UpdateFeed.h"

#include "AppInfo.h"
#include "I18n.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>

namespace {
constexpr int kTimeoutMs = 6000;
} // namespace

bool parseLinuxRelease(const QByteArray& json, LinuxRelease* out)
{
    if (!out)
        return false;

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    const QJsonObject linuxObj = doc.object().value(QStringLiteral("linux")).toObject();
    if (linuxObj.isEmpty())
        return false;

    LinuxRelease rel;
    rel.version = linuxObj.value(QStringLiteral("version")).toString().trimmed();
    if (rel.version.isEmpty())
        return false;

    const QJsonObject notes = linuxObj.value(QStringLiteral("notes")).toObject();
    rel.notesZh = notes.value(QStringLiteral("zh")).toString();
    rel.notesEn = notes.value(QStringLiteral("en")).toString();

    const QJsonObject download = linuxObj.value(QStringLiteral("download")).toObject();
    rel.downloadSite = download.value(QStringLiteral("site")).toString().trimmed();
    rel.downloadGitee = download.value(QStringLiteral("gitee")).toString().trimmed();
    rel.downloadGithub = download.value(QStringLiteral("github")).toString().trimmed();

    *out = rel;
    return true;
}

QString releaseNotes(const LinuxRelease& rel)
{
    if (I18n::resolved() == I18n::Lang::ZhCN) {
        if (!rel.notesZh.isEmpty())
            return rel.notesZh;
        return rel.notesEn;
    }
    if (!rel.notesEn.isEmpty())
        return rel.notesEn;
    return rel.notesZh;
}

QString preferredDownloadUrl(const LinuxRelease& rel, bool preferChina)
{
    const QStringList chinaOrder{rel.downloadGitee, rel.downloadSite, rel.downloadGithub};
    const QStringList otherOrder{rel.downloadSite, rel.downloadGithub, rel.downloadGitee};
    for (const QString& url : (preferChina ? chinaOrder : otherOrder)) {
        if (!url.isEmpty())
            return url;
    }
    return {};
}

bool preferChinaDownload()
{
    if (I18n::resolved() == I18n::Lang::ZhCN)
        return true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    return QLocale::system().territory() == QLocale::China;
#else
    return QLocale::system().country() == QLocale::China;
#endif
}

QUrl updateFeedUrl()
{
    const QByteArray env = qgetenv("FLIP_UPDATE_FEED");
    if (!env.isEmpty())
        return QUrl::fromUserInput(QString::fromUtf8(env));
    return QUrl(QStringLiteral(FLIP_UPDATE_FEED_URL));
}

int updateFeedTimeoutMs()
{
    return kTimeoutMs;
}
