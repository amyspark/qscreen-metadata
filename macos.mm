#include "screens.h"
#include <AppKit/Appkit.h>
#include <CoreGraphics/CoreGraphics.h>

#include <iostream>

QString CrossPlatformScreen::name() const
{
    // Using the NSScreen localizedName here would return a localized name
    // Unsure why Qt returns the correct device name, but alas
    return this->screen->name();
}

QString CrossPlatformScreen::manufacturer() const
{
    NSScreen *screen = [NSScreen mainScreen];
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

    return this->screen->serialNumber();
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
