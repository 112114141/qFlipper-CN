#include "recovery.h"

#include "devicestate.h"
#include "dfusefile.h"
#include "debug.h"

#include "device/stm32wb55.h"

using namespace Flipper;
using namespace Zero;

using namespace STM32;
using namespace WB55;

/* ----------------------------------------------------------------------------------------------------------------------------------
 * FUS operations are based on the info from AN5185
 * https://www.st.com/resource/en/application_note/dm00513965-st-firmware-upgrade-services-for-stm32wb-series-stmicroelectronics.pdf
 * ---------------------------------------------------------------------------------------------------------------------------------- */

#define to_hex_str(num) (QString::number(num, 16))

Recovery::Recovery(DeviceState *deviceState, QObject *parent):
    QObject(parent),
    m_deviceState(deviceState)
{}

Recovery::~Recovery()
{}

DeviceState *Recovery::deviceState() const
{
    return m_deviceState;
}

bool Recovery::exitRecoveryMode()
{
    m_deviceState->setStatusString(QStringLiteral("正在退出恢复模式..."));

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);
    const auto success = device.beginTransaction() && device.leave();

    begin_ignore_block();
    device.endTransaction();
    end_ignore_block();

    if(!success) {
        setErrorString("退出恢复模式失败");
    }

    return success;
}

bool Recovery::setBootMode(BootMode mode)
{
    const auto msg = (mode == BootMode::Normal) ?
               QStringLiteral("正在设置 OS 启动模式...") :
               QStringLiteral("正在设置恢复启动模式...");

    m_deviceState->setStatusString(msg);

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    if(!device.beginTransaction()) {
        setErrorString("无法设置启动模式: 启动事务失败。");
        return false;
    }

    auto ob = device.optionBytes();

    if(!ob.isValid()) {
        setErrorString("无法设置启动模式: 读取选项字节失败。");
        return false;
    }

    ob.setValue("nBOOT0", mode == BootMode::Normal);
    ob.setValue("nSWBOOT0", mode == BootMode::Normal);

    const auto success = device.setOptionBytes(ob);

    if(!success) {
        setErrorString("无法设置启动模式: 设置选项字节失败");
    }

    begin_ignore_block();
    device.endTransaction();
    end_ignore_block();

    return success;
}

Recovery::WirelessStatus Recovery::wirelessStatus()
{
    debug_msg("正在获取协处理器(无线)状态...");

    if(!m_deviceState->isOnline()) {
        debug_msg("获取 FUS 状态失败。设备当前处于离线状态。");
        return WirelessStatus::Invalid;
    }

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    if(!device.beginTransaction()) {
        debug_msg("获取 FUS 状态失败。如果设备刚刚重启，这是正常的。");
        return WirelessStatus::Invalid;
    }

    const auto state = device.FUSGetState();
    if(!state.isValid()) {
        debug_msg("获取 FUS 状态失败。如果设备刚刚重启，这是正常的。");
        return WirelessStatus::Invalid;
    }

    if(!device.endTransaction()) {
        debug_msg("获取 FUS 状态失败。如果设备刚刚重启，这是正常的。");
        return WirelessStatus::Invalid;
    }

    const auto status = state.status();
    const auto error = state.error();

    debug_msg(QStringLiteral("当前 FUS 状态: %1, %2。").arg(state.statusString(), state.errorString()));

    if((status == FUSState::Idle) && (error == FUSState::NoError)) {
        return WirelessStatus::FUSRunning;
    } else if(status == FUSState::ErrorOccured) {
         if(error == FUSState::NotRunning)
            return WirelessStatus::WSRunning;
         else
             return WirelessStatus::ErrorOccured;

    } else {
        return WirelessStatus::UnhandledState;
    }
}

