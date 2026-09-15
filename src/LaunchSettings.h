#pragma once

#include <QSettings>
#include <QString>

// QSettings keys under org XixiangfengTech / Flip (see AppInfo.h).
namespace LaunchSettings {

inline QString tipsDontShowKey()
{
    return QStringLiteral("tips/dontShowAgain");
}

inline QString updateDontAskKey()
{
    return QStringLiteral("update/dontAsk");
}

inline bool shouldShowStartupTips(const QSettings& settings)
{
    return !settings.value(tipsDontShowKey(), false).toBool();
}

inline void setStartupTipsDontShow(QSettings& settings, bool dontShow = true)
{
    settings.setValue(tipsDontShowKey(), dontShow);
}

inline bool shouldAskForUpdates(const QSettings& settings)
{
    return !settings.value(updateDontAskKey(), false).toBool();
}

inline void setUpdateDontAsk(QSettings& settings, bool dontAsk = true)
{
    settings.setValue(updateDontAskKey(), dontAsk);
}

} // namespace LaunchSettings
