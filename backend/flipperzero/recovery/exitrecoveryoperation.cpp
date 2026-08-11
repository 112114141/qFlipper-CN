#include "exitrecoveryoperation.h"

#include "flipperzero/recovery.h"
#include "flipperzero/devicestate.h"

using namespace Flipper;
using namespace Zero;

ExitRecoveryOperation::ExitRecoveryOperation(Recovery *recovery, QObject *parent):
    AbstractRecoveryOperation(recovery, parent)
{}

const QString ExitRecoveryOperation::description() const
{
    return QStringLiteral("退出恢复模式 @%1").arg(deviceState()->name());
}

void ExitRecoveryOperation::nextStateLogic()
{
    if(operationState() == BasicOperationState::Ready) {
        setOperationState(OperationState::WaitingForOnline);
        exitRecovery();

    } else if(operationState() == OperationState::WaitingForOnline) {
        finish();
    }
}

void ExitRecoveryOperation::onOperationTimeout()
{
    finishWithError(BackendError::RecoveryError, QStringLiteral("无法退出恢复模式：操作超时"));
}

void ExitRecoveryOperation::exitRecovery()
{
    if(!recovery()->exitRecoveryMode()) {
        finishWithError(BackendError::RecoveryError, recovery()->errorString());
    } else {
        startTimeout();
    }
}
