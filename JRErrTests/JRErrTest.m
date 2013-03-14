#import "JRErrTest.h"
#import "JRErr.h"

@interface NSObject (TestJRErr)
- (BOOL)returnYesAndNoError:(NSError**)error;
- (BOOL)returnYesAndAnError:(NSError**)error;
- (BOOL)returnNoAndNoError:(NSError**)error;
- (BOOL)returnNoAndAnError:(NSError**)error;
- (void)returnVoidAndNoError:(NSError**)error;
- (void)returnVoidAndAnError:(NSError**)error;

- (id)returnPtrAndNoError:(NSError**)error;
- (id)returnPtrAndAnError:(NSError**)error;
- (id)returnNilPtrAndNoError:(NSError**)error;
- (id)returnNilPtrAndAnError:(NSError**)error;
@end

//-----------------------------------------------------------------------------------------

typedef enum {
	WeirdError_OK = 42,
	WeirdError_Error = -666
}	WeirdError;

static NSString* NSStringFromWeirdError(WeirdError weirdErr) {
    return weirdErr == WeirdError_OK ? @"WeirdError_OK" : @"WeirdError_Error";
}

static WeirdError ReturnWeirdOK() { return WeirdError_OK; }
static WeirdError ReturnWeirdError() { return WeirdError_Error; }
static BOOL WeirdErrorDecider(const char *codeResultType,
                              intptr_t codeResultValue)
{
    return codeResultValue == WeirdError_OK;
}
static void WeirdErrorAnnotator(const char *codeResultType,
                                intptr_t codeResultValue,
                                NSMutableDictionary *errorUserInfo)
{
    [errorUserInfo setObject:NSStringFromWeirdError((int)codeResultValue) forKey:@"weirdErrName"];
}

#define JRPushWeirdErr(CODE)  JRPushErrImpl(CODE, WeirdErrorDecider, WeirdErrorAnnotator, kPushJRErr)

//-----------------------------------------------------------------------------------------

#if __has_feature(objc_arc)
    #define autorelease self
#endif

@implementation JRErrTest

- (void)testEverything {
    NSObject *obj = [[[NSObject alloc] init] autorelease];
    
    assert(!jrErr);
    JRPushErr([obj returnYesAndNoError:jrErrRef]);
    assert(!jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnYesAndAnError:jrErrRef]);
    assert(!jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnNoAndNoError:jrErrRef]);
    assert(jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnNoAndAnError:jrErrRef]);
    assert(jrErr);
    assert([jrErr code] == 42);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnVoidAndNoError:jrErrRef]);
    assert(!jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnVoidAndAnError:jrErrRef]);
    assert(jrErr);
    assert([jrErr code] == 42);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnPtrAndNoError:jrErrRef]);
    assert(!jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnPtrAndAnError:jrErrRef]);
    assert(!jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnNilPtrAndNoError:jrErrRef]);
    assert(jrErr);
    assert([[jrErr domain] isEqualToString:@"JRErrDomain"]);
    assert([jrErr code] == -1);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr([obj returnNilPtrAndAnError:jrErrRef]);
    assert(jrErr);
    assert([[jrErr domain] isEqualToString:@"TestJRErrDomain"]);
    assert([jrErr code] == 42);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr(ReturnWeirdOK());
    assert(!jrErr); // because 42 isn't 0 (proof std JRPushErr isn't enough)
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushErr(ReturnWeirdError());
    assert(!jrErr); // because -666 isn't 0 (proof std JRPushErr isn't enough)
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushWeirdErr(ReturnWeirdOK());
    assert(!jrErr);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
    
    assert(!jrErr);
    JRPushWeirdErr(ReturnWeirdError());
    assert(jrErr);
    assert([[[jrErr userInfo] objectForKey:@"weirdErrName"] isEqualToString:NSStringFromWeirdError(WeirdError_Error)]);
    [[JRErrContext currentContext] popError];
    assert(!jrErr);
}

@end

//-----------------------------------------------------------------------------------------

@implementation NSObject (TestJRErr)

- (BOOL)returnYesAndNoError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    // Poison the error object so we'll crash if it's inspected
    // (errors shouldn't be looked at if the method indicates success):
    *error = (NSError*)0x1;
    return YES;
}

- (BOOL)returnYesAndAnError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    *error = [NSError errorWithDomain:@"TestJRErrDomain" code:42 userInfo:nil];
    return YES;
}

- (BOOL)returnNoAndNoError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    return NO;
}

- (BOOL)returnNoAndAnError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    *error = [NSError errorWithDomain:@"TestJRErrDomain" code:42 userInfo:nil];
    return NO;
}

- (void)returnVoidAndNoError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
}

- (void)returnVoidAndAnError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    *error = [NSError errorWithDomain:@"TestJRErrDomain" code:42 userInfo:nil];
}

- (id)returnPtrAndNoError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    *error = (NSError*)0x1;
    return self;
}

- (id)returnPtrAndAnError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    *error = [NSError errorWithDomain:@"TestJRErrDomain" code:42 userInfo:nil];
    return self;
}

- (id)returnNilPtrAndNoError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    return nil;
}

- (id)returnNilPtrAndAnError:(NSError**)error {
    NSParameterAssert(error);
    NSParameterAssert(*error == nil);
    *error = [NSError errorWithDomain:@"TestJRErrDomain" code:42 userInfo:nil];
    return nil;
}

@end
