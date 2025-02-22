// 
// Shijima-Qt - Cross-platform shimeji simulation app for desktop
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include "PrivateActiveWindowObserver.hpp"

// Private API
extern "C" AXError _AXUIElementGetWindow(AXUIElementRef, CGWindowID *out);

static BOOL GetDataFromUIElement(AXUIElementRef element, CGRect *outRect,
    pid_t *outPid, CGWindowID *outWindowID)
{
    AXError err;
    BOOL success;
    if (outPid != NULL) {
        err = AXUIElementGetPid(element, outPid);
        if (err != kAXErrorSuccess) return NO;
    }
    if (outRect != NULL) {
        AXValueRef ret;
        err = AXUIElementCopyAttributeValue(element, kAXPositionAttribute,
            (CFTypeRef *)&ret);
        if (err != kAXErrorSuccess) return NO;
        success = AXValueGetValue(ret, (AXValueType)kAXValueCGPointType,
            &outRect->origin);
        if (!success) return NO;
        err = AXUIElementCopyAttributeValue(element, kAXSizeAttribute,
            (CFTypeRef *)&ret);
        if (err != kAXErrorSuccess) return NO;
        success = AXValueGetValue(ret, (AXValueType)kAXValueCGSizeType,
            &outRect->size);
        if (!success) return NO;
    }
    if (outWindowID != NULL) {
        err = _AXUIElementGetWindow(element, outWindowID);
        if (err != kAXErrorSuccess) return NO;
    }
    return YES;
}

namespace Platform {

PrivateActiveWindowObserver::PrivateActiveWindowObserver() {
    AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)@{
        (__bridge NSString *)kAXTrustedCheckOptionPrompt: @YES
    });
}

ActiveWindow PrivateActiveWindowObserver::getActiveWindow() {
    NSRunningApplication *frontmost = [[NSWorkspace sharedWorkspace] frontmostApplication];
    if (frontmost.processIdentifier == getpid()) {
        // Use the application that was previously active
        if (m_activePid == -1) {
            return m_activeWindow = {};
        }
        frontmost = [NSRunningApplication
            runningApplicationWithProcessIdentifier:m_activePid];
        if (frontmost == nil) {
            return m_activeWindow = {};
        }
    }
    AXUIElementRef appRef = AXUIElementCreateApplication(frontmost.processIdentifier);
    if (appRef == NULL) {
        return m_activeWindow = {};
    }
    AXUIElementRef focusedWindowRef;
    AXError result = AXUIElementCopyAttributeValue(appRef, kAXFocusedWindowAttribute,
        (CFTypeRef*)&focusedWindowRef);
    if (result != kAXErrorSuccess) {
        return m_activeWindow = {};
    }
    CGRect rect;
    pid_t pid;
    CGWindowID windowID;
    BOOL gotData = GetDataFromUIElement(focusedWindowRef, &rect, &pid, &windowID);
    if (!gotData) {
        //NSLog(@"Failed to get window data");
        return m_activeWindow = {};
    }
    m_activePid = pid;
    QString uid = QString::fromStdString(std::to_string(pid) + 
        "-" + std::to_string(windowID));
    return m_activeWindow = { uid, (long)pid, rect.origin.x,
        rect.origin.y, rect.size.width, rect.size.height };
}

}