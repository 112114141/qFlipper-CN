#include "correctoptionbytesoperation.h"

#include <QDebug>
#include <QIODevice>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(LOG_RECOVERY)

#include "flipperzero/devicestate.h"
#include "flipperzero/recovery.h"

using namespace Flipper;
using namespace Zero;

CorrectOptionBytesOperation::CorrectOptionBytesOperation(Recovery *recovery, QIODevice *file, QObject *parent):
    AbstractRecoveryOperation(recovery, parent),
    m_file(file)
{}

const QString CorrectOptionBytesOperation::description() const
{
    return QStringLiteral("修正选项字节 @%1").arg(deviceState()->name());
}

void CorrectOptionBytesOperation::nextStateLogic()
{
    if(operationState() == AbstractOperation::Ready) {
        setOperationState(CorrectOptionBytesOperation::CorrectingOptionBytes);
        correctOptionBytes();

    } else if(operationState() == CorrectOptionBytesOperation::CorrectingOptionBytes) {
        finish();
    } else {}
}

void CorrectOptionBytesOperation::onOperationTimeout()
{
    if(!deviceState()->isOnline()) {
        finishWithError(BackendError::RecoveryError, QStringLiteral("写入修正后的选项字节失败: 操作超时"));
    } else {
        qCDebug(LOG_RECOVERY) << "设备在线时超时，假设其仍然可用";
        advanceOperationState();
    }
}

void CorrectOptionBytesOperation::correctOptionBytes()
{
    if(!recovery()->downloadOptionBytes(m_file)) {
        finishWithError(BackendError::RecoveryError, recovery()->errorString());
    } else {
        startTimeout();
    }
}
