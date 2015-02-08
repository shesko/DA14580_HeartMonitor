/*
 Copyright 2013-2014 JUMA Technology
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#import "AppDelegate.h"
#import "MainViewController.h"
#import <Cordova/CDVPlugin.h>

@implementation AppDelegate
@synthesize window, viewController;

- (id)init
{
    NSHTTPCookieStorage* cookieStorage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    [cookieStorage setCookieAcceptPolicy:NSHTTPCookieAcceptPolicyAlways];

    int cacheSizeMemory = 8 * 1024 * 1024; // 8MB
    int cacheSizeDisk = 32 * 1024 * 1024; // 32MB
#if __has_feature(objc_arc)
        NSURLCache* sharedCache = [[NSURLCache alloc] initWithMemoryCapacity:cacheSizeMemory diskCapacity:cacheSizeDisk diskPath:@"nsurlcache"];
#else
        NSURLCache* sharedCache = [[[NSURLCache alloc] initWithMemoryCapacity:cacheSizeMemory diskCapacity:cacheSizeDisk diskPath:@"nsurlcache"] autorelease];
#endif
    [NSURLCache setSharedURLCache:sharedCache];

    self = [super init];
    return self;
}

#pragma mark UIApplicationDelegate implementation

/**
 * This is main kick off after the app inits, the views and Settings are setup here. (preferred - iOS4 and up)
 */
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
     self.window = [[UIWindow alloc] initWithFrame:screenBounds];
     [self getWindow];
     self.viewController = (CDVViewController*)[[MainViewController alloc] init];
     self.window.rootViewController = self.viewController;
     [self.window makeKeyAndVisible];
    return YES;
}

-(void)getWindow
{
    if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 7.0){
        self.window.clipsToBounds = YES;
        [[UIApplication sharedApplication]setStatusBarStyle:UIStatusBarStyleLightContent];
        if([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0)
        {
            self.window.frame = CGRectMake(0, 20, self.window.frame.size.width, self.window.frame.size.height);
        }
        else if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad){
            self.window.frame = CGRectMake(0, 20, self.window.frame.size.width, self.window.frame.size.height-20);
        }else{
            self.window.frame = CGRectMake(0, 20, self.window.frame.size.width, self.window.frame.size.height);
        }
    }else{
        self.window.frame = CGRectMake(0, -20, self.window.frame.size.width, self.window.frame.size.height);
        //[[UIApplication sharedApplication] setStatusBarHidden:NO];
    }
}

// this happens while we are running ( in the background, or from within our own app )
// only valid if DSPS-Info.plist specifies a protocol to handle
- (BOOL)application:(UIApplication*)application handleOpenURL:(NSURL*)url
{
    if (!url) {
        return NO;
    }
    // calls into javascript global function 'handleOpenURL'
    NSString* jsString = [NSString stringWithFormat:@"handleOpenURL(\"%@\");", url];
    [self.viewController.webView stringByEvaluatingJavaScriptFromString:jsString];

    // all plugins will get the notification, and their handlers will be called
    [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:CDVPluginHandleOpenURLNotification object:url]];
    return YES;
}

// repost the localnotification using the default NSNotificationCenter so multiple plugins may respond
- (void)            application:(UIApplication*)application
    didReceiveLocalNotification:(UILocalNotification*)notification
{
    // re-post ( broadcast )
    [[NSNotificationCenter defaultCenter] postNotificationName:CDVLocalNotification object:notification];
}

- (NSUInteger)application:(UIApplication*)application supportedInterfaceOrientationsForWindow:(UIWindow*)window
{
    // iPhone doesn't support upside down by default, while the iPad does.  Override to allow all orientations always, and let the root view controller decide what's allowed (the supported orientations mask gets intersected).
    NSUInteger supportedInterfaceOrientations = (1 << UIInterfaceOrientationPortrait) ;
    return supportedInterfaceOrientations;
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application
{
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
}

@end
