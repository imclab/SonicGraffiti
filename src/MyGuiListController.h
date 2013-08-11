//
//  MyGuiListController.h
//  MapKitExample
//
//  Created by Parag Mital on 4/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#include "testApp.h"

@interface MyGuiListController : UIViewController <UITableViewDataSource, UITableViewDelegate>
{
    testApp *app;
}
@property (retain, nonatomic) IBOutlet UITableView *table;
- (IBAction)didSelectCancel:(id)sender;
- (IBAction)didSelectEdit:(id)sender;

@end