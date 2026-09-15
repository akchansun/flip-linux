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
    QString giteeAsset;
    QString githubAsset;
};

bool parseLinuxRelease(const QByteArray& json, LinuxRelease* out);
QString releaseNotes(const LinuxRelease& rel);
QUrl updateFeedUrl();
int updateFeedTimeoutMs();
