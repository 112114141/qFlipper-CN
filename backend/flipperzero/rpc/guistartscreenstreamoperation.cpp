#include "guistartscreenstreamoperation.h"

#include "protobufplugininterface.h"

using namespace Flipper;
using namespace Zero;

GuiStartScreenStreamOperation::GuiStartScreenStreamOperation(uint32_t id, QObject *parent):
    AbstractProtobufOperation(id, parent)
{}

const QString GuiStartScreenStreamOperation::description() const
{
    return QStringLiteral("界面启动屏幕串流");
}

const QByteArray GuiStartScreenStreamOperation::encodeRequest(ProtobufPluginInterface *encoder)
{
    return encoder->guiStartScreenStream(id());
}
