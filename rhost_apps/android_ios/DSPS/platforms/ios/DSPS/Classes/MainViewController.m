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

#import "MainViewController.h"
#import "AppDelegate.h"

#define URL_HOMEPAGE                    @"assets/index.html"
@interface MainViewController ()

@end

@implementation MainViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(exitApplication) name:@"EXIT_APP" object:nil];
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    if ([[UIDevice currentDevice] userInterfaceIdiom] != UIUserInterfaceIdiomPad)
    {
        self.view.frame=CGRectMake(0, 0, 320, 578);
    }
    self.edgesForExtendedLayout = UIRectEdgeNone;
    homePage = [[BCWebViewController alloc] init];
    homePage.startPage = URL_HOMEPAGE;
    homePage.view.hidden = NO;
    [self.view addSubview:homePage.view];
}

- (void)exitApplication
{    
    AppDelegate *app = [UIApplication sharedApplication].delegate;
    UIWindow *window = app.window;
    
    [UIView animateWithDuration:1.0f animations:^{
        window.alpha = 0;
        window.frame = CGRectMake(0, window.bounds.size.width, 0, 0);
    } completion:^(BOOL finished) {
        exit(0);
    }];
}

@end
