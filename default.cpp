#include "screens.h"
#include <AppKit/Appkit.h>
#include <CoreGraphics/CoreGraphics.h>

#include <iostream>

QString CrossPlatformScreen::name() const
{
    return this->screen->name();
}

QString CrossPlatformScreen::manufacturer() const
{
    return this->screen->manufacturer();
}

QString CrossPlatformScreen::model() const
{
    return this->screen->serialNumber();
}

QString CrossPlatformScreen::serialNumber() const
{
    return this->screen->serialNumber();
}
