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

#import "BCUtility.h"

#define URL                                   @"url"
#define DEVICEADDRESS                         @"deviceAddress"
#define DEVICEADDRESS_APP                     @"deviceAddressForApp"
#define GO_PAGE                               @"openNewApp"
#define CALLBACKREDICT                        @"callbackRedict"
#define REDIRECTTOAPP                         @"redirectToApp"

#define EVENT_ADDNEWAPP                       @"addAppToManage"
#define EVENT_REMOVEAPP                       @"removeApp"
#define REMOVEAPP_KEYURL                      @"removeAppURL"


#define REFRESHPAGE                           @"refreshPage"
#define GETDEVICEID                           @"getOwnDeviceAddress"
#define EVENT_NAME                            @"eventName"
#define OPENAPP                               @"openAppForAppID"


@implementation BCUtility
@synthesize callbackAndUrl;

- (void)pluginInitialize{
    [super pluginInitialize];
    callbackAndUrl = [[NSMutableDictionary alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(callbackRedict:) name:CALLBACKREDICT object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(redirectToApp:) name:REDIRECTTOAPP object:nil];
}

- (void)redirectToApp:(CDVInvokedUrlCommand*)command{
    NSMutableArray *arryConnect=[[NSMutableArray alloc] initWithArray:command.arguments];
    if (arryConnect.count > 0) {
        NSMutableDictionary *pageInfo = [arryConnect objectAtIndex:0];
        if (![pageInfo valueForKey:URL] || [[pageInfo valueForKey:URL] isEqualToString:@""])  {
            CDVPluginResult* result = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR];
            [self.commandDelegate sendPluginResult:result callbackId:command.callbackId];
        }else{
            [callbackAndUrl setValue:command.callbackId forKey:[pageInfo valueForKey:URL]];
            [[NSUserDefaults standardUserDefaults] setValue:[pageInfo valueForKey:DEVICEADDRESS] forKey:DEVICEADDRESS_APP];
            [[NSNotificationCenter defaultCenter] postNotificationName:GO_PAGE object:pageInfo];
        }
    }
}

- (void)callbackRedict:(NSNotification *)object{
    NSMutableDictionary *redirectInfo = [object object];
    CDVPluginResult* result;
    if ([[redirectInfo valueForKey:CALLBACKREDICT] boolValue]) {
        result = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK ];
    }else{
        result = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR ];
    }
    [self.commandDelegate sendPluginResult:result callbackId:[callbackAndUrl valueForKey:[redirectInfo valueForKey:URL]]];
    
}

- (void)addApp:(CDVInvokedUrlCommand *)command
{
    [self.callbackAndUrl setValue:command.callbackId forKey:EVENT_ADDNEWAPP];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(callBackNewAppMessage:) name:EVENT_ADDNEWAPP object:nil];
}

- (void)callBackNewAppMessage:(NSNotification *)object{
    NSMutableDictionary *callbackInfo = [object object];
    CDVPluginResult* result  = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:callbackInfo];
    [result setKeepCallbackAsBool:TRUE];
    [self.commandDelegate sendPluginResult:result callbackId:[self.callbackAndUrl valueForKey:EVENT_ADDNEWAPP]];
}

//-(void)addApp:(CDVInvokedUrlCommand *)command
//{
//    [self.callbackAndUrl setValue:command.callbackId forKey:EVENT_ADDNEWAPP];
//    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(callBackNewAppMessage:) name:EVENT_ADDNEWAPP object:nil];
//    
//}
- (void)removeApp:(CDVInvokedUrlCommand *)command
{
    [self.callbackAndUrl setValue:command.callbackId forKey:EVENT_REMOVEAPP];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(callBackRemoveAppMessage:) name:EVENT_REMOVEAPP object:nil];
}

- (void)callBackRemoveAppMessage:(NSNotification *)object
{
    NSMutableDictionary *callbackInfo = [object object];
    CDVPluginResult* result = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:callbackInfo];
    [result setKeepCallbackAsBool:TRUE];
    [self.commandDelegate sendPluginResult:result callbackId:[self.callbackAndUrl valueForKey:EVENT_REMOVEAPP]];
}

-(void)openApps:(CDVInvokedUrlCommand *)command
{
    NSMutableArray *arryConnect=[[NSMutableArray alloc] initWithArray:command.arguments];
    if (arryConnect.count>0) {
        for (NSDictionary *dic in arryConnect) {
            [[NSNotificationCenter defaultCenter] postNotificationName:OPENAPP object:dic];
        }
        NSMutableDictionary *pageIndex = [arryConnect objectAtIndex:0];
        [[NSUserDefaults standardUserDefaults]removeObjectForKey:DEVICEADDRESS_APP];
        [[NSNotificationCenter defaultCenter] postNotificationName:GO_PAGE object:pageIndex];
    }
}
-(void)openAllApp:(CDVInvokedUrlCommand *)command
{
    NSMutableArray *arryConnect=[[NSMutableArray alloc] initWithArray:command.arguments];
    if (arryConnect.count>0) {
        for (NSDictionary *dic in arryConnect) {
            [[NSNotificationCenter defaultCenter] postNotificationName:OPENAPP object:dic];
        }
        NSMutableDictionary *pageIndex = [arryConnect objectAtIndex:0];
        [[NSUserDefaults standardUserDefaults]removeObjectForKey:DEVICEADDRESS_APP];
        [[NSNotificationCenter defaultCenter] postNotificationName:GO_PAGE object:pageIndex];
    }
}
- (void)openApp:(CDVInvokedUrlCommand*)command
{
    NSMutableArray *arryConnect=[[NSMutableArray alloc] initWithArray:command.arguments];
    if (arryConnect.count > 0) {
        NSMutableDictionary *dic =[command.arguments objectAtIndex:0];
        [[NSNotificationCenter defaultCenter] postNotificationName:OPENAPP object:dic];
    }
}

- (void)retry:(CDVInvokedUrlCommand*)command
{
    [[NSNotificationCenter defaultCenter] postNotificationName:REFRESHPAGE object:nil];
}

- (void)exitApp:(CDVInvokedUrlCommand*)command
{
    [[NSNotificationCenter defaultCenter] postNotificationName:@"EXIT_APP" object:nil];
}

@end
