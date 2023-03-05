#include "screens.h"
#include <AppKit/Appkit.h>
#include <CoreGraphics/CoreGraphics.h>

#include <QWidget>

static QString displayName(CGDirectDisplayID displayID)
{
    QIOType<io_iterator_t> iterator;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault,
        IOServiceMatching("IODisplayConnect"), &iterator))
        return QString();

    QIOType<io_service_t> display;
    while ((display = IOIteratorNext(iterator)) != 0)
    {
        NSDictionary *info = [(__bridge NSDictionary*)IODisplayCreateInfoDictionary(
            display, kIODisplayOnlyPreferredName) autorelease];

        if ([[info objectForKey:@kDisplayVendorID] longValue] != CGDisplayVendorNumber(displayID))
            continue;

        if ([[info objectForKey:@kDisplayProductID] longValue] != CGDisplayModelNumber(displayID))
            continue;

        if ([[info objectForKey:@kDisplaySerialNumber] longValue] != CGDisplaySerialNumber(displayID))
            continue;

        NSDictionary *localizedNames = [info objectForKey:@kDisplayProductName];
        if (![localizedNames count])
            break; // Correct screen, but no name in dictionary

        return QString::fromNSString([localizedNames objectForKey:[[localizedNames allKeys] objectAtIndex:0]]);
    }

    return QString();
}

QString CrossPlatformScreen::name() const
{
    // Using the NSScreen localizedName here would return a localized name
    // Unsure why Qt returns the correct device name, but alas
    return this->screen->name();
}

QString CrossPlatformScreen::manufacturer() const
{
    QPlatformScreen *nativeScreen = screen->handle();

    NSNumber *id = (NSNumber *)[[screen deviceDescription] valueForKey:@"NSScreenNumber"];
    if (id != nullptr) {
        return QString::number(CGDisplayVendorNumber([id unsignedIntValue]), 16);
    }

    return this->screen->manufacturer();
}

QString CrossPlatformScreen::model() const
{
    NSScreen *screen = [NSScreen mainScreen];
    NSNumber *id = (NSNumber *)[[screen deviceDescription] valueForKey:@"NSScreenNumber"];

    if (id != nullptr) {
        return QString::number(CGDisplayModelNumber([id unsignedIntValue]), 16);
    }

    return this->screen->model();
}

QString CrossPlatformScreen::serialNumber() const
{
    NSScreen *screen = [NSScreen mainScreen];
    NSNumber *id = (NSNumber *)[[screen deviceDescription] valueForKey:@"NSScreenNumber"];

    if (id != nullptr) {
        return QString::number(CGDisplaySerialNumber([id unsignedIntValue]), 16);
    }

    return this->screen->serialNumber();
}
