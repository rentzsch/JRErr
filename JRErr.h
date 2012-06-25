// JRErr.h semver:0.0.7
//   Copyright (c) 2012 Jonathan 'Wolf' Rentzsch: http://rentzsch.com
//   Some rights reserved: http://opensource.org/licenses/MIT
//   https://github.com/rentzsch/JRErr

#import <Foundation/Foundation.h>

#define jrErr [[JRErrContext currentContext] currentError]

extern NSString * const JRErrDomain;

//-----------------------------------------------------------------------------------------

#define JRPushErrImpl(CODE, __shouldThrow)                                                                          \
    ({                                                                                                              \
        NSError *__jrErr = nil;                                                                                     \
        NSError **jrErrRef __attribute__((unused)) = &__jrErr;                                                      \
        BOOL __hasVoidReturnType;                                                                                   \
        intptr_t __codeResult = (intptr_t) __builtin_choose_expr(__builtin_types_compatible_p(typeof(CODE), void),  \
            (__hasVoidReturnType = YES, CODE, -1),                                                                  \
            (__hasVoidReturnType = NO, CODE));                                                                      \
        BOOL __hasError = NO;                                                                                       \
        if (__hasVoidReturnType) {                                                                                  \
            if (__jrErr) {                                                                                          \
                __hasError = YES;                                                                                   \
            }                                                                                                       \
        } else {                                                                                                    \
            if (!__codeResult) {                                                                                    \
                __hasError = YES;                                                                                   \
            }                                                                                                       \
        }                                                                                                           \
        if (__hasError) {                                                                                           \
            NSMutableDictionary *__userInfo = [NSMutableDictionary dictionaryWithObjectsAndKeys:                    \
                [NSString stringWithUTF8String:__FILE__], @"__FILE__",                                              \
                [NSNumber numberWithInt:__LINE__], @"__LINE__",                                                     \
                [NSString stringWithUTF8String:__PRETTY_FUNCTION__], @"__PRETTY_FUNCTION__",                        \
                [NSString stringWithUTF8String:#CODE], @"CODE",                                                     \
                [NSThread callStackSymbols], @"callStack",                                                          \
                nil];                                                                                               \
            NSError *__mergedError;                                                                                 \
            if (__jrErr) {                                                                                          \
                [__userInfo setValuesForKeysWithDictionary:[__jrErr userInfo]];                                     \
                __mergedError = [NSError errorWithDomain:[__jrErr domain]                                           \
                                                     code:[__jrErr code]                                            \
                                                 userInfo:__userInfo];                                              \
            } else {                                                                                                \
                __mergedError = [NSError errorWithDomain:JRErrDomain                                                \
                                                     code:-1                                                        \
                                                 userInfo:__userInfo];                                              \
            }                                                                                                       \
            [[JRErrContext currentContext] pushError:__mergedError];                                                \
            if (__shouldThrow) {                                                                                    \
                [[NSException exceptionWithName:@"NSError"                                                          \
                                         reason:[__mergedError description]                                         \
                                       userInfo:[NSDictionary dictionaryWithObject:__mergedError                    \
                                                                            forKey:@"error"]] raise];               \
            }                                                                                                       \
        }                                                                                                           \
        _Pragma("clang diagnostic push")                                                                            \
        _Pragma("clang diagnostic ignored \"-Wunused-value\"")                                                      \
        (typeof(CODE))__codeResult;                                                                                 \
        _Pragma("clang diagnostic pop")                                                                             \
    })

#define JRPushErr(CODE)   JRPushErrImpl(CODE, NO)
#define JRThrowErr(CODE)  JRPushErrImpl(CODE, YES)

//-----------------------------------------------------------------------------------------

// If you want to use JRPushErrMsg with [NSString stringWithFormat:] and its ilk, you'll have to wrap the call
// in an extra set of parentheses to overcome that the preprocessor doesn't understand Obj-C syntax:
//     JRPushErrMsg(([NSString stringWithFormat:@"Couldn't open file %@", fileName]), @"Unknown format.");

#define JRPushErrMsg(__failureDescription, __reasonDescription)                                                 \
    {{                                                                                                          \
        NSDictionary *__userInfo = [NSMutableDictionary dictionaryWithObjectsAndKeys:                           \
                                   __failureDescription, NSLocalizedDescriptionKey,                             \
                                   __reasonDescription, NSLocalizedFailureReasonErrorKey,                       \
                                   [NSString stringWithUTF8String:__FILE__], @"__FILE__",                       \
                                   [NSNumber numberWithInt:__LINE__], @"__LINE__",                              \
                                   [NSString stringWithUTF8String:__PRETTY_FUNCTION__], @"__PRETTY_FUNCTION__", \
                                   [NSThread callStackSymbols], @"callStack",                                   \
                                   nil];                                                                        \
        [[JRErrContext currentContext] pushError:[NSError errorWithDomain:[[self class] description]            \
                                                                     code:-1                                    \
                                                                 userInfo:__userInfo]];                         \
    }}

//-----------------------------------------------------------------------------------------

#define JRErrEqual(__err, __domain, __code)    \
    (__err && [[__err domain] isEqualToString:__domain] && [__err code] == __code)

//-----------------------------------------------------------------------------------------

#define JRMakeErrUserInfo()                                                             \
    [NSMutableDictionary dictionaryWithObjectsAndKeys:                                  \
        [NSString stringWithUTF8String:__FILE__], @"__FILE__",                          \
        [NSNumber numberWithInt:__LINE__], @"__LINE__",                                 \
        [NSString stringWithUTF8String:__PRETTY_FUNCTION__], @"__PRETTY_FUNCTION__",    \
        [NSThread callStackSymbols], @"callStack",                                      \
        nil]                                                                            \

//-----------------------------------------------------------------------------------------

// Function-macros with optional parameters technique stolen from http://stackoverflow.com/a/8814003/5260

#define returnJRErr(...)            \
    returnJRErr_X(,                 \
        ##__VA_ARGS__,              \
        returnJRErr_2(__VA_ARGS__), \
        returnJRErr_1(__VA_ARGS__), \
        returnJRErr_0(__VA_ARGS__))

#define returnJRErr_X(ignored,A,B,FUNC,...) FUNC

#define returnJRErr_0() \
    returnJRErr_2(YES, NO)

#define returnJRErr_1(_successValue) \
    returnJRErr_2(_successValue, nil)

#if defined(JRLogNSError)
    #define returnJRErr_2(_successValue, _errorValue)                               \
        if (jrErr) {                                                                \
            if (error) {                                                            \
                *error = jrErr;                                                     \
            } else {                                                                \
                for(NSError *_errItr in [JRErrContext currentContext].errorStack) { \
                    JRLogNSError(_errItr);                                          \
                }                                                                   \
                [[JRErrContext currentContext].errorStack removeAllObjects];        \
            }                                                                       \
            return _errorValue;                                                     \
        } else {                                                                    \
            return _successValue;                                                   \
        }
#else
    #define returnJRErr_2(_successValue, _errorValue)                               \
        if (jrErr) {                                                                \
            if (error) {                                                            \
                *error = jrErr;                                                     \
            } else {                                                                \
                for(NSError *_errItr in [JRErrContext currentContext].errorStack) { \
                    NSLog(@"error: %@", _errItr);                                   \
                }                                                                   \
                [[JRErrContext currentContext].errorStack removeAllObjects];        \
            }                                                                       \
            return _errorValue;                                                     \
        } else {                                                                    \
            return _successValue;                                                   \
        }
#endif

//----------------------------------------------------------------------------------------- 

@interface JRErrContext : NSObject {
#ifndef NOIVARS
  @protected
    NSMutableArray  *errorStack;
#endif
}
@property(retain)  NSMutableArray  *errorStack;

+ (JRErrContext*)currentContext;

- (NSError*)currentError;

- (void)pushError:(NSError*)error;
- (NSError*)popError;

@end
