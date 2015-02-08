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

#import "BCWebViewController.h"

@interface BCWebViewController ()
{
    BOOL errorLoad;
}
@end

@implementation BCWebViewController

- (void)webViewDidFinishLoad:(UIWebView*)theWebView
{
    [super webViewDidFinishLoad:theWebView];
    theWebView.scrollView.bounces = NO;
    [self jsExecute:@"www/cordova.js"];
}

- (void)jsExecute:(NSString *)jsFilePath
{
    NSString *filePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:jsFilePath];
    NSString *cordovaString = [[NSString alloc] initWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
    [self.webView stringByEvaluatingJavaScriptFromString:cordovaString];
}

@end
