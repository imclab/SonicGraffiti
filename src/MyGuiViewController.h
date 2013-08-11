//
//  MyGuiViewController.h
//  iPhoneGuiExample
//
//  Created by Parag Mital on 3/20/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#include "testApp.h"

@interface MyGuiViewController : UIViewController {
    testApp *app;
}
- (IBAction)didSelectRecord:(id)sender;
- (IBAction)didSelectCamera:(id)sender;
- (IBAction)didSelectPageCurl:(id)sender;

@end
