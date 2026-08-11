#include "startrpcoperation.h"

#include <QSerialPort>

using namespace Flipper;
using namespace Zero;

StartRPCOperation::StartRPCOperation(QSerialPort *serialPort, QObject *parent):
    AbstractSerialOperation(serialPort, parent)
{}

const QString StartRPCOperation::description() const
{
    return QStringLiteral("启动 RPC 会话 @%1").arg(QString(serialPort()->portName()));
}

void StartRPCOperation::onSerialPortReadyRead()
{
    serialPort()->startTransaction();

    if(!serialPort()->readAll().endsWith(s_cmd + '\n')) {
        serialPort()->rollbackTransaction();
    } else {
        serialPort()->commitTransaction();
        finish();
    }
}

void StartRPCOperation::onOperationTimeout()
{
    if(operationState() == State::LeavingCli) {
        finishWithError(BackendError::SerialError, QStringLiteral("无法启动 RPC 会话"));
    } else if(operationState() == State::WaitingForPing) {
        finishWithError(BackendError::ProtocolError, QStringLiteral("设备未响应 ping"));
    }
}

bool StartRPCOperation::begin()
{
    setOperationState(State::LeavingCli);
    return (serialPort()->write(s_cmd) == s_cmd.size()) && serialPort()->flush();
}

const QByteArray StartRPCOperation::s_cmd("start_rpc_session\r");
