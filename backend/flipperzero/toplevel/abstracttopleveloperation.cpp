#include "abstracttopleveloperation.h"

#include <QTimer>

#include "flipperzero/devicestate.h"

using namespace Flipper;
using namespace Zero;

AbstractTopLevelOperation::AbstractTopLevelOperation(DeviceState *deviceState, QObject *parent):
    AbstractOperation(parent),
    m_deviceState(deviceState)
{
    m_deviceState->clearError();
    m_deviceState->setProgress(-1);
    m_deviceState->setPersistent(true);
}

AbstractTopLevelOperation::~AbstractTopLevelOperation()
{
    m_deviceState->setPersistent(false);
}

DeviceState *AbstractTopLevelOperation::deviceState() const
{
    return m_deviceState;
}

void AbstractTopLevelOperation::start()
{
    if(operationState() != AbstractOperation::Ready) {
        finishWithError(BackendError::UnknownError, QStringLiteral("试图启动一个已经在运行或已完成的操作。"));
    } else {
        advanceOperationState();
    }
}

void AbstractTopLevelOperation::advanceOperationState()
{
    QTimer::singleShot(0, this, &AbstractTopLevelOperation::nextStateLogic);
}

void AbstractTopLevelOperation::registerSubOperation(AbstractOperation *operation)
{
    connect(operation, &AbstractOperation::finished, this, [=]() {
        if(operation->isError()) {
            onSubOperationError(operation);
        } else {
            advanceOperationState();
        }
    });
}

void AbstractTopLevelOperation::onSubOperationError(AbstractOperation *operation)
{
    // Default implementation. Ignoring the operation error here, as we need to provide a respective top-level error instead
    finishWithError(BackendError::OperationError, operation->errorString());
}
