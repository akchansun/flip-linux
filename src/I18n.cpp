#include "I18n.h"

#include <QHash>
#include <QLocale>
#include <QString>

namespace I18n {
namespace {

Lang g_pref = Lang::Auto;

QHash<QString, QString> zhTable()
{
    return {
        {"app.name", QStringLiteral("看图")},
        {"menu.file", QStringLiteral("文件(&F)")},
        {"menu.view", QStringLiteral("查看(&V)")},
        {"menu.help", QStringLiteral("帮助(&H)")},
        {"menu.language", QStringLiteral("语言(&L)")},
        {"file.openImage", QStringLiteral("打开图片(&O)…")},
        {"file.openFolder", QStringLiteral("打开文件夹(&D)…")},
        {"file.quit", QStringLiteral("退出(&Q)")},
        {"view.previous", QStringLiteral("上一张")},
        {"view.next", QStringLiteral("下一张")},
        {"view.first", QStringLiteral("第一张")},
        {"view.last", QStringLiteral("最后一张")},
        {"view.zoomIn", QStringLiteral("放大")},
        {"view.zoomOut", QStringLiteral("缩小")},
        {"view.actualSize", QStringLiteral("实际大小")},
        {"view.fitDefault", QStringLiteral("适应窗口")},
        {"view.fullscreen", QStringLiteral("全屏")},
        {"view.langAuto", QStringLiteral("跟随系统")},
        {"view.langZh", QStringLiteral("简体中文")},
        {"view.langEn", QStringLiteral("English")},
        {"help.about", QStringLiteral("关于(&A)")},
        {"empty.hint", QStringLiteral("打开图片，或把文件拖到这里")},
        {"empty.sub", QStringLiteral("方向键在同一文件夹里前后翻页")},
        {"error.cannotOpen", QStringLiteral("无法打开该图片")},
        {"dialog.openImage", QStringLiteral("打开图片")},
        {"dialog.openFolder", QStringLiteral("打开文件夹")},
        {"dialog.images", QStringLiteral("图片")},
        {"dialog.allFiles", QStringLiteral("所有文件")},
        {"about.title", QStringLiteral("关于 看图")},
        {"about.body",
         QStringLiteral(
             "<p><b>看图</b>（Flip）是面向办公场景的轻量看图软件：打开一张图片，即可用方向键在"
             "<b>同一文件夹</b>里前后翻页，类似经典的 Windows 照片查看器。</p>"
             "<p>默认缩放：小于窗口的图片按 100% 显示；大于窗口的图片缩小以适应窗口，居中、默认不放大。</p>"
             "<p>许可证：MIT<br>开发：喜相逢科技 / Xixiangfeng Tech<br>"
             "网站：<a href=\"https://www.ak129.cn/flip/\">https://www.ak129.cn/flip/</a></p>")},
        {"status.ready", QStringLiteral("打开一张图片开始浏览")},
        {"status.position", QStringLiteral("%1 / %2")},
        {"usage",
         QStringLiteral("用法: Flip [图片文件或文件夹]\n"
                        "      Flip --self-test\n"
                        "      Flip --help")},
    };
}

QHash<QString, QString> enTable()
{
    return {
        {"app.name", QStringLiteral("Flip")},
        {"menu.file", QStringLiteral("&File")},
        {"menu.view", QStringLiteral("&View")},
        {"menu.help", QStringLiteral("&Help")},
        {"menu.language", QStringLiteral("&Language")},
        {"file.openImage", QStringLiteral("&Open Image…")},
        {"file.openFolder", QStringLiteral("Open &Folder…")},
        {"file.quit", QStringLiteral("&Quit")},
        {"view.previous", QStringLiteral("Previous")},
        {"view.next", QStringLiteral("Next")},
        {"view.first", QStringLiteral("First")},
        {"view.last", QStringLiteral("Last")},
        {"view.zoomIn", QStringLiteral("Zoom In")},
        {"view.zoomOut", QStringLiteral("Zoom Out")},
        {"view.actualSize", QStringLiteral("Actual Size")},
        {"view.fitDefault", QStringLiteral("Fit Window")},
        {"view.fullscreen", QStringLiteral("Full Screen")},
        {"view.langAuto", QStringLiteral("System default")},
        {"view.langZh", QStringLiteral("简体中文")},
        {"view.langEn", QStringLiteral("English")},
        {"help.about", QStringLiteral("&About")},
        {"empty.hint", QStringLiteral("Open an image, or drop a file here")},
        {"empty.sub", QStringLiteral("Arrow keys page through the same folder")},
        {"error.cannotOpen", QStringLiteral("Could not open this image")},
        {"dialog.openImage", QStringLiteral("Open Image")},
        {"dialog.openFolder", QStringLiteral("Open Folder")},
        {"dialog.images", QStringLiteral("Images")},
        {"dialog.allFiles", QStringLiteral("All files")},
        {"about.title", QStringLiteral("About Flip")},
        {"about.body",
         QStringLiteral(
             "<p><b>Flip</b> (看图) is a lightweight office image viewer: open one picture, then "
             "page through the <b>same folder</b> with the arrow keys, like classic Windows Photo Viewer.</p>"
             "<p>Default zoom: images smaller than the window stay at 100%; larger images scale down to fit, "
             "centered, with no upscaling by default.</p>"
             "<p>License: MIT<br>Developer: 喜相逢科技 / Xixiangfeng Tech<br>"
             "Website: <a href=\"https://www.ak129.cn/flip/\">https://www.ak129.cn/flip/</a></p>")},
        {"status.ready", QStringLiteral("Open an image to start browsing")},
        {"status.position", QStringLiteral("%1 / %2")},
        {"usage",
         QStringLiteral("Usage: Flip [image-file-or-folder]\n"
                        "       Flip --self-test\n"
                        "       Flip --help")},
    };
}

const QHash<QString, QString>& tableFor(Lang lang)
{
    static const QHash<QString, QString> zh = zhTable();
    static const QHash<QString, QString> en = enTable();
    return lang == Lang::ZhCN ? zh : en;
}

} // namespace

void setLang(Lang lang)
{
    g_pref = lang;
}

Lang lang()
{
    return g_pref;
}

Lang resolved()
{
    if (g_pref != Lang::Auto)
        return g_pref;
    const QLocale loc = QLocale::system();
    if (loc.language() == QLocale::Chinese)
        return Lang::ZhCN;
    return Lang::En;
}

QString t(const char* key)
{
    const auto& table = tableFor(resolved());
    const QString k = QString::fromLatin1(key);
    const auto it = table.constFind(k);
    if (it != table.cend())
        return it.value();
    return k;
}

} // namespace I18n
