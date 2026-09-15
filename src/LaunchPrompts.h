#pragma once

class QWidget;
struct LinuxRelease;

// Startup tips (modal), then an async online update check. Call after the main
// window is visible so a CLI image path can load first.
void runLaunchPrompts(QWidget* parent);

void showStartupTipsIfNeeded(QWidget* parent);
void startOnlineUpdateCheck(QWidget* parent);
void showUpdateAvailableDialog(QWidget* parent, const LinuxRelease& rel);
