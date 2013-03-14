// JRErr.h semver:1.0.0
//   Copyright (c) 2012-2013 Jonathan 'Wolf' Rentzsch: http://rentzsch.com
//   Some rights reserved: http://opensource.org/licenses/mit
//   https://github.com/rentzsch/JRErr

#import <Foundation/Foundation.h>

#ifndef NS
    #ifdef NS_NAMESPACE
        #define JRNS_CONCAT_TOKENS(a,b) a##_##b
        #define JRNS_EVALUATE(a,b) JRNS_CONCAT_TOKENS(a,b)
        #define NS(original_name) JRNS_EVALUATE(NS_NAMESPACE, original_name)
    #else
        #define NS(original_name) original_name
    #endif
#endif

#define jrErr [[JRErrContext currentContext] currentError]

extern NSString * const NS(JRErrDomain);
#define JRErrDomain NS(JRErrDomain)

//-----------------------------------------------------------------------------------------

#define JRPushErrImpl(CODE, __decider, __annotator, __shouldThrow)                                                  \
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
            __hasError = !__decider(@encode(typeof(CODE)), __codeResult);                                           \
        }                                                                                                           \
        if (__hasError) {                                                                                           \
            NSMutableDictionary *__userInfo = [NSMutableDictionary dictionaryWithObjectsAndKeys:                    \
                [NSString stringWithUTF8String:__FILE__], @"__FILE__",                                              \
                [NSNumber numberWithInt:__LINE__], @"__LINE__",                                                     \
                [NSString stringWithUTF8String:__PRETTY_FUNCTION__], @"__PRETTY_FUNCTION__",                        \
                [NSString stringWithUTF8String:#CODE], @"CODE",                                                     \
                [NSThread callStackSymbols], @"callStack",                                                          \
                nil];                                                                                               \
            if (__annotator) {                                                                                      \
                __annotator(@encode(typeof(CODE)), __codeResult, __userInfo);                                       \
            }                                                                                                       \
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
                @throw [JRErrException exceptionWithError:__mergedError];                                           \
            }                                                                                                       \
        }                                                                                                           \
        _Pragma("clang diagnostic push")                                                                            \
        _Pragma("clang diagnostic ignored \"-Wunused-value\"")                                                      \
        (typeof(CODE))__codeResult;                                                                                 \
        _Pragma("clang diagnostic pop")                                                                             \
    })

extern BOOL JRErrStandardDecider(const char *codeResultType,
                                 intptr_t codeResultValue); // NO indicates an error was detected
extern void JRErrStandardAnnotator(const char *codeResultType,
                                   intptr_t codeResultValue,
                                   NSMutableDictionary *errorUserInfo);

#define kPushJRErr   NO
#define kThrowJRErr  YES

#define JRPushErr(CODE)   JRPushErrImpl(CODE, JRErrStandardDecider, JRErrStandardAnnotator, kPushJRErr)
#define JRThrowErr(CODE)  JRPushErrImpl(CODE, JRErrStandardDecider, JRErrStandardAnnotator, kThrowJRErr)

//-----------------------------------------------------------------------------------------

#define JRMakeErrUserInfo()                                                             \
    [NSMutableDictionary dictionaryWithObjectsAndKeys:                                  \
        [NSString stringWithUTF8String:__FILE__], @"__FILE__",                          \
        [NSNumber numberWithInt:__LINE__], @"__LINE__",                                 \
        [NSString stringWithUTF8String:__PRETTY_FUNCTION__], @"__PRETTY_FUNCTION__",    \
        [NSThread callStackSymbols], @"callStack",                                      \
        nil]

//-----------------------------------------------------------------------------------------

// If you want to use JRPushErrMsg with [NSString stringWithFormat:] and its ilk, you'll have to wrap the call
// in an extra set of parentheses to overcome that the preprocessor doesn't understand Obj-C syntax:
//     JRPushErrMsg(([NSString stringWithFormat:@"Couldn't open file %@", fileName]), @"Unknown format.");

#define JRMakeErrMsg(__failureDescription, __reasonDescription)                                                 \
    ({                                                                                                          \
        NSMutableDictionary *__userInfo = JRMakeErrUserInfo();                                                  \
        [__userInfo setObject:__failureDescription forKey:NSLocalizedDescriptionKey];                           \
        if (__reasonDescription) {                                                                              \
            [__userInfo setObject:__reasonDescription forKey:NSLocalizedFailureReasonErrorKey];                 \
        }                                                                                                       \
        NSError *__err = [NSError errorWithDomain:[[self class] description]                                    \
                                             code:-1                                                            \
                                         userInfo:__userInfo];                                                  \
        __err;                                                                                                  \
    })

#define JRPushErrMsgImpl(__failureDescription, __reasonDescription, __shouldThrow)                              \
    {{                                                                                                          \
        NSError *__err = JRMakeErrMsg(__failureDescription,__reasonDescription);                                \
        [[JRErrContext currentContext] pushError:__err];                                                        \
        if (__shouldThrow) {                                                                                    \
            @throw [JRErrException exceptionWithError:__err];                                                   \
        }                                                                                                       \
    }}

#define JRPushErrMsg(__failureDescription, __reasonDescription)     \
    JRPushErrMsgImpl(__failureDescription, __reasonDescription, NO)

#define JRThrowErrMsg(__failureDescription, __reasonDescription)    \
    JRPushErrMsgImpl(__failureDescription, __reasonDescription, YES)

//-----------------------------------------------------------------------------------------

#define JRErrEqual(__err, __domain, __code)    \
    (__err && [[__err domain] isEqualToString:__domain] && [__err code] == __code)

//-----------------------------------------------------------------------------------------

#if defined(JRLogNSError)
    #define LogJRErr()                                                      \
        for(NSError *_errItr in [JRErrContext currentContext].errorStack) { \
            JRLogNSError(_errItr);                                          \
        }                                                                   \
        [[JRErrContext currentContext].errorStack removeAllObjects];
#else
    #define LogJRErr()                                                      \
        for(NSError *_errItr in [JRErrContext currentContext].errorStack) { \
            NSLog(@"error: %@", _errItr);                                   \
        }                                                                   \
        [[JRErrContext currentContext].errorStack removeAllObjects];
#endif

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

#define returnJRErr_2(_successValue, _errorValue)                           \
    if (jrErr) {                                                            \
        if (error) {                                                        \
            *error = jrErr;                                                 \
            [[JRErrContext currentContext].errorStack removeAllObjects];    \
        } else {                                                            \
            LogJRErr();                                                     \
        }                                                                   \
        return _errorValue;                                                 \
    } else {                                                                \
        return _successValue;                                               \
    }

//----------------------------------------------------------------------------------------- 

@interface NS(JRErrContext) : NSObject {
#ifndef NOIVARS
  @protected
    NSMutableArray  *errorStack;
#endif
}
@property(retain)  NSMutableArray  *errorStack;

+ (NS(JRErrContext)*)currentContext;

- (NSError*)currentError;

- (void)pushError:(NSError*)error;
- (NSError*)popError;

@end
#define JRErrContext NS(JRErrContext)

//-----------------------------------------------------------------------------------------

@interface NS(JRErrException) : NSException
+ (id)exceptionWithError:(NSError*)error;
- (id)initWithError:(NSError*)error;
@end
#define JRErrException NS(JRErrException)
