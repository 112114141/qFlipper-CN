#include "cli.h"

#include <QDebug>
#include <QLoggingCategory>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "logger.h"
#include "preferences.h"

#include "flipperzero/flipperzero.h"
#include "flipperzero/devicestate.h"

Q_LOGGING_CATEGORY(LOG_CLI, "CLI")

Cli::Cli(int argc, char *argv[]):
    QCoreApplication(argc, argv),
    m_pendingOperation(NoOperation),
    m_repeatCount(1)
{
#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    initConnections();
    initLogger();
    initParser();

    processOptions();
    processArguments();

    qCInfo(LOG_CLI) << "正在等待设备...";
}

Cli::~Cli()
{}

void Cli::onBackendStateChanged()
{
    const auto state = m_backend.backendState();
    if(state == ApplicationBackend::BackendState::ErrorOccured) {
        qCCritical(LOG_CLI).nospace() << "发生错误: " << m_backend.errorType() << "。正在退出。";
        return exit(-1);

    } else if(state == ApplicationBackend::BackendState::WaitingForDevices) {
        qCCritical(LOG_CLI) << "所有设备已断开连接。正在退出。";
        return exit(0);

    } else if(state == ApplicationBackend::BackendState::Ready) {
        // Start the pending operation as soon as the device is ready...
        if(m_pendingOperation != DefaultAction) {
            startPendingOperation();
            return;

        } else if(m_backend.firmwareUpdateState() == ApplicationBackend::FirmwareUpdateState::ErrorOccured) {
            qCCritical(LOG_CLI) << "获取固件更新失败。正在退出。";
            return exit(-1);
        }

        const auto isFirmwareReady = m_backend.firmwareUpdateState() != ApplicationBackend::FirmwareUpdateState::Checking &&
                                     m_backend.firmwareUpdateState() != ApplicationBackend::FirmwareUpdateState::Unknown;
        if(isFirmwareReady) {
            startPendingOperation();
        } else {
            // ... But there are special cases when we might have to wait for the firmware update to become available.
            connect(&m_backend, &ApplicationBackend::firmwareUpdateStateChanged, this, &Cli::onUpdateStateChanged);
        }

    } else if(state == ApplicationBackend::BackendState::Finished) {
        m_backend.finalizeOperation();
    }
}

void Cli::onUpdateStateChanged()
{
    if(m_backend.firmwareUpdateState() == ApplicationBackend::FirmwareUpdateState::ErrorOccured) {
        qCCritical(LOG_CLI) << "获取固件更新失败。正在退出。";
        return exit(-1);
    }

    const auto isFirmwareReady = m_backend.firmwareUpdateState() != ApplicationBackend::FirmwareUpdateState::Checking &&
                                     m_backend.firmwareUpdateState() != ApplicationBackend::FirmwareUpdateState::Unknown;
    if(isFirmwareReady) {
        disconnect(&m_backend, &ApplicationBackend::firmwareUpdateStateChanged, this, &Cli::onUpdateStateChanged);
        startPendingOperation();
    }
}

void Cli::initConnections()
{
    connect(&m_backend, &ApplicationBackend::backendStateChanged, this, &Cli::onBackendStateChanged);
}

void Cli::initLogger()
{
    qInstallMessageHandler(Logger::messageOutput);
    globalLogger->setLogLevel(Logger::Terse);
}

