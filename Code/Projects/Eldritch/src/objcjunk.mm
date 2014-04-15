#include "core.h"
#include "objcjunk.h"
#include "simplestring.h"

#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSFileManager.h>

SimpleString ObjCJunk::GetUserDirectory()
{
    NSAutoreleasePool* pPool = [[NSAutoreleasePool alloc] init];
    
    NSArray* pPaths = NSSearchPathForDirectoriesInDomains( NSApplicationSupportDirectory, NSUserDomainMask, YES );
    ASSERT( [pPaths count] > 0 );
    NSString* pPath = [pPaths objectAtIndex:0];
    const SimpleString RetVal = SimpleString( [pPath UTF8String] ) + SimpleString( "/Eldritch/" );
    
    NSString* pFullPath = [NSString stringWithUTF8String:RetVal.CStr()];
    NSFileManager* pFileManager = [[NSFileManager alloc] init];
    if( ![pFileManager fileExistsAtPath:pFullPath])
    {
        [pFileManager createDirectoryAtPath:pFullPath withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    [pPool drain];
    
    return RetVal;
}