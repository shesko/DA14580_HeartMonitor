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

#import <Cordova/CDVPlugin.h>
#import <Cordova/CDVJSON.h>

@interface BCUtility : CDVPlugin

@property (strong, nonatomic) NSMutableDictionary *callbackAndUrl;

- (void)redirectToApp:(CDVInvokedUrlCommand*)command;
- (void)retry:(CDVInvokedUrlCommand*)command;
- (void)openApp:(CDVInvokedUrlCommand*)command;
-(void)openAllApp:(CDVInvokedUrlCommand*)command;
- (void)exitApp:(CDVInvokedUrlCommand*)command;
@end
