#include "systemfactoryresetoperation.h"
#include "protobufplugininterface.h"

using namespace Flipper;
using namespace Zero;

SystemFactoryResetOperation::SystemFactoryResetOperation(uint32_t id, QObject *parent):
    AbstractProtobufOperation(id, parent)
{}

const QString SystemFactoryResetOperation::description() const
{
    return QStringLiteral("恢复出厂设置");
}

const QByteArray SystemFactoryResetOperation::encodeRequest(ProtobufPluginInterface *encoder)
{
    finishLater();
    return encoder->systemFactoryReset(id());
}
