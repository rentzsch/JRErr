// JRErr.h semver:0.0.3
//   Copyright (c) 2012 Jonathan 'Wolf' Rentzsch: http://rentzsch.com
//   Some rights reserved: http://opensource.org/licenses/MIT
//   https://github.com/rentzsch/JRErr

#import <Foundation/Foundation.h>

#define jrErr [[JRErrContext currentContext] currentError]

#define JRPushErr(CODE)                                                                                             \
    ({                                                                                                              \
        NSError *_jrErr = nil;                                                                                      \
        NSError **jrErrRef = &_jrErr;                                                                               \
        BOOL _hasVoidReturnType;                                                                                    \
        intptr_t _codeResult = (intptr_t) __builtin_choose_expr(__builtin_types_compatible_p(typeof(CODE), void),   \
            (_hasVoidReturnType = YES, CODE, -1),                                                                   \
            (_hasVoidReturnType = NO, CODE));                                                                       \
        BOOL _hasError = NO;                                                                                        \
        if (_hasVoidReturnType) {                                                                                   \
            if (_jrErr) {                                                                                           \
                _hasError = YES;                                                                                    \
            }                                                                                                       \
        } else {                                                                                                    \
            if (!_codeResult) {                                                                                     \
                _hasError = YES;                                                                                    \
            }                                                                                                       \
        }                                                                                                           \
        if (_hasError) {                                                                                            \
            NSMutableDictionary *_userInfo = [NSMutableDictionary dictionaryWithObjectsAndKeys:                     \
                [NSString stringWithUTF8String:__FILE__],   @"__FILE__",                                            \
                [NSNumber numberWithInt:__LINE__],   @"__LINE__",                                                   \
                [NSString stringWithUTF8String:__PRETTY_FUNCTION__], @"__PRETTY_FUNCTION__",                        \
                [NSString stringWithUTF8String:#CODE], @"CODE",                                                     \
                [NSThread callStackSymbols], @"callStack",                                                          \
                nil];                                                                                               \
            NSError *_mergedError;                                                                                  \
            if (_jrErr) {                                                                                           \
                [_userInfo setValuesForKeysWithDictionary:[_jrErr userInfo]];                                       \
                _mergedError = [NSError errorWithDomain:[_jrErr domain]                                             \
                                                     code:[_jrErr code]                                             \
                                                 userInfo:_userInfo];                                               \
            } else {                                                                                                \
                _mergedError = [NSError errorWithDomain:@"JRErrDomain"                                              \
                                                     code:-1                                                        \
                                                 userInfo:_userInfo];                                               \
            }                                                                                                       \
            [[JRErrContext currentContext] pushError:_mergedError];                                                 \
        }                                                                                                           \
        _Pragma("clang diagnostic push")                                                                            \
        _Pragma("clang diagnostic ignored \"-Wunused-value\"")                                                      \
        (typeof(CODE))_codeResult;                                                                                  \
        _Pragma("clang diagnostic pop")                                                                             \
    })

#define returnJRErr_0() \
    returnJRErr_2(YES, NO)

#define returnJRErr_1(_successValue) \
    returnJRErr_2(_successValue, nil)

#if defined(JRLogNSError)
    #define returnJRErr_2(_successValue, _errorValue)   \
        if (jrErr) {                                    \
            if (error) {                                \
                *error = jrErr;                         \
            } else {                                    \
                JRLogNSError(jrErr);                    \
            }                                           \
            return _errorValue;                         \
        } else {                                        \
            return _successValue;                       \
        }
#else
    #define returnJRErr_2(_successValue, _errorValue)   \
        if (jrErr) {                                    \
            if (error) {                                \
                *error = jrErr;                         \
            } else {                                    \
                NSLog(@"error: %@", jrErr);             \
            }                                           \
            return _errorValue;                         \
        } else {                                        \
            return _successValue;                       \
        }
#endif

#define returnJRErr_X(ignored,A,B,FUNC,...) FUNC

#define returnJRErr(...)            \
    returnJRErr_X(,                 \
        ##__VA_ARGS__,              \
        returnJRErr_2(__VA_ARGS__), \
        returnJRErr_1(__VA_ARGS__), \
        returnJRErr_0(__VA_ARGS__))

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
