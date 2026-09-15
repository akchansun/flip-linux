#pragma once

#include <QString>
#include <QUrl>

struct LinuxRelease {
    QString version;
    QString notesZh;
    QString notesEn;
    QString downloadSite;
    QString downloadGitee;
    QString downloadGithub;
};

bool parseLinuxRelease(const QByteArray& json, LinuxRelease* out);
QString releaseNotes(const LinuxRelease& rel);
QString preferredDownloadUrl(const LinuxRelease& rel, bool preferChina);
bool preferChinaDownload();
QUrl updateFeedUrl();
int updateFeedTimeoutMs();
