// dir_patch_types.h
// dir patch
//
/*
 The MIT License (MIT)
 Copyright (c) 2018-2019 HouSisong
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef DirPatch_dir_patch_types_h
#define DirPatch_dir_patch_types_h
#include "../../libHDiffPatch/HPatch/patch_types.h"
#ifdef __cplusplus
extern "C" {
#endif
    
struct THDiffSets{
    hpatch_BOOL isDiffInMem;//or diff by stream
    hpatch_BOOL isSingleCompressedDiff;
    //diff in mem
    hpatch_BOOL isUseBigCacheMatch;
    hpatch_BOOL isUseFastMatchBlock;
    size_t matchScore;
    size_t fastMatchBlockSize;
    //diff by stream
    size_t patchStepMemSize;
    size_t matchBlockSize;
};

#ifndef _IS_NEED_DIR_DIFF_PATCH
#   define _IS_NEED_DIR_DIFF_PATCH  1
#endif
#ifndef _IS_NEED_SINGLE_STREAM_DIFF
#   define _IS_NEED_SINGLE_STREAM_DIFF 1
#endif


#if (_IS_NEED_DIR_DIFF_PATCH)
#   define kMaxOpenFileNumber_limit_min          4
#   define kMaxOpenFileNumber_default_min        8 //must >= limit_min
#   define kMaxOpenFileNumber_default_diff      48
#   define kMaxOpenFileNumber_default_patch     24
#endif

#ifdef _WIN32
static const char kPatch_dirSeparator = '\\';
#else
static const char kPatch_dirSeparator = '/';
#endif
static const char kPatch_dirSeparator_saved = '/';

static hpatch_inline  //align upper
hpatch_StreamPos_t toAlignRangeSize(hpatch_StreamPos_t rangeSize,size_t kAlignSize)
        { return (rangeSize+kAlignSize-1)/kAlignSize*kAlignSize; }

#ifdef __cplusplus
}
#endif
#endif //DirPatch_dir_patch_types_h
