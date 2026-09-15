#pragma once

#include "FolderPager.h"

#include <QMainWindow>

class ImageView;
class QAction;
class QLabel;
class QMenu;
class QDragEnterEvent;
class QDropEvent;
class QShowEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool openPath(const QString& path);

protected:
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void retranslateUi();
    void applyLanguagePreference();
    void openImageDialog();
    void openFolderDialog();
    void goPrevious();
    void goNext();
    void goFirst();
    void goLast();
    void showAbout();
    void loadCurrent();
    void updateChrome();
    void handleDropped(const QStringList& paths);
    void restoreGeometryFromSettings();

    ImageView* m_view = nullptr;
    QLabel* m_statusLabel = nullptr;
    FolderPager m_pager;
    QString m_error;

    QAction* m_openImageAction = nullptr;
    QAction* m_openFolderAction = nullptr;
    QAction* m_quitAction = nullptr;
    QAction* m_prevAction = nullptr;
    QAction* m_nextAction = nullptr;
    QAction* m_zoomInAction = nullptr;
    QAction* m_zoomOutAction = nullptr;
    QAction* m_actualAction = nullptr;
    QAction* m_fitAction = nullptr;
    QAction* m_fullScreenAction = nullptr;
    QAction* m_langAutoAction = nullptr;
    QAction* m_langZhAction = nullptr;
    QAction* m_langEnAction = nullptr;
    QAction* m_aboutAction = nullptr;

    QMenu* m_fileMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_langMenu = nullptr;
    QMenu* m_helpMenu = nullptr;

    bool m_didLaunchPrompts = false;
};
