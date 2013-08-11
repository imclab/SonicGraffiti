//
//  MyGuiViewController.m
//  iPhoneGuiExample
//
//  Created by Parag Mital on 3/20/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "MyGuiViewController.h"
#include "ofxiPhoneExtras.h"

@implementation MyGuiViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    app = (testApp *)ofGetAppPtr();
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (IBAction)didSelectRecord:(id)sender {
    app->selectedRecord();
}

- (IBAction)didSelectCamera:(id)sender {
    app->selectedCamera();
}

- (IBAction)didSelectPageCurl:(id)sender {
    app->selectedPageCurl();
}

@end
