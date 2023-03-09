// Based on https://gist.github.com/texus/3212ebc1ed1502ecd265cc7cf1322b02

#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>

#include <iostream>
#include <map>
#include <memory>

#include <private/qedidparser_p.h>
#include <winerror.h>

#include "screens.h"

struct CaseInsensitiveComparator
{
    bool operator()( const QString& lhs, const QString& rhs ) const noexcept
    {
        return QString::compare(lhs, rhs, Qt::CaseInsensitive) < 0;
    }
};

using kvmap = std::map<QString, QString>;

using screens = std::map<QString, kvmap, CaseInsensitiveComparator>;

static kvmap getDeviceNamesToIdMap() noexcept
{
    kvmap namesToIdMap;

    // Query how many display paths there are
    UINT32 nrPaths = 0;
    UINT32 nrModes = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &nrPaths, &nrModes) != ERROR_SUCCESS)
        return {};

    // Retrieve the active display paths.
    // Although we don't need the modes, documentation of QueryDisplayConfig says we can't use NULL for them.
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(nrPaths);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(nrModes);
    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &nrPaths, paths.data(), &nrModes, modes.data(), NULL) != ERROR_SUCCESS)
        return {};

    // Loop over the display paths and map the device name to the unique id that we will use
    for (const auto &path: paths)
    {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName{};
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        sourceName.header.adapterId = path.sourceInfo.adapterId;
        sourceName.header.id = path.sourceInfo.id;
        if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS)
            continue;

        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName{};
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        targetName.header.adapterId = path.sourceInfo.adapterId;
        targetName.header.id = path.targetInfo.id;
        if (DisplayConfigGetDeviceInfo(&targetName.header)  != ERROR_SUCCESS)
            continue;

        namesToIdMap.emplace(QString::fromWCharArray(sourceName.viewGdiDeviceName), QString::fromWCharArray(targetName.monitorDevicePath));
    }

    return namesToIdMap;
}

static screens queryRegistryForScreen() noexcept
{
    screens map;

    static constexpr GUID GUID_DEVINTERFACE_MONITOR = { 0xe6f07b5f, 0xee97, 0x4a90, {0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7} };
    const HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_MONITOR, NULL, NULL, DIGCF_DEVICEINTERFACE);

    // Loop over the device interfaces using the SetupAPI
    DWORD monitorIndex = 0;
    SP_DEVICE_INTERFACE_DATA devInfo{};
    devInfo.cbSize = sizeof(devInfo);
    while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_MONITOR, monitorIndex, &devInfo))
    {
        ++monitorIndex;

        // Retrieve the id of the device interface
        DWORD requiredSize = 0;
        if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInfo, NULL, 0, &requiredSize, NULL) || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            continue;

        std::vector<WCHAR> devPathBuffer(requiredSize);
        auto *devPathData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(devPathBuffer.data());
        devPathData->cbSize = sizeof(std::remove_pointer_t<decltype(devPathData)>);
        SP_DEVINFO_DATA devInfoData{};
        devInfoData.cbSize = sizeof(devInfoData);
        if (!SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInfo, devPathData, requiredSize, NULL, &devInfoData))
            continue;

        // Find the EDID registry key
        std::unique_ptr<std::remove_pointer<HKEY>::type, decltype(&RegCloseKey)> hEDIDRegKey { SetupDiOpenDevRegKey(hDevInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ), &RegCloseKey };
        if (!hEDIDRegKey || (hEDIDRegKey.get() == INVALID_HANDLE_VALUE))
            continue; // Error

        // Read the EDID data from the registry
        DWORD sizeOfDataEDID = 0;
        if (RegQueryValueExW(hEDIDRegKey.get(), L"EDID", NULL, NULL, NULL, &sizeOfDataEDID) != ERROR_SUCCESS)
            continue;

        QByteArray edidData;
        edidData.resize(sizeOfDataEDID);

        if (RegQueryValueExW(hEDIDRegKey.get(), L"EDID", NULL, NULL, reinterpret_cast<unsigned char*>(edidData.data()), &sizeOfDataEDID) != ERROR_SUCCESS)
            continue;

        QEdidParser edid;

        if (!edid.parse(edidData))
            continue;

        kvmap entry = {
            {QString("name"), edid.identifier},
            {QString("manufacturer"), edid.manufacturer},
            {QString("model"), edid.model},
            {QString("serialNumber"), edid.serialNumber}
        };

        map.emplace(QString::fromWCharArray(devPathData->DevicePath), entry);
    }

    return map;
}

QString CrossPlatformScreen::name() const
{
    const auto names = getDeviceNamesToIdMap();

    const auto data = queryRegistryForScreen();

    const auto x = names.find(screen->name());

    if (x != names.end()) {
        const auto y = data.find(x->second);

        if (y != data.end()) {
            const auto prop = y->second.find("name");

            if (prop != y->second.end()) {
                if (!prop->second.isEmpty()) {
                    return prop->second;
                }
            }
        }
    }

    return this->screen->name();
}

QString CrossPlatformScreen::manufacturer() const
{
    const auto names = getDeviceNamesToIdMap();

    const auto data = queryRegistryForScreen();

    const auto x = names.find(screen->name());

    if (x != names.end()) {
        const auto y = data.find(x->second);

        if (y != data.end()) {
            const auto prop = y->second.find("manufacturer");

            if (prop != y->second.end()) {
                return prop->second;
            }
        }
    }

    return this->screen->manufacturer();
}

QString CrossPlatformScreen::model() const
{
    const auto names = getDeviceNamesToIdMap();

    const auto data = queryRegistryForScreen();

    const auto x = names.find(screen->name());

    if (x != names.end()) {
        const auto y = data.find(x->second);

        if (y != data.end()) {
            const auto prop = y->second.find("model");

            if (prop != y->second.end()) {
                return prop->second;
            }
        }
    }

    return this->screen->model();
}

QString CrossPlatformScreen::serialNumber() const
{
    const auto names = getDeviceNamesToIdMap();

    const auto data = queryRegistryForScreen();

    const auto x = names.find(screen->name());

    if (x != names.end()) {
        const auto y = data.find(x->second);

        if (y != data.end()) {
            const auto prop = y->second.find("serialNumber");

            if (prop != y->second.end()) {
                return prop->second;
            }
        }
    }

    return this->screen->serialNumber();
}
