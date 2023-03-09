#include "screens.h"

#include <AppKit/Appkit.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/graphics/IOGraphicsLib.h>

#include <QApplication>
#include <QWidget>
#include <QWindow>
#include <private/qedidparser_p.h>

using kvmap = std::map<QString, QString>;

static kvmap edidForDisplay(CGDirectDisplayID displayID)
{
    const auto vendor = CGDisplayVendorNumber(displayID);

    const auto model = CGDisplayModelNumber(displayID);

    const auto serialNumber = CGDisplaySerialNumber(displayID);

    io_iterator_t iterator{};
    if (IOServiceGetMatchingServices(kIOMasterPortDefault,
        IOServiceMatching("IODisplayConnect"), &iterator) != KERN_SUCCESS)
        return {};

    io_service_t display{};
    while ((display = IOIteratorNext(iterator)) != 0)
    {
        const NSDictionary *info = [(__bridge NSDictionary*)IODisplayCreateInfoDictionary(
            display, kIODisplayOnlyPreferredName) autorelease];

        if ([[info objectForKey:@kDisplayVendorID] unsignedIntValue] != vendor
            || [[info objectForKey:@kDisplayProductID] unsignedIntValue] != model
            || [[info objectForKey:@kDisplaySerialNumber] unsignedIntValue] != serialNumber)
            continue;

        const QByteArray edidData = QByteArray::fromCFData((__bridge CFDataRef)[info objectForKey:@kIODisplayEDIDKey]);

        QEdidParser edid;

        if (!edid.parse(edidData))
            break;

        return {
            {QString("name"), edid.identifier},
            {QString("manufacturer"), edid.manufacturer},
            {QString("model"), edid.model},
            {QString("serialNumber"), edid.serialNumber}
        };
    }

    return {};
}

static NSNumber *screenId()
{
    const NSScreen *screen = [&]() {
        const QWidget *wdg = QApplication::activeWindow();

        if (!wdg)
            return [NSScreen mainScreen];

        const NSView *view = (NSView*)wdg->windowHandle()->winId();

        if (!view)
            return [NSScreen mainScreen];

        const NSWindow *window = [view window];

        if (!window)
            return [NSScreen mainScreen];

        return [window screen];
    }();

    return (NSNumber *)[[screen deviceDescription] valueForKey:@"NSScreenNumber"];
}

QString CrossPlatformScreen::name() const
{
    const NSNumber *id = screenId();

    if (id != nullptr) {
        const auto data = edidForDisplay([id unsignedIntValue]);

        const auto x = data.find("name");

        if (x != data.end() && !x->second.isEmpty()) {
            return x->second;
        }
    }

    return this->screen->name();
}

QString CrossPlatformScreen::manufacturer() const
{
    const NSNumber *id = screenId();

    if (id != nullptr) {
        const auto data = edidForDisplay([id unsignedIntValue]);

        const auto x = data.find("manufacturer");

        if (x != data.end() && !x->second.isEmpty()) {
            return x->second;
        }
    }

    return this->screen->manufacturer();
}

QString CrossPlatformScreen::model() const
{
   const NSNumber *id = screenId();

    if (id != nullptr) {
        const auto data = edidForDisplay([id unsignedIntValue]);

        const auto x = data.find("model");

        if (x != data.end() && !x->second.isEmpty()) {
            return x->second;
        }
    }

    return this->screen->model();
}

QString CrossPlatformScreen::serialNumber() const
{
    const NSNumber *id = screenId();

    if (id != nullptr) {
        const auto data = edidForDisplay([id unsignedIntValue]);

        const auto x = data.find("serialNumber");

        if (x != data.end() && !x->second.isEmpty()) {
            return x->second;
        }
    }

    return this->screen->serialNumber();
}
