#include "wirelessstackdownloadoperation.h"

#include <QDebug>
#include <QTimer>
#include <QIODevice>
#include <QFutureWatcher>
#include <QLoggingCategory>
#include <QtConcurrent/QtConcurrentRun>

#include "flipperzero/devicestate.h"
#include "flipperzero/recovery.h"

Q_DECLARE_LOGGING_CATEGORY(LOG_RECOVERY)

using namespace Flipper;
using namespace Zero;

static constexpr int INSTALL_TRY_COUNT = 3;
static constexpr int CHECK_TRY_COUNT = 3;
static constexpr int TIMER_INTERVAL_MS = 1000;

WirelessStackDownloadOperation::WirelessStackDownloadOperation(Recovery *recovery, QIODevice *file, uint32_t targetAddress, QObject *parent):
    AbstractRecoveryOperation(recovery, parent),
    m_file(file),
    m_loopTimer(new QTimer(this)),
    m_targetAddress(targetAddress),
    m_installTryCount(INSTALL_TRY_COUNT)
{
    m_loopTimer->setInterval(TIMER_INTERVAL_MS);

    connect(m_loopTimer, &QTimer::timeout, this, &WirelessStackDownloadOperation::nextStateLogic);
    connect(this, &AbstractOperation::finished, m_loopTimer, &QTimer::stop);
}

const QString WirelessStackDownloadOperation::description() const
{
    return QStringLiteral("协处理器固件下载 @%1").arg(deviceState()->name());
}

void WirelessStackDownloadOperation::nextStateLogic()
{
    if(operationState() == Ready) {
        setOperationState(StartingFUS);
        startFUS();

    } else if(operationState() == StartingFUS) {
        setOperationState(DeletingWirelessStack);
        deleteWirelessStack();

    } else if(operationState() == DeletingWirelessStack) {
        if(isWirelessStackDeleted()) {
            setOperationState(DownloadingWirelessStack);
            downloadWirelessStack();
        }

    } else if(operationState() == DownloadingWirelessStack) {
        setOperationState(UpgradingWirelessStack);
        upgradeWirelessStack();

    } else if(operationState() == UpgradingWirelessStack) {
        if(isWirelessStackUpgraded()) {
            setOperationState(CheckingWirelessStack);
            checkWirelessStack();
        }

    } else if(operationState() == CheckingWirelessStack) {
        if(isWirelessStackOK()) {
            finish();
        } else {
            tryAgain();
        }
    }
}

void WirelessStackDownloadOperation::onOperationTimeout()
{
    QString msg;

    if(operationState() == StartingFUS) {
        msg = QStringLiteral("启动固件升级服务失败: 操作超时。");
    } else if(operationState() == DeletingWirelessStack) {
        msg = QStringLiteral("删除现有无线协议栈失败: 操作超时。");
    } else if(operationState() == UpgradingWirelessStack) {
        msg = QStringLiteral("升级无线协议栈失败: 操作超时。");
    } else {
        msg = QStringLiteral("此处不应超时，可能是 bug。");
    }

    if(!deviceState()->isOnline()) {
        finishWithError(BackendError::RecoveryError, msg);
    } else {
        qCDebug(LOG_RECOVERY) << "设备在线时超时，假设其仍然可用";
        advanceOperationState();
    }
}

void WirelessStackDownloadOperation::startFUS()
{
    if(!recovery()->startFUS()) {
        finishWithError(BackendError::RecoveryError, recovery()->errorString());
    } else {
        startTimeout();
    }
}

void WirelessStackDownloadOperation::deleteWirelessStack()
{
    if(!recovery()->deleteWirelessStack()) {
        finishWithError(BackendError::RecoveryError, recovery()->errorString());
    } else {
        m_loopTimer->start();
    }
}

bool WirelessStackDownloadOperation::isWirelessStackDeleted()
{
    const auto status = recovery()->wirelessStatus();

    const auto waitNext = (status == Recovery::WirelessStatus::Invalid) ||
                          (status == Recovery::WirelessStatus::UnhandledState);
    if(waitNext) {
        return false;
    }

    m_loopTimer->stop();

    const auto errorOccured = (status == Recovery::WirelessStatus::WSRunning) ||
                              (status == Recovery::WirelessStatus::ErrorOccured);
    if(errorOccured) {
        finishWithError(BackendError::RecoveryError, QStringLiteral("完成无线协议栈移除失败。"));
    }

    return !errorOccured;
}

void WirelessStackDownloadOperation::downloadWirelessStack()
{
    auto *watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcherBase::finished, this, [=]() {
        if(watcher->result()) {
            advanceOperationState();
        } else {
            finishWithError(BackendError::RecoveryError, QStringLiteral("下载无线协议栈失败。"));
        }

        watcher->deleteLater();
    });
#if QT_VERSION < 0x060000
    watcher->setFuture(QtConcurrent::run(recovery(), &Recovery::downloadWirelessStack, m_file, m_targetAddress));
#else
    watcher->setFuture(QtConcurrent::run(&Recovery::downloadWirelessStack, recovery(), m_file, m_targetAddress));
#endif
}

void WirelessStackDownloadOperation::upgradeWirelessStack()
{
    if(!recovery()->upgradeWirelessStack()) {
        finishWithError(BackendError::RecoveryError, recovery()->errorString());
    } else {
        m_loopTimer->start();
    }
}

bool WirelessStackDownloadOperation::isWirelessStackUpgraded()
{
    const auto status = recovery()->wirelessStatus();

    const auto waitNext = (status == Recovery::WirelessStatus::Invalid) ||
                          (status == Recovery::WirelessStatus::UnhandledState);
    if(waitNext) {
        return false;
    }

    m_loopTimer->stop();

    const auto errorOccured = (status == Recovery::WirelessStatus::ErrorOccured);
    if(errorOccured) {
        finishWithError(BackendError::RecoveryError, QStringLiteral("完成无线协议栈安装失败。"));
    }

    return !errorOccured;
}

void WirelessStackDownloadOperation::checkWirelessStack()
{
    m_checkTryCount = CHECK_TRY_COUNT;
    m_loopTimer->start();

    advanceOperationState();
}

bool WirelessStackDownloadOperation::isWirelessStackOK()
{
    return recovery()->checkWirelessStack();
}

void WirelessStackDownloadOperation::tryAgain()
{
    if(--m_checkTryCount > 0) {
        qCDebug(LOG_RECOVERY) << "无线协议栈检查似乎失败，正在重试...";

    } else if(--m_installTryCount > 0) {
        qCDebug(LOG_RECOVERY) << "无线协议栈安装似乎失败，正在重试...";

        m_loopTimer->stop();

        setOperationState(Ready);
        advanceOperationState();

    } else {
        finishWithError(BackendError::RecoveryError, QStringLiteral("多次尝试后仍无法安装无线协议栈，放弃"));
    }
}
