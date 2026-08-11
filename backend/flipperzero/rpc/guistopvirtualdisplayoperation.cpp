#include "guistopvirtualdisplayoperation.h"

#include "protobufplugininterface.h"

using namespace Flipper;
using namespace Zero;

GuiStopVirtualDisplayOperation::GuiStopVirtualDisplayOperation(uint32_t id, QObject *parent):
    AbstractProtobufOperation(id, parent)
{}

const QString GuiStopVirtualDisplayOperation::description() const
{
    return QStringLiteral("界面停止虚拟显示");
}

const QByteArray GuiStopVirtualDisplayOperation::encodeRequest(ProtobufPluginInterface *encoder)
{
    return encoder->guiStopVirtualDisplay(id());
}
