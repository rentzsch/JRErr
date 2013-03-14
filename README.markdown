# JRErr

JRErr is a small (two-file) source library that eases correct use of `NSError`.

While NSError by itself is a fine class, there's lots of reasons to hate Apple's standard NSError implementation pattern:

* Every method needs a local `NSError*` to pass to other methods. Let's call this duplicative variable `localError`.

* Because you [can't examine `localError` directly to detect errors](http://rentzsch.tumblr.com/post/260201639/nserror-is-hard), you'll need another, related variable. Let's call that duplicative related variable `hasError`.

* Repetitively testing `hasError` and indenting all your do-real-work code inside a series of `if` statements is mucho lame-o. Dude, there's these things called exceptions...

* It takes eight lines of boilerplate to return your method's result and correctly return an encountered error or log it. That's seven lines too many.

## Example

JRErr reduces these 84 lines of code:

    - (BOOL)incrementBuildNumberInFile:(NSURL*)fileURL
                                 error:(NSError**)error
    {
        NSParameterAssert(fileURL);
        
        static NSString *const sErrorDescription = @"Unrecognized File Format";
        static NSString *const sBuildNumberKey = @"BuildNumber";
        
        BOOL hasError = NO;
        NSError *localError = nil;
        
        NSData *fileData = [NSData dataWithContentsOfURL:fileURL
                                                 options:0
                                                   error:&localError];
        if (!fileData) {
            hasError = YES;
        }
        
        NSMutableDictionary *fileDict = nil;
        if (!hasError) {
            fileDict = [NSPropertyListSerialization propertyListWithData:fileData
                                                                 options:NSPropertyListMutableContainers
                                                                  format:NULL
                                                                   error:&localError];
            if (!fileDict) {
                hasError = YES;
            }
        }
        
        NSNumber *buildNumber = nil;
        if (!hasError) {
            buildNumber = [fileDict objectForKey:sBuildNumberKey];
            
            if (buildNumber) {
                if (![buildNumber isKindOfClass:[NSNumber class]]) {
                    NSString *errReason = @"BuildNumber isn't a Number";
                    localError = [NSError errorWithDomain:@"MyClass"
                                                     code:-1
                                                 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                                           sErrorDescription, NSLocalizedDescriptionKey,
                                                           errReason, NSLocalizedFailureReasonErrorKey,
                                                           nil]];
                    hasError = YES;
                }
            } else {
                NSString *errReason = @"BuildNumber is missing";
                localError = [NSError errorWithDomain:@"MyClass"
                                                 code:-1
                                             userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                                       sErrorDescription, NSLocalizedDescriptionKey,
                                                       errReason, NSLocalizedFailureReasonErrorKey,
                                                       nil]];
                hasError = YES;
            }
        }
        
        if (!hasError) {
            buildNumber = [NSNumber numberWithInt:[buildNumber intValue] + 1];
            [fileDict setObject:buildNumber forKey:sBuildNumberKey];
            fileData = [NSPropertyListSerialization dataWithPropertyList:fileDict
                                                                  format:NSPropertyListXMLFormat_v1_0
                                                                 options:0
                                                                   error:&localError];
            if (!fileData) {
                hasError = YES;
            }
        }
        
        if (!hasError) {
            if (![fileData writeToURL:fileURL options:NSDataWritingAtomic error:&localError]) {
                hasError = YES;
            }
        }
        
        if (hasError) {
            if (error) {
                *error = localError;
            } else {
                NSLog(@"error: %@", localError);
            }
        }
        return hasError;
    }

…to these 34 lines of code:

    - (BOOL)incrementBuildNumberInFile:(NSURL*)fileURL
                                 error:(NSError**)error
    {
        NSParameterAssert(fileURL);
        
        static NSString *const sErrorDescription = @"Unrecognized File Format";
        static NSString *const sBuildNumberKey = @"BuildNumber";
        
        @try {
            NSData *fileData = JRThrowErr([NSData dataWithContentsOfURL:fileURL
                                                                options:0
                                                                  error:jrErrRef]);
            NSMutableDictionary *fileDict = JRThrowErr([NSPropertyListSerialization propertyListWithData:fileData
                                                                                                 options:NSPropertyListMutableContainers
                                                                                                  format:NULL
                                                                                                   error:jrErrRef]);
            NSNumber *buildNumber = [fileDict objectForKey:sBuildNumberKey];
            if (buildNumber) {
                if (![buildNumber isKindOfClass:[NSNumber class]]) {
                    JRThrowErrMsg(sErrorDescription, @"BuildNumber isn't a Number");
                }
            } else {
                JRThrowErrMsg(sErrorDescription, @"BuildNumber is missing");
            }
            
            buildNumber = [NSNumber numberWithInt:[buildNumber intValue] + 1];
            [fileDict setObject:buildNumber forKey:sBuildNumberKey];
            fileData = JRThrowErr([NSPropertyListSerialization dataWithPropertyList:fileDict
                                                                             format:NSPropertyListXMLFormat_v1_0
                                                                            options:0
                                                                              error:jrErrRef]);
            JRThrowErr([fileData writeToURL:fileURL options:NSDataWritingAtomic error:jrErrRef]);
        } @catch (JRErrException *x) {}
        
        returnJRErr();
    }

…and fortifies its generated NSErrors with extra error-origination information (`__FILE__`, `__LINE__`, `__PRETTY__FUNCTION__`, the code within JRThrowErr()'s argument in string form and even the stack trace).

## Theory of Operation

JRErr maintains a thread-local object (`JRErrContext`) that maintains a stack of `NSError`s.

Errors are temporarily pushed onto the thread's error stack as they are encountered (via `JRPushErr()` or `JRThrowErr()`). Errors should only exist on the stack for short periods of time: usually just the span of a single method.

`returnJRErr()` is called at the end of the method. It's responsible for populating the method's `error` argument based on the error stack and returning the method's value. It also logs any errors if the error argument is NULL.

`returnJRErr()` comes in three variants based on the number of arguments provided:

* 0 arguments: assumes the method's signature returns `BOOL`. If no errors are on the stack, returns YES. Otherwise returns NO.

* 1 arguments: assumes the method's signature returns a pointer. If no errors are on the stack, returns its argument. Otherwise returns nil.

* 2 arguments: offers complete control of the method's return value. If no errors are on the stack, returns its first argument. Otherwise returns its second argument.

## Version History

### v1.0.0: Mar 14 2013

* First stable release. Work begins on v2.x...