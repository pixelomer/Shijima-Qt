#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include "PrivateActiveWindowObserver.hpp"

@interface SHJActiveWindowObserver : NSObject
@end

@implementation SHJActiveWindowObserver

- (instancetype)init {
    if ((self = [super init])) {
        NSWorkspace *sharedWorkspace = [NSWorkspace sharedWorkspace];
        [sharedWorkspace addObserver:self
            forKeyPath:@"frontmostApplication"
            options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld
            context:nil];   
    }
    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object 
    change:(NSDictionary<NSKeyValueChangeKey, id> *)change 
    context:(void *)context
{
    NSLog(@"%@", change);
}

@end

namespace Platform {

PrivateActiveWindowObserver::PrivateActiveWindowObserver() {
    m_observer = [SHJActiveWindowObserver new];
}

ActiveWindow PrivateActiveWindowObserver::getActiveWindow() {
    return {};
}

}