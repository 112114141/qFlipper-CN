#include "regionprovisioningoperation.h"

#include <QDebug>
#include <QLocale>
#include <QLoggingCategory>

#include "regioninfo.h"
#include "tempdirectories.h"
#include "remotefilefetcher.h"
#include "protobufplugininterface.h"

#include "flipperzero/devicestate.h"

#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/storagewriteoperation.h"

Q_DECLARE_LOGGING_CATEGORY(CATEGORY_DEBUG)

using namespace Flipper;
using namespace Zero;

RegionProvisioningOperation::RegionProvisioningOperation(ProtobufSession *rpc, DeviceState *_deviceState, QObject *parent):
    AbstractUtilityOperation(rpc, _deviceState, parent),
    m_regionInfoFile(globalTempDirs->createTempFile(this)),
    m_regionDataFile(globalTempDirs->createTempFile(this))
{
    connect(this, &AbstractOperation::started, this, [=]() {
        deviceState()->setStatusString(QStringLiteral("正在设置区域数据..."));
    });
}

const QString RegionProvisioningOperation::description() const
{
    return QStringLiteral("区域配置 @%1").arg(deviceState()->deviceInfo().name);
}

const QByteArray RegionProvisioningOperation::localeCountry()
{
    const auto localeName = QLocale::system().name();
    return localeName.split('_').value(1).toLocal8Bit();
}

void RegionProvisioningOperation::nextStateLogic()
{
    if(operationState() == Ready) {
        setOperationState(CheckingHardwareRegion);
        checkHardwareRegion();

    } else if(operationState() == CheckingHardwareRegion) {
        setOperationState(FetchingRegionInfo);
        fetchRegionInfo();

    } else if(operationState() == FetchingRegionInfo) {
        setOperationState(GeneratingRegionData);
        generateRegionData();

    } else if(operationState() == GeneratingRegionData) {
        setOperationState(UploadingRegionData);
        uploadRegionData();

    } else if(operationState() == UploadingRegionData) {
        finish();
    }
}

void RegionProvisioningOperation::checkHardwareRegion()
{
    const auto &hardwareInfo = deviceState()->deviceInfo().hardware;

    if(hardwareInfo.region == Region::Dev) {
        qCDebug(CATEGORY_DEBUG) << "检测到开发硬件区域，跳过区域配置...";
        setOperationState(UploadingRegionData);
    }

    advanceOperationState();
}

void RegionProvisioningOperation::fetchRegionInfo()
{
    static const auto apiUrl = QStringLiteral("https://update.flipperzero.one/regions/api/v0/bundle");
    auto *fetcher = new RemoteFileFetcher(apiUrl, m_regionInfoFile, this);

    if(fetcher->isError()) {
        finishWithError(fetcher->error(), QStringLiteral("获取区域信息文件失败: %1").arg(fetcher->errorString()));
        return;
    }

    connect(fetcher, &RemoteFileFetcher::finished, this, [=]() {
        if(fetcher->isError()) {
            finishWithError(fetcher->error(), QStringLiteral("获取区域信息文件失败: %1").arg(fetcher->errorString()));
        } else {
            advanceOperationState();
        }
    });
}

void RegionProvisioningOperation::generateRegionData()
{
    if(!m_regionInfoFile->open(QIODevice::ReadOnly)) {
        finishWithError(BackendError::DiskError, m_regionInfoFile->errorString());
        return;
    }

    const RegionInfo regionInfo(m_regionInfoFile->readAll());
    m_regionInfoFile->close();

    if(!regionInfo.isValid()) {
        finishWithError(BackendError::DataError, QStringLiteral("服务器返回了无效数据"));
        return;

    } else if(regionInfo.isError()) {
        finishWithError(BackendError::DataError, regionInfo.errorString());
        return;
    }

    const auto countryCode = regionInfo.hasCountryCode() ? regionInfo.detectedCountry() : localeCountry();
    const auto bandKeys = regionInfo.countryBandKeys(countryCode);

    qCDebug(CATEGORY_DEBUG) << "检测到的区域:" << (countryCode.isEmpty() ? QByteArrayLiteral("未知") : countryCode);
    qCDebug(CATEGORY_DEBUG) << "允许的频段:" << bandKeys;

    BandInfoList bands;

    // Convert RegionInfo::BandList to BandInfoList. Probably pointless, used only for decoupling reasons.
    const auto allowedBands = regionInfo.bandsByKeys(bandKeys);
    for(const auto &band : allowedBands) {
        bands.append({
            band.start,
            band.end,
            band.powerLimit,
            band.dutyCycle
        });
    }

    if(!m_regionDataFile->open(QIODevice::WriteOnly)) {
        finishWithError(BackendError::DiskError, m_regionDataFile->errorString());
        return;
    }

    const auto regionData = rpc()->pluginInstance()->regionBands(countryCode, bands);

    if(regionData.isEmpty()) {
        finishWithError(BackendError::UnknownError, QStringLiteral("编码区域数据失败"));
        return;
    }

    const auto bytesWritten = m_regionDataFile->write(regionData);
    m_regionDataFile->close();

    if((bytesWritten <= 0) || (bytesWritten != regionData.size())) {
        finishWithError(BackendError::DiskError, QStringLiteral("将区域数据保存到临时文件失败"));
    } else {
        advanceOperationState();
    }
}

void RegionProvisioningOperation::uploadRegionData()
{
    auto *operation = rpc()->storageWrite("/int/.region_data", m_regionDataFile);

    connect(operation, &AbstractOperation::finished, this, [=]() {
        if(operation->isError()) {
            finishWithError(operation->error(), operation->errorString());
        } else {
            advanceOperationState();
        }
    });
}