void Cli::initParser()
{
    m_parser.addPositionalArgument(QStringLiteral("backup"), QStringLiteral("备份内部存储内容"), QStringLiteral("{backup <backup_file>,"));
    m_parser.addPositionalArgument(QStringLiteral("restore"), QStringLiteral("恢复内部存储内容"), QStringLiteral("restore <backup_file>,"));
    m_parser.addPositionalArgument(QStringLiteral("erase"), QStringLiteral("擦除内部存储内容"), QStringLiteral("erase,"));
    m_parser.addPositionalArgument(QStringLiteral("wipe"), QStringLiteral("擦除整个 MCU 闪存"), QStringLiteral("wipe,"));
    m_parser.addPositionalArgument(QStringLiteral("firmware"), QStringLiteral("刷写 Core1 固件"), QStringLiteral("firmware <firmware_file.dfu>,"));
    m_parser.addPositionalArgument(QStringLiteral("core2radio"), QStringLiteral("刷写 Core2 无线协议栈"), QStringLiteral("core2radio <firmware_file.bin>,"));
    m_parser.addPositionalArgument(QStringLiteral("core2fus"), QStringLiteral("刷写 Core2 固件升级服务"), QStringLiteral("core2fus <firmware_file.bin> <target_address>}"));

    m_options.append(QCommandLineOption({QStringLiteral("d"), QStringLiteral("debug-level")}, QStringLiteral("0 - 仅错误, 1 - 简洁, 2 - 完整"), QStringLiteral("1")));
    m_options.append(QCommandLineOption({QStringLiteral("n"), QStringLiteral("repeat-number")}, QStringLiteral("重复操作的次数, 0 - 无限循环"), QStringLiteral("1")));
    m_options.append(QCommandLineOption({QStringLiteral("c"), QStringLiteral("update-channel")}, QStringLiteral("固件更新/修复的更新通道"), globalPrefs->firmwareUpdateChannel()));

    m_parser.setApplicationDescription(QStringLiteral("qFlipper 的文本模式非交互式版本。不带参数运行可快速执行固件更新/修复。"));

    m_parser.addOptions(m_options);
    m_parser.addVersionOption();
    m_parser.addHelpOption();

    m_parser.process(*this);
}

void Cli::processOptions()
{
    processDebugLevelOption();
    processRepeatNumberOption();
    processUpdateChannelOption();
}

void Cli::processArguments()
{
    const auto args = m_parser.positionalArguments();

    if(args.isEmpty()) {
        beginDefaultAction();
    } else if(args.startsWith(QStringLiteral("backup"))) {
        beginBackup();
    } else if(args.startsWith(QStringLiteral("restore"))) {
        beginRestore();
    } else if(args.startsWith(QStringLiteral("erase"))) {
        beginErase();
    } else if(args.startsWith(QStringLiteral("wipe"))) {
        beginWipe();
    } else if(args.startsWith(QStringLiteral("firmware"))) {
        beginFirmware();
    } else if(args.startsWith(QStringLiteral("core2radio"))) {
        beginCore2Radio();
    } else if(args.startsWith(QStringLiteral("core2fus"))) {
        beginCore2FUS();
    } else {
        m_parser.showHelp(-1);
    }
}

void Cli::processDebugLevelOption()
{
    const auto &debugLevelOption = m_options[DebugLevelOption];

    if(!m_parser.isSet(debugLevelOption)) {
        return;
    }

    bool canConvert;
    const auto num = m_parser.value(debugLevelOption).toInt(&canConvert);

    if(!canConvert || (num < 0 || num > 2)) {
        qCCritical(LOG_CLI) << "调试级别必须是以下值之一: 0, 1, 2。";
        std::exit(-1);
    }

    globalLogger->setLogLevel((Logger::LogLevel)num);
}

void Cli::processRepeatNumberOption()
{
    const auto &repeatNumberOption = m_options[RepeatNumberOption];

    if(!m_parser.isSet(repeatNumberOption)) {
        return;
    }

    bool canConvert;
    const auto num = m_parser.value(repeatNumberOption).toInt(&canConvert);

    if(!canConvert || (num < 0)) {
        qCCritical(LOG_CLI) << "重复次数必须是非负整数。";
        std::exit(-1);
    }

    qCInfo(LOG_CLI).noquote() << "将重复执行操作" << (num ? QStringLiteral("%1 次。").arg(num) : QStringLiteral("无限循环。"));

    m_repeatCount = num ? num : -1;
}

void Cli::processUpdateChannelOption()
{
    const auto &updateChannelOption = m_options[UpdateChannelOption];

    if(!m_parser.isSet(updateChannelOption)) {
        return;
    }

    static const QStringList allowedChannelNames = {
        QStringLiteral("release"),
        QStringLiteral("release-candidate"),
        QStringLiteral("development")
    };

    const auto channelName = m_parser.value(updateChannelOption);

    if(!allowedChannelNames.contains(channelName)) {
        qCCritical(LOG_CLI) << "未知的更新通道。可用的通道为:" << allowedChannelNames;
        std::exit(-1);
    }

    globalPrefs->setFirmwareUpdateChannel(channelName);
}

void Cli::beginDefaultAction()
{
    qCInfo(LOG_CLI) << "正在执行完整固件更新...";
    m_pendingOperation = DefaultAction;
}

void Cli::beginBackup()
{
    verifyArgumentCount(2);
    m_fileParameter = QUrl::fromLocalFile(m_parser.positionalArguments().at(1));

    qCInfo(LOG_CLI).noquote().nospace() << "正在备份内部存储到 " << m_fileParameter.toLocalFile() << "...";
    m_pendingOperation = Backup;
}

