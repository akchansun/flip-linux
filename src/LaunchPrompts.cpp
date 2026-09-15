#include "LaunchPrompts.h"

#include "AppInfo.h"
#include "I18n.h"
#include "LaunchSettings.h"
#include "UpdateFeed.h"
#include "VersionCompare.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

class UpdateChecker final : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QWidget* parentWindow)
        : QObject(parentWindow)
        , m_window(parentWindow)
    {
    }

    void start()
    {
        const QUrl url = updateFeedUrl();
        if (!url.isValid())
            return;

        auto* nam = new QNetworkAccessManager(this);
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Flip/%1").arg(QStringLiteral(FLIP_VERSION)));
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        req.setTransferTimeout(updateFeedTimeoutMs());
#endif

        m_reply = nam->get(req);
        connect(m_reply, &QNetworkReply::finished, this, &UpdateChecker::onFinished);

        auto* watchdog = new QTimer(this);
        watchdog->setSingleShot(true);
        connect(watchdog, &QTimer::timeout, this, [this] {
            if (m_reply)
                m_reply->abort();
        });
        watchdog->start(updateFeedTimeoutMs());
    }

private slots:
    void onFinished()
    {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        if (!reply) {
            deleteLater();
            return;
        }

        const QNetworkReply::NetworkError error = reply->error();
        const QByteArray payload = (error == QNetworkReply::NoError) ? reply->readAll() : QByteArray();
        reply->deleteLater();

        if (error != QNetworkReply::NoError) {
            deleteLater();
            return;
        }

        LinuxRelease rel;
        if (!parseLinuxRelease(payload, &rel)) {
            deleteLater();
            return;
        }
        if (!isNewerVersion(rel.version, QStringLiteral(FLIP_VERSION))) {
            deleteLater();
            return;
        }

        QWidget* window = m_window;
        if (!window) {
            deleteLater();
            return;
        }
        QTimer::singleShot(0, window, [window, rel] { showUpdateAvailableDialog(window, rel); });
        deleteLater();
    }

private:
    QPointer<QWidget> m_window;
    QNetworkReply* m_reply = nullptr;
};

void showStartupTipsIfNeeded(QWidget* parent)
{
    QSettings settings;
    if (!LaunchSettings::shouldShowStartupTips(settings))
        return;

    QDialog dialog(parent);
    dialog.setWindowTitle(I18n::t("tips.title"));
    dialog.setModal(true);
    dialog.setMinimumWidth(500);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 16, 20, 12);
    layout->setSpacing(12);
    auto* label = new QLabel(I18n::t("tips.body"), &dialog);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(460);
    layout->addWidget(label);

    auto* buttons = new QDialogButtonBox(&dialog);
    QPushButton* dontBtn = buttons->addButton(I18n::t("tips.dontShow"), QDialogButtonBox::ResetRole);
    QPushButton* okBtn = buttons->addButton(I18n::t("tips.ok"), QDialogButtonBox::AcceptRole);
    okBtn->setDefault(true);
    okBtn->setFocus();
    layout->addWidget(buttons);

    QObject::connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(dontBtn, &QPushButton::clicked, &dialog, [&] {
        LaunchSettings::setStartupTipsDontShow(settings, true);
        settings.sync();
        dialog.accept();
    });

    dialog.adjustSize();
    dialog.exec();
}

void showUpdateAvailableDialog(QWidget* parent, const LinuxRelease& rel)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(I18n::t("update.title"));
    dialog.setModal(true);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 16, 20, 12);
    layout->setSpacing(12);
    const QString notes = releaseNotes(rel).toHtmlEscaped().replace(QLatin1Char('\n'),
                                                                   QStringLiteral("<br>"));
    auto* label = new QLabel(
        I18n::t("update.body").arg(QStringLiteral(FLIP_VERSION), rel.version, notes), &dialog);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(460);
    layout->addWidget(label);

    auto* buttons = new QDialogButtonBox(&dialog);
    QPushButton* dontBtn = buttons->addButton(I18n::t("update.dont"), QDialogButtonBox::ResetRole);
    QPushButton* laterBtn = buttons->addButton(I18n::t("update.later"), QDialogButtonBox::RejectRole);
    QPushButton* goBtn = buttons->addButton(I18n::t("update.go"), QDialogButtonBox::AcceptRole);
    goBtn->setDefault(true);
    goBtn->setFocus();
    layout->addWidget(buttons);

    QObject::connect(laterBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(goBtn, &QPushButton::clicked, &dialog, [&] {
        const QString url = preferredDownloadUrl(rel, preferChinaDownload());
        if (!url.isEmpty())
            QDesktopServices::openUrl(QUrl(url));
        dialog.accept();
    });
    QObject::connect(dontBtn, &QPushButton::clicked, &dialog, [&] {
        QSettings settings;
        LaunchSettings::setUpdateDontAsk(settings, true);
        settings.sync();
        dialog.reject();
    });

    dialog.setMinimumWidth(500);
    dialog.adjustSize();
    dialog.exec();
}

void startOnlineUpdateCheck(QWidget* parent)
{
    QSettings settings;
    if (!LaunchSettings::shouldAskForUpdates(settings))
        return;
    if (!parent)
        return;

    auto* checker = new UpdateChecker(parent);
    checker->start();
}

void runLaunchPrompts(QWidget* parent)
{
    showStartupTipsIfNeeded(parent);
    if (!parent)
        return;
    // After tips is fully dismissed so the check never nests inside that modal loop.
    QTimer::singleShot(0, parent, [parent] { startOnlineUpdateCheck(parent); });
}

#include "LaunchPrompts.moc"
