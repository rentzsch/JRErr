// JRErr.h semver:0.0.2
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

#define JRReturnErr()                           \
    if (jrErr) {                                \
        if (error) {                            \
            *error = jrErr;                     \
        } else {                                \
            JRLogNSError(jrErr);                \
        }                                       \
        return NO;                              \
    } else {                                    \
        return YES;                             \
    }


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