bool Recovery::startFUS()
{
    m_deviceState->setStatusString("正在启动固件升级服务(FUS)...");

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    if(!device.beginTransaction()) {
        setErrorString("无法启动 FUS: 启动事务失败。");
        return false;
    }

    auto state = device.FUSGetState();
    auto success = state.isValid();

    if(!success) {
        setErrorString("无法启动 FUS: 获取 FUS 状态失败。");

    } else if((state.status() == FUSState::Idle) && (state.error() == FUSState::NoError)) {
        debug_msg("FUS 已在运行，为保持一致性正在重启...");
        success = device.leave();

    } else if((state.status() == FUSState::ErrorOccured) && (state.error() == FUSState::NotRunning)) {
        debug_msg(QString("FUS 似乎未在运行: %1, %2。").arg(state.statusString(), state.errorString()));

        // Send a second GET_STATE to actually start FUS
        begin_ignore_block();
        state = device.FUSGetState();
        end_ignore_block();

    } else {
        setErrorString("无法启动 FUS: 意外的 FUS 状态。");
        success = false;
    }

    begin_ignore_block();
    device.endTransaction();
    end_ignore_block();

    // At this point, there is no way to know whether FUS has actually started, but things are looking as expected.
    return success;
}

// TODO: check status to see if the wireless stack is present at all
bool Recovery::startWirelessStack()
{
    m_deviceState->setStatusString("正在尝试启动无线协议栈...");

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    auto success = device.beginTransaction() && device.FUSStartWirelessStack();
    check_continue(device.endTransaction(), "^^^ 此时可能无影响... ^^^");

    if(!success) {
        setErrorString("启动无线协议栈失败。");
    }

    return success;
}

bool Recovery::deleteWirelessStack()
{
    m_deviceState->setStatusString("正在删除旧的协处理器固件...");

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    const auto success = device.beginTransaction() && device.FUSFwDelete() && device.endTransaction();

    if(!success) {
        setErrorString("无法删除旧的协处理器固件: 启动无线协议栈固件移除失败。");
    }

    return success;
}

bool Recovery::downloadFirmware(QIODevice *file)
{
    if(!file->open(QIODevice::ReadOnly)) {
        setErrorString("无法刷写固件: 打开文件失败。");
        return false;

    } else if(file->bytesAvailable() <= 0) {
        setErrorString("无法刷写固件: 文件为空。");
        return false;

    } else {
        m_deviceState->setStatusString("正在刷写固件...");
    }

    DfuseFile fw(file);
    DfuseDevice dev(m_deviceState->deviceInfo().usbInfo);

    file->close();

    connect(&dev, &DfuseDevice::progressChanged, this, [=](int operation, double progress) {
        m_deviceState->setProgress(progress / 2.0 + (operation == DfuseDevice::Download ? 50 : 0));
    });

    const auto success = dev.beginTransaction() && dev.download(&fw) && dev.endTransaction();

    if(!success) {
        setErrorString("无法刷写固件: 操作过程中发生错误。");
    }

    return success;
}

bool Recovery::downloadWirelessStack(QIODevice *file, uint32_t addr)
{
    debug_msg("正在尝试刷写协处理器固件镜像...");

    if(!file->open(QIODevice::ReadOnly)) {
        setErrorString("无法刷写协处理器固件镜像: 打开文件失败。");
        return false;

    } else if(!file->bytesAvailable()) {
        setErrorString("无法刷写协处理器固件镜像: 文件为空。");
        return false;

    } else {
        m_deviceState->setStatusString("正在刷写协处理器固件镜像...");
    }

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    if(!device.beginTransaction()) {
        setErrorString("无法刷写协处理器固件镜像: 启动事务失败。");
        return false;
    }

    if(!addr) {
        const auto ob = device.optionBytes();

        if(!ob.isValid()) {
            setErrorString("无法刷写协处理器固件镜像: 读取选项字节失败。");
            return false;
        }

        const auto origin = device.partitionOrigin((uint8_t)STM32WB55::Partition::Flash);
        const auto pageSize = (uint32_t)0x1000; // TODO: do not hardcode page size

        addr = (origin + (pageSize * ob.value("SFSA")) - file->bytesAvailable()) & (~(pageSize - 1));

        debug_msg(QString("SFSA 值为 0x%1").arg(QString::number(ob.value("SFSA"), 16)));
        debug_msg(QString("协处理器固件镜像的目标地址为 0x%1").arg(QString::number(addr, 16)));

    } else {
        debug_msg(QString("协处理器固件镜像的目标地址已被覆盖为 0x%1").arg(QString::number(addr, 16)));
    }

    connect(&device, &DfuseDevice::progressChanged, this, [=](int operation, double progress) {
        m_deviceState->setProgress(progress / 2.0 + (operation == DfuseDevice::Download ? 50 : 0));
    });

    bool success;

    if(!(success = device.erase(addr, file->bytesAvailable()))) {
        setErrorString("无法刷写协处理器固件镜像: 擦除内部存储失败。");
    } else if(!(success = device.download(file, addr, 0))) {
        setErrorString("无法刷写协处理器固件镜像: 写入内部存储失败。");
    } else if(!(success = device.endTransaction())) {
        setErrorString("无法刷写协处理器固件镜像: 结束事务失败。");
    } else {}

    file->close();

    return success;
}

