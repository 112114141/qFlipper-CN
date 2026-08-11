#include "setbootmodeoperation.h"

#include <QDebug>
#include <QLoggingCategory>

#include "flipperzero/devicestate.h"
#include "flipperzero/recovery.h"

Q_DECLARE_LOGGING_CATEGORY(LOG_RECOVERY)

using namespace Flipper;
using namespace Zero;

SetBootModeOperation::SetBootModeOperation(Recovery *recovery, QObject *parent):
    AbstractRecoveryOperation(recovery, parent)
{}

const QString SetBootModeOperation::description() const
{
    return QStringLiteral("设置 %1 启动模式 @%2").arg(typeString(), deviceState()->name());
}

void SetBootModeOperation::nextStateLogic()
{
    if(operationState() == AbstractOperation::Ready) {
        setOperationState(SetBootModeOperation::WaitingForBoot);
        setBootMode();

    } else if(operationState() == SetBootModeOperation::WaitingForBoot) {
        finish();
    }
}

void SetBootModeOperation::onOperationTimeout()
{
    if(!deviceState()->isOnline()) {
        finishWithError(BackendError::RecoveryError, QStringLiteral("设置 %1 模式失败: 操作超时").arg(typeString()));
    } else {
        qCDebug(LOG_RECOVERY) << "设备在线时超时，假设其仍然可用";
        advanceOperationState();
    }
}

void SetBootModeOperation::setBootMode()
{
    if(!recovery()->setBootMode((Recovery::BootMode)bootMode())) {
        finishWithError(BackendError::RecoveryError, recovery()->errorString());
    } else {
        startTimeout();
    }
}

SetRecoveryBootOperation::SetRecoveryBootOperation(Recovery *recovery, QObject *parent):
    SetBootModeOperation(recovery, parent)
{}

int SetRecoveryBootOperation::bootMode() const
{
    return (int)Recovery::BootMode::DFUOnly;
}

const QString SetRecoveryBootOperation::typeString() const
{
    return QStringLiteral("恢复");
}

SetOSBootOperation::SetOSBootOperation(Recovery *recovery, QObject *parent):
    SetBootModeOperation(recovery, parent)
{}

int SetOSBootOperation::bootMode() const
{
    return (int)Recovery::BootMode::Normal;
}

const QString SetOSBootOperation::typeString() const
{
    return QStringLiteral("OS");
}
