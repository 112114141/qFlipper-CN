#include "applicationupdater.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QLoggingCategory>

#include "preferences.h"
#include "remotefilefetcher.h"

Q_LOGGING_CATEGORY(CATEGORY_SELFUPDATES, "SFU")

using namespace Flipper;
using namespace Updates;

ApplicationUpdater::ApplicationUpdater(QObject *parent):
    QObject(parent),
    m_state(Idle),
    m_progress(0)
{}

void ApplicationUpdater::reset()
{
    setState(Idle);
    setProgress(0);
}

ApplicationUpdater::State ApplicationUpdater::state() const
{
    return m_state;
}

double ApplicationUpdater::progress() const
{
    return m_progress;
}

// TODO: Handle -rcxx suffixes correctly
bool ApplicationUpdater::canUpdate(const Flipper::Updates::VersionInfo &versionInfo) const
{
    const auto appDate = QDateTime::fromSecsSinceEpoch(APP_TIMESTAMP).date();
    const auto appVersion = QStringLiteral(APP_VERSION);
    const auto appCommit = QStringLiteral(APP_COMMIT);

    if(!globalPrefs->checkApplicationUpdates()) {
        return false;
    } else if(versionInfo.date() > appDate) {
        return true;
    } else if(globalPrefs->applicationUpdateChannel() == QStringLiteral("development")) {
        return (versionInfo.date() == appDate) && (versionInfo.number() != appCommit);
    } else if(globalPrefs->applicationUpdateChannel() == QStringLiteral("release-candidate")) {
        return VersionInfo::compare(versionInfo.number(), appVersion) > 0;
    } else if(globalPrefs->applicationUpdateChannel() == QStringLiteral("release")) {
        return VersionInfo::compare(versionInfo.number(), appVersion) > 0;
    } else {
        return false;
    }
}

void ApplicationUpdater::installUpdate(const Flipper::Updates::VersionInfo &versionInfo)
{
#ifdef Q_OS_WINDOWS
    const auto fileInfo = versionInfo.fileInfo(QStringLiteral("installer"), QStringLiteral("windows/amd64"));
#elif defined(Q_OS_MAC)
    const auto fileInfo = versionInfo.fileInfo(QStringLiteral("dmg"), QStringLiteral("macos/amd64"));
#elif defined(Q_OS_LINUX)
    const auto fileInfo = versionInfo.fileInfo(QStringLiteral("AppImage"), QStringLiteral("linux/amd64"));
#else
#error "Unsupported OS"
#endif

    const auto fileName = QFileInfo(fileInfo.url()).fileName();

#if defined(Q_OS_WINDOWS) || defined(Q_OS_MAC)
    const auto filePath = QDir::temp().absoluteFilePath(fileName);
#elif defined(Q_OS_LINUX)
    const auto filePath = QDir::current().absoluteFilePath(fileName);
#else
    #error "Unsupported OS"
#endif

    auto *file = new QFile(filePath + QStringLiteral(".part"));
    auto *fetcher = new RemoteFileFetcher(this);

    const auto  cleanup = [=]() {
        file->deleteLater();
        fetcher->deleteLater();
    };

    connect(fetcher, &RemoteFileFetcher::finished, this, [=]() {
        if(fetcher->isError()) {
            qCWarning(CATEGORY_SELFUPDATES).noquote() << "下载应用程序更新包失败:" << fetcher->errorString();
            setState(ErrorOccured);

            cleanup();
            return;
        }

        qCInfo(CATEGORY_SELFUPDATES) << "应用程序更新包已下载完成。";
        setState(Updating);

        // IMPORTANT -- The file is closed automatically before renaming (https://doc.qt.io/qt-5/qfile.html#rename)
        QFile oldFile(filePath);
        if(oldFile.exists() && !oldFile.remove()) {
            qCDebug(CATEGORY_SELFUPDATES).noquote() << "删除旧更新包失败:" << oldFile.fileName();
            setState(ErrorOccured);

            cleanup();
            return;

        } else if(!file->rename(filePath)) {
            qCDebug(CATEGORY_SELFUPDATES).noquote() << "重命名 .part 文件失败:" << file->fileName();
            setState(ErrorOccured);

            file->remove();
            cleanup();
            return;
        }

    #if defined(Q_OS_LINUX)

        const auto executable = QFile::Permission::ExeUser  | QFile::Permission::ExeOwner |
                                QFile::Permission::ExeGroup | QFile::Permission::ExeOther;

        file->setPermissions(file->permissions() | executable);

    #endif

        cleanup();

        if(!performUpdate(filePath)) {
            qCWarning(CATEGORY_SELFUPDATES) << "启动应用程序更新进程失败。";
            setState(ErrorOccured);
        }
    });

    connect(fetcher, &RemoteFileFetcher::progressChanged, this, &ApplicationUpdater::setProgress);

    if(!fetcher->fetch(fileInfo, file)) {
        qCWarning(CATEGORY_SELFUPDATES) << "开始下载更新包失败。";
        setState(ErrorOccured);

        file->remove();
        cleanup();

    } else {
        qCWarning(CATEGORY_SELFUPDATES) << "正在下载应用程序更新包...";
        setState(Downloading);
    }
}

void ApplicationUpdater::setState(ApplicationUpdater::State state)
{
    if(m_state == state) {
        return;
    }

    m_state = state;
    emit stateChanged();
}

void ApplicationUpdater::setProgress(double progress)
{
    if(qFuzzyCompare(m_progress, progress)) {
        return;
    }

    m_progress = progress;
    emit progressChanged();
}

bool ApplicationUpdater::performUpdate(const QString &path)
{
    const auto exitApplication = []() {
        qCInfo(CATEGORY_SELFUPDATES) << "更新已启动，正在退出应用程序...";
        QCoreApplication::exit(0);
    };

#if defined(Q_OS_WINDOWS)
    const auto success = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    if(success) exitApplication();
    return success;

#elif defined(Q_OS_MAC)
    auto *mountDmg = new QProcess(this);
    mountDmg->setProgram(QStringLiteral("open"));
    mountDmg->setArguments({path});

    connect(mountDmg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus exitStatus) {

        mountDmg->deleteLater();

        if(!exitCode && exitStatus == QProcess::NormalExit) {
            exitApplication();
        } else {
            qCWarning(CATEGORY_SELFUPDATES) << "打开磁盘镜像失败。";
        }
    });

    mountDmg->start();
    return mountDmg->error() == QProcess::UnknownError; //Really? no NoError code?

#elif defined(Q_OS_LINUX)
    const auto success = QProcess::startDetached(path);
    if(success) exitApplication();
    return success;

#else
#error "Unsupported OS"
#endif
}
