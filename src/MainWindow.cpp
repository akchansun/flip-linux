#include "MainWindow.h"

#include "AppInfo.h"
#include "I18n.h"
#include "ImageView.h"
#include "LaunchPrompts.h"

#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

namespace {
constexpr int kDefaultWidth = 960;
constexpr int kDefaultHeight = 640;
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setAcceptDrops(true);
    setWindowIcon(QIcon(QStringLiteral(":/icons/flip.svg")));
    resize(kDefaultWidth, kDefaultHeight);
    setupUi();
    applyLanguagePreference();
    retranslateUi();
    restoreGeometryFromSettings();
}

void MainWindow::setupUi()
{
    m_view = new ImageView(this);
    setCentralWidget(m_view);

    m_openImageAction = new QAction(this);
    m_openImageAction->setShortcut(QKeySequence::Open);
    m_openImageAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    connect(m_openImageAction, &QAction::triggered, this, &MainWindow::openImageDialog);

    m_openFolderAction = new QAction(this);
    m_openFolderAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    m_openFolderAction->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(m_openFolderAction, &QAction::triggered, this, &MainWindow::openFolderDialog);

    m_quitAction = new QAction(this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    connect(m_quitAction, &QAction::triggered, this, &QWidget::close);

    m_prevAction = new QAction(this);
    m_prevAction->setShortcut(QKeySequence(Qt::Key_Left));
    m_prevAction->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    connect(m_prevAction, &QAction::triggered, this, &MainWindow::goPrevious);

    m_nextAction = new QAction(this);
    m_nextAction->setShortcut(QKeySequence(Qt::Key_Right));
    m_nextAction->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    connect(m_nextAction, &QAction::triggered, this, &MainWindow::goNext);

    m_zoomInAction = new QAction(this);
    m_zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(Qt::Key_Plus), QKeySequence(Qt::Key_Equal)});
    connect(m_zoomInAction, &QAction::triggered, m_view, &ImageView::zoomIn);

    m_zoomOutAction = new QAction(this);
    m_zoomOutAction->setShortcuts({QKeySequence::ZoomOut, QKeySequence(Qt::Key_Minus)});
    connect(m_zoomOutAction, &QAction::triggered, m_view, &ImageView::zoomOut);

    m_actualAction = new QAction(this);
    m_actualAction->setShortcut(QKeySequence(Qt::Key_1));
    connect(m_actualAction, &QAction::triggered, m_view, &ImageView::zoomActual);

    m_fitAction = new QAction(this);
    m_fitAction->setShortcuts({QKeySequence(Qt::Key_0), QKeySequence(Qt::Key_F)});
    connect(m_fitAction, &QAction::triggered, m_view, &ImageView::zoomFitDefault);

    m_fullScreenAction = new QAction(this);
    m_fullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_fullScreenAction->setCheckable(true);
    connect(m_fullScreenAction, &QAction::triggered, this, [this](bool on) {
        if (on)
            showFullScreen();
        else
            showNormal();
    });

    m_langAutoAction = new QAction(this);
    m_langAutoAction->setCheckable(true);
    m_langZhAction = new QAction(this);
    m_langZhAction->setCheckable(true);
    m_langEnAction = new QAction(this);
    m_langEnAction->setCheckable(true);
    auto* langGroup = new QActionGroup(this);
    langGroup->setExclusive(true);
    langGroup->addAction(m_langAutoAction);
    langGroup->addAction(m_langZhAction);
    langGroup->addAction(m_langEnAction);
    connect(m_langAutoAction, &QAction::triggered, this, [this] {
        QSettings().setValue(QStringLiteral("language"), QStringLiteral("auto"));
        applyLanguagePreference();
        retranslateUi();
    });
    connect(m_langZhAction, &QAction::triggered, this, [this] {
        QSettings().setValue(QStringLiteral("language"), QStringLiteral("zh_CN"));
        applyLanguagePreference();
        retranslateUi();
    });
    connect(m_langEnAction, &QAction::triggered, this, [this] {
        QSettings().setValue(QStringLiteral("language"), QStringLiteral("en"));
        applyLanguagePreference();
        retranslateUi();
    });

    m_tipsAction = new QAction(this);
    m_tipsAction->setShortcut(QKeySequence::HelpContents);
    connect(m_tipsAction, &QAction::triggered, this, &MainWindow::showTips);

    m_visitWebsiteAction = new QAction(this);
    connect(m_visitWebsiteAction, &QAction::triggered, this, &MainWindow::visitWebsite);

    m_aboutAction = new QAction(this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    m_fileMenu = menuBar()->addMenu(QString());
    m_fileMenu->addAction(m_openImageAction);
    m_fileMenu->addAction(m_openFolderAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_viewMenu = menuBar()->addMenu(QString());
    m_viewMenu->addAction(m_prevAction);
    m_viewMenu->addAction(m_nextAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_zoomInAction);
    m_viewMenu->addAction(m_zoomOutAction);
    m_viewMenu->addAction(m_actualAction);
    m_viewMenu->addAction(m_fitAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_fullScreenAction);
    m_langMenu = m_viewMenu->addMenu(QString());
    m_langMenu->addAction(m_langAutoAction);
    m_langMenu->addAction(m_langZhAction);
    m_langMenu->addAction(m_langEnAction);

    m_helpMenu = menuBar()->addMenu(QString());
    m_helpMenu->addAction(m_tipsAction);
    m_helpMenu->addAction(m_visitWebsiteAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutAction);

    auto* toolbar = addToolBar(QStringLiteral("main"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->addAction(m_openImageAction);
    toolbar->addAction(m_openFolderAction);
    toolbar->addSeparator();
    toolbar->addAction(m_prevAction);
    toolbar->addAction(m_nextAction);
    toolbar->addSeparator();
    toolbar->addAction(m_fitAction);
    toolbar->addAction(m_actualAction);

    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);

    connect(m_view, &ImageView::previousRequested, this, &MainWindow::goPrevious);
    connect(m_view, &ImageView::nextRequested, this, &MainWindow::goNext);
    connect(m_view, &ImageView::filesDropped, this, &MainWindow::handleDropped);
    connect(m_view, &ImageView::scaleChanged, this, [this](double) { updateChrome(); });
}

void MainWindow::applyLanguagePreference()
{
    const QString pref = QSettings().value(QStringLiteral("language"), QStringLiteral("auto")).toString();
    if (pref == QLatin1String("zh_CN"))
        I18n::setLang(I18n::Lang::ZhCN);
    else if (pref == QLatin1String("en"))
        I18n::setLang(I18n::Lang::En);
    else
        I18n::setLang(I18n::Lang::Auto);

    m_langAutoAction->setChecked(pref == QLatin1String("auto") || pref.isEmpty());
    m_langZhAction->setChecked(pref == QLatin1String("zh_CN"));
    m_langEnAction->setChecked(pref == QLatin1String("en"));
}

void MainWindow::retranslateUi()
{
    m_fileMenu->setTitle(I18n::t("menu.file"));
    m_viewMenu->setTitle(I18n::t("menu.view"));
    m_helpMenu->setTitle(I18n::t("menu.help"));
    m_langMenu->setTitle(I18n::t("menu.language"));

    m_openImageAction->setText(I18n::t("file.openImage"));
    m_openFolderAction->setText(I18n::t("file.openFolder"));
    m_quitAction->setText(I18n::t("file.quit"));
    m_prevAction->setText(I18n::t("view.previous"));
    m_nextAction->setText(I18n::t("view.next"));
    m_zoomInAction->setText(I18n::t("view.zoomIn"));
    m_zoomOutAction->setText(I18n::t("view.zoomOut"));
    m_actualAction->setText(I18n::t("view.actualSize"));
    m_fitAction->setText(I18n::t("view.fitDefault"));
    m_fullScreenAction->setText(I18n::t("view.fullscreen"));
    m_langAutoAction->setText(I18n::t("view.langAuto"));
    m_langZhAction->setText(I18n::t("view.langZh"));
    m_langEnAction->setText(I18n::t("view.langEn"));
    m_tipsAction->setText(I18n::t("help.tips"));
    m_visitWebsiteAction->setText(I18n::t("help.visitWebsite"));
    m_aboutAction->setText(I18n::t("help.about"));
    QApplication::setApplicationDisplayName(I18n::t("app.name"));
    m_view->update();
    updateChrome();
}

bool MainWindow::openPath(const QString& path)
{
    m_error.clear();
    if (!m_pager.open(path)) {
        m_error = I18n::t("error.cannotOpen");
        m_view->clearImage();
        updateChrome();
        return false;
    }
    loadCurrent();
    return m_view->hasImage();
}

void MainWindow::openImageDialog()
{
    const QString start = m_pager.currentPath().isEmpty() ? QDir::homePath()
                                                          : QFileInfo(m_pager.currentPath()).absolutePath();
    const QString path = QFileDialog::getOpenFileName(this, I18n::t("dialog.openImage"), start,
                                                      FolderPager::fileDialogFilter());
    if (!path.isEmpty())
        openPath(path);
}

void MainWindow::openFolderDialog()
{
    const QString start = m_pager.directory().isEmpty() ? QDir::homePath() : m_pager.directory();
    const QString path = QFileDialog::getExistingDirectory(this, I18n::t("dialog.openFolder"), start);
    if (!path.isEmpty())
        openPath(path);
}

void MainWindow::goPrevious()
{
    if (m_pager.previous())
        loadCurrent();
}

void MainWindow::goNext()
{
    if (m_pager.next())
        loadCurrent();
}

void MainWindow::goFirst()
{
    if (m_pager.first())
        loadCurrent();
}

void MainWindow::goLast()
{
    if (m_pager.last())
        loadCurrent();
}

void MainWindow::loadCurrent()
{
    m_error.clear();
    const QString path = m_pager.currentPath();
    if (path.isEmpty()) {
        m_view->clearImage();
        updateChrome();
        return;
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        m_error = I18n::t("error.cannotOpen");
        m_view->clearImage();
    } else {
        m_view->setImage(image);
    }
    updateChrome();
}

void MainWindow::updateChrome()
{
    if (m_pager.isEmpty()) {
        setWindowTitle(I18n::t("app.name"));
        m_statusLabel->setText(m_error.isEmpty() ? I18n::t("status.ready") : m_error);
        return;
    }

    const QString name = m_pager.currentFileName();
    setWindowTitle(name + QStringLiteral(" — ") + I18n::t("app.name"));

    QStringList parts;
    parts << I18n::t("status.position").arg(m_pager.index() + 1).arg(m_pager.count());
    if (!m_error.isEmpty()) {
        parts << m_error;
    } else if (m_view->hasImage()) {
        const QSize sz = m_view->imagePixelSize();
        parts << QStringLiteral("%1 × %2").arg(sz.width()).arg(sz.height());
        parts << QStringLiteral("%1%").arg(qRound(m_view->scale() * 100.0));
    }
    m_statusLabel->setText(parts.join(QStringLiteral("  ·  ")));
}

void MainWindow::handleDropped(const QStringList& paths)
{
    if (paths.isEmpty())
        return;
    openPath(paths.first());
}

void MainWindow::showTips()
{
    QDialog dialog(this);
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
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    if (QPushButton* ok = buttons->button(QDialogButtonBox::Ok)) {
        ok->setText(I18n::t("tips.ok"));
        ok->setDefault(true);
    }
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);
    dialog.adjustSize();
    dialog.exec();
}

void MainWindow::visitWebsite()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral(FLIP_WEBSITE_URL)));
}

