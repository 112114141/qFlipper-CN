#include "abstractutilityoperation.h"

#include <QTimer>

#include "flipperzero/devicestate.h"

using namespace Flipper;
using namespace Zero;

AbstractUtilityOperation::AbstractUtilityOperation(ProtobufSession *rpc, DeviceState *deviceState, QObject *parent):
    AbstractOperation(parent),
    m_rpc(rpc),
    m_deviceState(deviceState)
{}

void AbstractUtilityOperation::start()
{
    if(operationState() != AbstractOperation::Ready) {
        finishWithError(BackendError::UnknownError, QStringLiteral("试图启动一个已经在运行或已完成的操作。"));
    } else {
        emit started();
        advanceOperationState();
    }
}

ProtobufSession *AbstractUtilityOperation::rpc() const
{
    return m_rpc;
}

DeviceState *AbstractUtilityOperation::deviceState() const
{
    return m_deviceState;
}

void AbstractUtilityOperation::advanceOperationState()
{
    QTimer::singleShot(0, this, &AbstractUtilityOperation::nextStateLogic);
}
