//
//  main.m
//  Scary DMX
//
//  Created by Jason DiPrinzio on 1/24/09.
//  Copyright Inspirotech Inc 2009. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#ifndef _CLI_MAIN
int main(int argc, char *argv[])
{
    //NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = NSApplicationMain(argc,  (const char **) argv);
    //[pool release];
    return retVal;
}
#endif