void MainWindow::showAbout()
{
    QDialog dialog(this);
    dialog.setWindowTitle(I18n::t("about.title"));
    dialog.setModal(true);
    auto* layout = new QVBoxLayout(&dialog);
    auto* label = new QLabel(I18n::t("about.body").arg(QStringLiteral(FLIP_VERSION)), &dialog);
    label->setTextFormat(Qt::RichText);
    label->setOpenExternalLinks(true);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    layout->addWidget(label);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);
    dialog.resize(560, 320);
    dialog.exec();
}

void MainWindow::restoreGeometryFromSettings()
{
    QSettings settings;
    const QByteArray geo = settings.value(QStringLiteral("geometry")).toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (m_didLaunchPrompts)
        return;
    m_didLaunchPrompts = true;
    // After the window is up so a CLI image path can paint first; one combined tips/update dialog.
    QTimer::singleShot(0, this, [this] { runLaunchPrompts(this); });
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings().setValue(QStringLiteral("geometry"), saveGeometry());
    QMainWindow::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
    case Qt::Key_Backspace:
        goPrevious();
        return;
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_Space:
        goNext();
        return;
    case Qt::Key_Home:
        goFirst();
        return;
    case Qt::Key_End:
        goLast();
        return;
    case Qt::Key_Escape:
        if (isFullScreen()) {
            m_fullScreenAction->setChecked(false);
            showNormal();
            return;
        }
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    QStringList paths;
    for (const QUrl& url : event->mimeData()->urls()) {
        if (url.isLocalFile())
            paths << url.toLocalFile();
    }
    handleDropped(paths);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(event);
}
