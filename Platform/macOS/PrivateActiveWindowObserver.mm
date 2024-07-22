#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include "PrivateActiveWindowObserver.hpp"

static void GetDataFromUIElement(AXUIElementRef element, CGRect *outRect,
    pid_t *outPid)
{
    pid_t pid;
    CGRect rect;
    {
        CGPoint point;
        CGSize size;
        AXError err;
        BOOL success;
        AXValueRef ret;
        err = AXUIElementGetPid(element, &pid);
        if (err != kAXErrorSuccess) return;
        err = AXUIElementCopyAttributeValue(element, kAXPositionAttribute, (CFTypeRef *)&ret);
        if (err != kAXErrorSuccess) return;
        success = AXValueGetValue(ret, (AXValueType)kAXValueCGPointType, &point);
        if (!success) return;
        err = AXUIElementCopyAttributeValue(element, kAXSizeAttribute, (CFTypeRef *)&ret);
        if (err != kAXErrorSuccess) return;
        success = AXValueGetValue(ret, (AXValueType)kAXValueCGSizeType, &size);
        if (!success) return;
        rect = { .size = size, .origin = point };
    }
    if (outRect != NULL) *outRect = rect;
    if (outPid != NULL) *outPid = pid;
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
    GetDataFromUIElement(focusedWindowRef, &rect, &pid);
    m_activePid = pid;
    return m_activeWindow = { rect.origin.x, rect.origin.y, rect.size.width,
        rect.size.height };
}

}