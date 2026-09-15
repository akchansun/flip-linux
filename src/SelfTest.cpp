#include "SelfTest.h"

#include "AppInfo.h"
#include "FitZoom.h"
#include "FolderPager.h"
#include "I18n.h"
#include "LaunchSettings.h"
#include "NaturalSort.h"
#include "UpdateFeed.h"
#include "VersionCompare.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QSettings>
#include <QTemporaryDir>
#include <cmath>
#include <cstdio>

namespace {

int g_fails = 0;

void expect(bool cond, const char* msg)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", msg);
        ++g_fails;
    } else {
        std::printf("ok  %s\n", msg);
    }
}

void expectNear(double actual, double expected, const char* msg)
{
    const bool ok = std::fabs(actual - expected) < 1e-9;
    if (!ok)
        std::fprintf(stderr, "FAIL: %s (got %f, expected %f)\n", msg, actual, expected);
    expect(ok, msg);
}

QImage solidImage(int w, int h, QRgb color)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(color);
    return img;
}

bool saveOrLog(const QImage& img, const QString& path)
{
    if (img.save(path))
        return true;
    std::fprintf(stderr, "note: could not write %s\n", qPrintable(path));
    return false;
}

} // namespace

int runSelfTest()
{
    std::printf("Flip self-test\n");

    expectNear(defaultFitScale(QSize(200, 150), QSize(800, 600)), 1.0, "small image stays 100%");
    expectNear(defaultFitScale(QSize(800, 600), QSize(800, 600)), 1.0, "exact fit stays 100%");
    expectNear(defaultFitScale(QSize(1920, 1080), QSize(800, 600)), 800.0 / 1920.0,
               "wide image scales down to width");
    expectNear(defaultFitScale(QSize(400, 1600), QSize(800, 600)), 600.0 / 1600.0,
               "tall image scales down to height");
    expect(defaultFitScale(QSize(4000, 3000), QSize(1000, 1000)) < 1.0, "large image never upscales");
    expectNear(defaultFitScale(QSize(10, 10), QSize(1000, 1000)), 1.0, "tiny image never upscales");

    expect(naturalLessThan(QStringLiteral("img2.png"), QStringLiteral("img10.png")),
           "natural sort 2 < 10");
    expect(naturalLessThan(QStringLiteral("img1.png"), QStringLiteral("img2.png")),
           "natural sort 1 < 2");
    expect(!naturalLessThan(QStringLiteral("img10.png"), QStringLiteral("img2.png")),
           "natural sort 10 not < 2");

    expect(QStringLiteral(FLIP_VERSION) == QStringLiteral("1.1.0"), "app version 1.1.0");
    qunsetenv("FLIP_UPDATE_FEED");
    expect(compareVersions(QStringLiteral("1.1.0"), QStringLiteral("1.0.0")) > 0, "1.1.0 > 1.0.0");
    expect(compareVersions(QStringLiteral("1.0.0"), QStringLiteral("1.1.0")) < 0, "1.0.0 < 1.1.0");
    expect(compareVersions(QStringLiteral("1.0"), QStringLiteral("1.0.0")) == 0, "1.0 == 1.0.0");
    expect(compareVersions(QStringLiteral("v1.2.0"), QStringLiteral("1.1.9")) > 0, "v1.2.0 > 1.1.9");
    expect(compareVersions(QStringLiteral("1.10.0"), QStringLiteral("1.9.0")) > 0, "1.10.0 > 1.9.0");
    expect(isNewerVersion(QStringLiteral("1.2.0"), QStringLiteral("1.1.0")), "remote 1.2.0 is newer");
    expect(!isNewerVersion(QStringLiteral("1.0.0"), QStringLiteral("1.1.0")), "older remote is not newer");
    expect(!isNewerVersion(QStringLiteral("1.1.0"), QStringLiteral("1.1.0")), "same version is not newer");
    expect(!isNewerVersion(QString(), QStringLiteral("1.1.0")), "empty remote is not newer");

    const QByteArray sampleJson = QByteArrayLiteral(
        "{\n"
        "  \"schema\": 1,\n"
        "  \"product\": \"Flip\",\n"
        "  \"linux\": {\n"
        "    \"version\": \"1.0.0\",\n"
        "    \"notes\": {\n"
        "      \"zh\": \"同目录翻页看图；方向键与边缘点击翻页。需 Qt 6 运行库。\",\n"
        "      \"en\": \"Same-folder image paging; arrow keys and edge click. Needs Qt 6 runtime.\"\n"
        "    },\n"
        "    \"download\": {\n"
        "      \"site\": \"https://www.ak129.cn/flip/#linux\",\n"
        "      \"gitee\": \"https://gitee.com/akcg/flip-linux/releases/tag/v1.0.0\",\n"
        "      \"github\": \"https://github.com/akchansun/flip-linux/releases/tag/v1.0.0\"\n"
        "    }\n"
        "  }\n"
        "}\n");
    LinuxRelease rel;
    expect(parseLinuxRelease(sampleJson, &rel), "parse linux version.json");
    expect(rel.version == QStringLiteral("1.0.0"), "parsed linux version");
    expect(rel.downloadGitee.contains(QStringLiteral("gitee.com")), "parsed gitee url");
    expect(preferredDownloadUrl(rel, true).contains(QStringLiteral("gitee.com")),
           "china prefers gitee");
    expect(preferredDownloadUrl(rel, false).contains(QStringLiteral("ak129.cn")),
           "others prefer site");
    expect(!parseLinuxRelease(QByteArrayLiteral("{not json"), &rel), "reject invalid json");
    expect(!parseLinuxRelease(QByteArrayLiteral("{\"macos\":{}}"), &rel), "reject missing linux");
    expect(updateFeedUrl().toString().contains(QStringLiteral("/flip/version.json")),
           "default update feed url");

    {
        QTemporaryDir cfg;
        expect(cfg.isValid(), "settings temp dir");
        QSettings s(cfg.path() + QStringLiteral("/flip.ini"), QSettings::IniFormat);
        expect(LaunchSettings::shouldShowStartupTips(s), "tips shown by default");
        expect(LaunchSettings::shouldAskForUpdates(s), "update asked by default");
        LaunchSettings::setStartupTipsDontShow(s, true);
        LaunchSettings::setUpdateDontAsk(s, true);
        s.sync();
        QSettings s2(cfg.path() + QStringLiteral("/flip.ini"), QSettings::IniFormat);
        expect(!LaunchSettings::shouldShowStartupTips(s2), "tips dontShow persists");
        expect(!LaunchSettings::shouldAskForUpdates(s2), "update dontAsk persists");
        expect(s2.contains(LaunchSettings::tipsDontShowKey()), "tips settings key");
        expect(s2.contains(LaunchSettings::updateDontAskKey()), "update settings key");
    }

    I18n::setLang(I18n::Lang::ZhCN);
    expect(I18n::t("app.name") == QStringLiteral("看图"), "zh app name");
    expect(I18n::t("tips.dontShow") == QStringLiteral("不再提示"), "zh dont show tips");
    expect(I18n::t("update.dont") == QStringLiteral("不更新"), "zh dont update");
    expect(I18n::t("about.body").arg(QStringLiteral(FLIP_VERSION)).contains(QStringLiteral("1.1.0")),
           "zh about shows version");
    I18n::setLang(I18n::Lang::En);
    expect(I18n::t("app.name") == QStringLiteral("Flip"), "en app name");
    expect(I18n::t("tips.dontShow") == QStringLiteral("Don't show again"), "en dont show tips");
    expect(I18n::t("update.later") == QStringLiteral("Later"), "en later");

    QTemporaryDir tmp;
    expect(tmp.isValid(), "temp dir");
    const QString dir = tmp.path();

    const QImage red = solidImage(200, 150, qRgb(200, 40, 40));
    const QImage blue = solidImage(1920, 400, qRgb(40, 80, 180));
    const QImage green = solidImage(64, 64, qRgb(40, 160, 70));

    expect(saveOrLog(red, dir + QStringLiteral("/img2.png")), "write img2.png");
    expect(saveOrLog(blue, dir + QStringLiteral("/img10.png")), "write img10.png");
    expect(saveOrLog(green, dir + QStringLiteral("/img1.png")), "write img1.png");
    saveOrLog(red, dir + QStringLiteral("/photo.jpg"));
    saveOrLog(green, dir + QStringLiteral("/icon.bmp"));
    saveOrLog(red, dir + QStringLiteral("/scan.tif"));
    saveOrLog(green, dir + QStringLiteral("/sticker.webp"));

    FolderPager pager;
    expect(pager.open(dir + QStringLiteral("/img2.png")), "open img2.png");
    expect(pager.count() >= 3, "folder has at least 3 pngs");
    expect(pager.currentFileName() == QStringLiteral("img2.png"), "current is img2");

    // Natural order among img1 / img2 / img10 regardless of other extra formats.
    QStringList names;
    pager.first();
    for (int i = 0; i < pager.count(); ++i) {
        pager.jumpTo(i);
        names << pager.currentFileName();
    }
    const int i1 = names.indexOf(QStringLiteral("img1.png"));
    const int i2 = names.indexOf(QStringLiteral("img2.png"));
    const int i10 = names.indexOf(QStringLiteral("img10.png"));
    expect(i1 >= 0 && i2 >= 0 && i10 >= 0 && i1 < i2 && i2 < i10, "img1 < img2 < img10 in folder");

    pager.open(dir + QStringLiteral("/img1.png"));
    const QString first = pager.currentFileName();
    pager.previous(); // wrap to last
    expect(!pager.currentFileName().isEmpty() && pager.currentFileName() != first,
           "previous wraps from first");
    pager.next();
    expect(pager.currentFileName() == first, "next wraps back to first");

    QImage loaded(dir + QStringLiteral("/img10.png"));
    expect(!loaded.isNull() && loaded.width() == 1920 && loaded.height() == 400, "load wide png");
    expectNear(defaultFitScale(loaded.size(), QSize(800, 600)), 800.0 / 1920.0,
               "loaded wide png fit-zoom");

    std::printf("Qt image read formats:");
    for (const QByteArray& fmt : QImageReader::supportedImageFormats())
        std::printf(" %s", fmt.constData());
    std::printf("\nQt image write formats:");
    for (const QByteArray& fmt : QImageWriter::supportedImageFormats())
        std::printf(" %s", fmt.constData());
    std::printf("\n");

    if (QFile::exists(dir + QStringLiteral("/photo.jpg"))) {
        QImage jpg(dir + QStringLiteral("/photo.jpg"));
        expect(!jpg.isNull(), "read jpeg");
    }
    if (QFile::exists(dir + QStringLiteral("/sticker.webp"))) {
        QImage webp(dir + QStringLiteral("/sticker.webp"));
        expect(!webp.isNull(), "read webp");
    }
    if (QFile::exists(dir + QStringLiteral("/scan.tif"))) {
        QImage tif(dir + QStringLiteral("/scan.tif"));
        expect(!tif.isNull(), "read tiff");
    }

    expect(FolderPager::isImageFile(QStringLiteral("/tmp/a.png")), "png suffix recognized");
    expect(FolderPager::isImageFile(QStringLiteral("/tmp/a.JPEG")), "jpeg suffix recognized");
    expect(!FolderPager::isImageFile(QStringLiteral("/tmp/notes.txt")), "txt is not an image");

    if (g_fails == 0)
        std::printf("self-test passed\n");
    else
        std::fprintf(stderr, "self-test failed: %d check(s)\n", g_fails);
    return g_fails == 0 ? 0 : 1;
}
