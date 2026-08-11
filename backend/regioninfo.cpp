#include "regioninfo.h"

#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(CATEGORY_DEBUG)

RegionInfo::RegionInfo():
    m_isValid(false)
{}

RegionInfo::RegionInfo(const QByteArray &text)
{
    const auto doc = QJsonDocument::fromJson(text);

    if(doc.isNull()) {
        qCCritical(CATEGORY_DEBUG) << "解析文档失败";
        return;
    } else if(!doc.isObject()) {
        qCCritical(CATEGORY_DEBUG) << "JSON 文档不是对象";
        return;
    }

    static const auto errorKey = QStringLiteral("error");
    static const auto successKey = QStringLiteral("success");

    const auto obj = doc.object();
    if(obj.contains(errorKey)) {
        parseError(obj.value(errorKey));
    } else if(obj.contains(successKey)) {
        parseSuccess(obj.value(successKey));
    } else {
        qCCritical(CATEGORY_DEBUG) << "收到有效的 JSON 对象，但既不是成功也不是错误信息";
    }
}

bool RegionInfo::isValid() const
{
    return m_isValid;
}

bool RegionInfo::isError() const
{
    return m_isError;
}

bool RegionInfo::hasCountryCode() const
{
    return !m_country.isEmpty();
}

const QString &RegionInfo::errorString() const
{
    return m_errorString;
}

int RegionInfo::errorCode() const
{
    return m_errorCode;
}

const RegionInfo::CountryKey &RegionInfo::detectedCountry() const
{
    return m_country;
}

const RegionInfo::BandKeyList &RegionInfo::defaultBandKeys() const
{
    return m_defaultBandKeys;
}

const RegionInfo::BandKeyList RegionInfo::countryBandKeys(const CountryKey &key) const
{
    return m_countries.contains(key) ? m_countries[key] : m_defaultBandKeys;
}

const RegionInfo::BandList RegionInfo::bandsByKeys(const BandKeyList &keys) const
{
    BandList ret;
    for(const auto &key : keys) {
        ret.append(m_bands.value(key));
    }
    return ret;
}

const RegionInfo::BandList RegionInfo::bandsByCountry(const CountryKey &key) const
{
    return bandsByKeys(countryBandKeys(key));
}

void RegionInfo::parseError(const QJsonValue &val)
{
    if(!val.isObject()) {
        qCCritical(CATEGORY_DEBUG) << "错误信息不是对象";
        return;
    }

    const auto obj = val.toObject();

    static const auto codeKey = QStringLiteral("code");
    static const auto textKey = QStringLiteral("text");

    const auto isComplete = obj.contains(codeKey) && obj.contains(textKey);
    if(!isComplete) {
        qCCritical(CATEGORY_DEBUG) << "错误对象缺少一个或多个字段";
        return;
    }

    m_errorCode = obj[codeKey].toInt();
    m_errorString = obj[textKey].toString();

    m_isValid = true;
    m_isError = true;
}

void RegionInfo::parseSuccess(const QJsonValue &val)
{
    if(!val.isObject()) {
        qCCritical(CATEGORY_DEBUG) << "成功信息不是对象";
        return;
    }

    const auto obj = val.toObject();

    const auto bandsKey = QStringLiteral("bands");
    const auto countriesKey = QStringLiteral("countries");
    const auto countryKey = QStringLiteral("country");
    const auto defaultKey = QStringLiteral("default");

    const auto isComplete = obj.contains(bandsKey) && obj.contains(countriesKey) &&
                            obj.contains(countryKey) && obj.contains(defaultKey);
    if(!isComplete) {
        qCCritical(CATEGORY_DEBUG) << "成功对象缺少一个或多个字段";
        return;
    }

    m_isError = false;
    m_isValid = parseBands(obj[bandsKey]) && parseCountries(obj[countriesKey]) &&
            parseCountry(obj[countryKey]) && parseDefault(obj[defaultKey]);
}

bool RegionInfo::parseBands(const QJsonValue &val)
{
    if(!val.isObject()) {
        qCCritical(CATEGORY_DEBUG) << "频段信息不是对象";
        return false;
    }

    const auto obj = val.toObject();

    if(obj.isEmpty()) {
        qCCritical(CATEGORY_DEBUG) << "频段信息为空对象";
        return false;
    }

    const auto keys = obj.keys();
    for(const auto &key : keys) {
        if(!insertBand(key.toLocal8Bit(), obj[key])) {
            return false;
        }
    }

    return true;
}

bool RegionInfo::parseCountries(const QJsonValue &val)
{
    if(!val.isObject()) {
        qCCritical(CATEGORY_DEBUG) << "国家信息不是对象";
        return false;
    }

    const auto obj = val.toObject();

    if(obj.isEmpty()) {
        qCCritical(CATEGORY_DEBUG) << "国家信息为空对象";
        return false;
    }

    const auto keys = obj.keys();
    for(const auto &key : keys) {
        if(!insertCountry(key.toLocal8Bit(), obj[key])) {
            return false;
        }
    }

    return true;
}

bool RegionInfo::parseCountry(const QJsonValue &val)
{
    if(!val.isString() && !val.isNull()) {
        qCCritical(CATEGORY_DEBUG) << "国家不是字符串";
        return false;

    } else {
        m_country = val.toString().toLocal8Bit();
        return true;
    }
}

bool RegionInfo::parseDefault(const QJsonValue &val)
{
    if(!val.isArray()) {
        qCCritical(CATEGORY_DEBUG) << "默认频段列表不是数组";
        return false;
    }

    const QJsonArray arr = val.toArray();

    if(arr.isEmpty()) {
        qCCritical(CATEGORY_DEBUG) << "默认频段为空数组";
        return false;
    }

    for(const auto &el : arr) {
        m_defaultBandKeys.append(el.toString().toLocal8Bit());
    }

    return true;
}

bool RegionInfo::insertBand(const BandKey &key, const QJsonValue &val)
{
    if(!val.isObject()) {
        qCCritical(CATEGORY_DEBUG) << "频段信息不是对象";
        return false;
    }

    const auto obj = val.toObject();

    const auto startKey = QStringLiteral("start");
    const auto endKey = QStringLiteral("end");
    const auto dutyCycleKey = QStringLiteral("duty_cycle");
    const auto maxPowerKey = QStringLiteral("max_power");

    const auto isComplete = obj.contains(startKey) && obj.contains(endKey) &&
                            obj.contains(dutyCycleKey) && obj.contains(maxPowerKey);

    if(!isComplete) {
        qCCritical(CATEGORY_DEBUG) << "频段对象缺少一个或多个字段";
        return false;
    }

    const Band band = {
        (unsigned)obj[startKey].toInt(),
        (unsigned)obj[endKey].toInt(),
        obj[maxPowerKey].toInt(),
        (unsigned)obj[dutyCycleKey].toInt(),
    };

    m_bands.insert(key, band);

    return true;
}

bool RegionInfo::insertCountry(const CountryKey &key, const QJsonValue &val)
{
    if(!val.isArray()) {
        qCCritical(CATEGORY_DEBUG) << "国家频段列表不是数组";
        return false;
    }

    const QJsonArray arr = val.toArray();

    if(arr.isEmpty()) {
        qCCritical(CATEGORY_DEBUG) << "国家频段为空数组";
        return false;
    }

    BandKeyList bandKeys;

    for(const auto &el : arr) {
        bandKeys.append(el.toString().toLocal8Bit());
    }

    m_countries.insert(key, bandKeys);

    return true;
}