void Cli::beginRestore()
{
    verifyArgumentCount(2);
    m_fileParameter = QUrl::fromLocalFile(m_parser.positionalArguments().at(1));

    qCInfo(LOG_CLI).noquote().nospace() << "正在从备份恢复内部存储 " << m_fileParameter.toLocalFile() << "...";
    m_pendingOperation = Restore;
}

void Cli::beginErase()
{
    verifyArgumentCount(1);
    qCInfo(LOG_CLI) << "正在执行设备恢复出厂设置...";
    m_pendingOperation = Erase;
}

void Cli::beginWipe()
{
    qCCritical(LOG_CLI) << "擦除功能尚未实现。抱歉！";
    std::exit(-1);
}

void Cli::beginFirmware()
{
    verifyArgumentCount(2);
    const auto arg = m_parser.positionalArguments().at(1);

    if(!arg.endsWith(QStringLiteral(".dfu"), Qt::CaseInsensitive)) {
        qCCritical(LOG_CLI) << "请提供 DFUse 格式的固件文件。";
        std::exit(-1);
    }

    m_fileParameter = QUrl::fromLocalFile(arg);

    qCInfo(LOG_CLI).noquote().nospace() << "正在从以下位置安装固件 " << m_fileParameter.toLocalFile() << "...";
    m_pendingOperation = Firmware;
}

void Cli::beginCore2Radio()
{
    verifyArgumentCount(2);
    const auto arg = m_parser.positionalArguments().at(1);

    if(!arg.endsWith(QStringLiteral(".bin"), Qt::CaseInsensitive)) {
        qCCritical(LOG_CLI) << "请提供 .bin 格式的固件文件。";
        std::exit(-1);
    }

    m_fileParameter = QUrl::fromLocalFile(arg);

    qCInfo(LOG_CLI).noquote().nospace() << "正在从以下位置安装无线固件 " << m_fileParameter.toLocalFile() << "...";
    m_pendingOperation = Core2Radio;
}

void Cli::beginCore2FUS()
{
    verifyArgumentCount(3);

    const auto args = m_parser.positionalArguments();
    const auto &arg1 = args[1];
    const auto &arg2 = args[2];

    if(!arg1.endsWith(QStringLiteral(".bin"), Qt::CaseInsensitive)) {
        qCCritical(LOG_CLI) << "请提供 .bin 格式的固件文件。";
        std::exit(-1);
    }

    m_fileParameter = QUrl::fromLocalFile(arg1);

    bool canConvert;
    m_core2Address = arg2.toULong(&canConvert, 16);

    if(!canConvert) {
        qCCritical(LOG_CLI) << "请提供有效的十六进制地址。";
        std::exit(-1);
    }

    m_pendingOperation = Core2FUS;
}

void Cli::startPendingOperation()
{
    if(m_repeatCount == 0) {
        qCInfo(LOG_CLI) << "全部完成！谢谢。";
        return exit(0);

    } else if(m_repeatCount > 0) {
        --m_repeatCount;
    }

    if(m_pendingOperation == DefaultAction) {
        m_backend.mainAction();
    } else if(m_pendingOperation == Backup) {
        m_backend.createBackup(m_fileParameter);
    } else if(m_pendingOperation == Restore) {
        m_backend.restoreBackup(m_fileParameter);
    } else if(m_pendingOperation == Erase) {
        m_backend.factoryReset();
    } else if(m_pendingOperation == Wipe) {
        // Not implemented yet
    } else if(m_pendingOperation == Firmware) {
        m_backend.installFirmware(m_fileParameter);
    } else if(m_pendingOperation == Core2Radio) {
        m_backend.installWirelessStack(m_fileParameter);
    } else if(m_pendingOperation == Core2FUS) {
        m_backend.installFUS(m_fileParameter, m_core2Address);
    } else {
        qCCritical(LOG_CLI) << "未处理的操作。可能是个 Bug！";
        exit(-1);
    }
}

void Cli::verifyArgumentCount(int num)
{
    const auto argCount = m_parser.positionalArguments().size();

    if(argCount != num) {
        qCCritical(LOG_CLI).nospace() << "期望 " << num << " 个参数，实际得到 " << argCount << " 个。正在退出。";
        std::exit(-1);
    }
}