bool Recovery::upgradeWirelessStack()
{
    debug_msg("正在发送 FW_UPGRADE 命令...");

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    const auto success = device.beginTransaction() && device.FUSFwUpgrade();
    check_continue(device.endTransaction(), "^^^ 此时可能无影响... ^^^");

    if(!success) {
        setErrorString("无法升级协处理器固件: 启动安装失败。");
    } else {
        m_deviceState->setStatusString("正在升级协处理器固件，请稍候...");
    }

    return success;
}

bool Recovery::checkWirelessStack()
{
    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    if(!device.beginTransaction()) {
        setErrorString(QStringLiteral("读取协处理器固件版本信息失败"));
        return false;
    }

    const auto versionInfo = device.versionInfo();

    if(!device.endTransaction()) {
        setErrorString(QStringLiteral("读取协处理器固件版本信息失败"));
        return false;
    }

    qCDebug(CATEGORY_DEBUG).noquote() << "FUS 版本:" << versionInfo.FUSVersion;
    qCDebug(CATEGORY_DEBUG).noquote() << "无线协议栈版本:" << versionInfo.WirelessVersion;

    return versionInfo.WirelessVersion != QStringLiteral("0.0.0");
}

bool Recovery::downloadOptionBytes(QIODevice *file)
{
    m_deviceState->setStatusString("正在下载选项字节...");

    check_return_bool(file->open(QIODevice::ReadOnly), "打开文件读取失败");
    const OptionBytes loaded(file);
    file->close();

    check_return_bool(loaded.isValid(), "从文件加载选项字节失败");

    STM32WB55 device(m_deviceState->deviceInfo().usbInfo);

    check_return_bool(device.beginTransaction(), "启动事务失败");
    const OptionBytes actual = device.optionBytes();

    const auto diff = actual.compare(loaded);

    bool success = false;

    if(diff.isEmpty()) {
        debug_msg("选项字节正常");

        success = device.leave();

        if(!success) {
            setErrorString("无法设置启动模式: 退出恢复模式失败。");
        }

    } else {
        for(auto it = diff.constKeyValueBegin(); it != diff.constKeyValueEnd(); ++it) {
            debug_msg(QString("选项字节不匹配 @%1: 当前: 0x%2, 其他: 0x%3")
                     .arg((*it).first, to_hex_str(actual.value((*it).first)), to_hex_str((*it).second)));
        }

        debug_msg("正在写入修正后的选项字节...");

        success = device.setOptionBytes(actual.corrected(diff));

        if(!success) {
            setErrorString("无法设置启动模式: 设置选项字节失败");
        }
    }

    begin_ignore_block();
    device.endTransaction();
    end_ignore_block();

    if(success) {
        m_deviceState->setStatusString(QStringLiteral("正在退出恢复模式..."));
    }

    return success;
}
