//decompress_plugin_demo.h
//  decompress plugin demo for HDiffz\HPatchz
/*
 The MIT License (MIT)
 Copyright (c) 2012-2017 HouSisong
 
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
#ifndef HPatch_decompress_plugin_demo_h
#define HPatch_decompress_plugin_demo_h
//decompress plugin demo:
//  zlibDecompressPlugin;
//  bz2DecompressPlugin;
//  lzmaDecompressPlugin;
//  lzma2DecompressPlugin;
//  lz4DecompressPlugin;
//  zstdDecompressPlugin;
//  brotliDecompressPlugin;
//  lzhamDecompressPlugin;

// _bz2DecompressPlugin_unsz support for bspatch_with_cache()

#include <stdlib.h> //malloc free
#include <stdio.h>  //fprintf
#include "libHDiffPatch/HPatch/patch_types.h"

#ifndef kDecompressBufSize
#   define kDecompressBufSize (1024*16)
#endif
#ifndef _IsNeedIncludeDefaultCompressHead
#   define _IsNeedIncludeDefaultCompressHead 1
#endif

#define _close_check(value) { if (!(value)) { LOG_ERR("check "#value " ERROR!\n"); result=hpatch_FALSE; } }

#ifdef  _CompressPlugin_zlib
#if (_IsNeedIncludeDefaultCompressHead)
#   include "zlib.h" // http://zlib.net/  https://github.com/madler/zlib
#endif
    typedef struct _zlib_TDecompress{
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        const struct hpatch_TStreamInput* codeStream;
        
        unsigned char*  dec_buf;
        size_t          dec_buf_size;
        z_stream        d_stream;
        signed char     windowBits;
    } _zlib_TDecompress;
    static hpatch_BOOL _zlib_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"zlib"))||(0==strcmp(compressType,"pzlib"));
    }

    static _zlib_TDecompress*  _zlib_decompress_open_by(hpatch_TDecompress* decompressPlugin,
                                                        const hpatch_TStreamInput* codeStream,
                                                        hpatch_StreamPos_t code_begin,
                                                        hpatch_StreamPos_t code_end,
                                                        int  isSavedWindowBits,
                                                        unsigned char* _mem_buf,size_t _mem_buf_size){
        _zlib_TDecompress* self=0;
        int ret;
        signed char kWindowBits=-MAX_WBITS;
        if (isSavedWindowBits){//load kWindowBits
            if (code_end-code_begin<1) return 0;
            if (!codeStream->read(codeStream,code_begin,(unsigned char*)&kWindowBits,
                                  (unsigned char*)&kWindowBits+1)) return 0;
            ++code_begin;
        }
        
        self=(_zlib_TDecompress*)_hpatch_align_upper(_mem_buf,sizeof(hpatch_StreamPos_t));
        assert((_mem_buf+_mem_buf_size)>((unsigned char*)self+sizeof(_zlib_TDecompress)));
        _mem_buf_size=(_mem_buf+_mem_buf_size)-((unsigned char*)self+sizeof(_zlib_TDecompress));
        _mem_buf=(unsigned char*)self+sizeof(_zlib_TDecompress);
        
        memset(self,0,sizeof(_zlib_TDecompress));
        self->dec_buf=_mem_buf;
        self->dec_buf_size=_mem_buf_size;
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->windowBits=kWindowBits;
        
        ret = inflateInit2(&self->d_stream,self->windowBits);
        if (ret!=Z_OK) return 0;
        return self;
    }
    static hpatch_decompressHandle  _zlib_decompress_open(hpatch_TDecompress* decompressPlugin,
                                                          hpatch_StreamPos_t dataSize,
                                                          const hpatch_TStreamInput* codeStream,
                                                          hpatch_StreamPos_t code_begin,
                                                          hpatch_StreamPos_t code_end){
        _zlib_TDecompress* self=0;
        unsigned char* _mem_buf=(unsigned char*)malloc(sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!_mem_buf) return 0;
        self=_zlib_decompress_open_by(decompressPlugin,codeStream,code_begin,code_end,1,
                                      _mem_buf,sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!self)
            free(_mem_buf);
        return self;
    }
    static hpatch_decompressHandle  _zlib_decompress_open_deflate(hpatch_TDecompress* decompressPlugin,
                                                                  hpatch_StreamPos_t dataSize,
                                                                  const hpatch_TStreamInput* codeStream,
                                                                  hpatch_StreamPos_t code_begin,
                                                                  hpatch_StreamPos_t code_end){
        _zlib_TDecompress* self=0;
        unsigned char* _mem_buf=(unsigned char*)malloc(sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!_mem_buf) return 0;
        self=_zlib_decompress_open_by(decompressPlugin,codeStream,code_begin,code_end,0,
                                      _mem_buf,sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!self)
            free(_mem_buf);
        return self;
    }
    static hpatch_BOOL _zlib_decompress_close_by(struct hpatch_TDecompress* decompressPlugin,
                                                 _zlib_TDecompress* self){
        hpatch_BOOL result=hpatch_TRUE;
        if (!self) return result;
        if (self->d_stream.state!=0){
            _close_check(Z_OK==inflateEnd(&self->d_stream));
        }
        memset(self,0,sizeof(_zlib_TDecompress));
        return result;
    }
    static hpatch_BOOL _zlib_decompress_close(struct hpatch_TDecompress* decompressPlugin,
                                              hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        hpatch_BOOL result=_zlib_decompress_close_by(decompressPlugin,self);
        if (self) free(self);
        return result;
    }
    static hpatch_BOOL _zlib_reset_for_next_node(_zlib_TDecompress* self){
        //backup
        Bytef*   next_out_back=self->d_stream.next_out;
        Bytef*   next_in_back=self->d_stream.next_in;
        unsigned int avail_out_back=self->d_stream.avail_out;
        unsigned int avail_in_back=self->d_stream.avail_in;
        //reset
        //if (Z_OK!=inflateEnd(&self->d_stream)) return hpatch_FALSE;
        //if (Z_OK!=inflateInit2(&self->d_stream,self->windowBits)) return hpatch_FALSE;
        if (Z_OK!=inflateReset(&self->d_stream)) return hpatch_FALSE;
        //restore
        self->d_stream.next_out=next_out_back;
        self->d_stream.next_in=next_in_back;
        self->d_stream.avail_out=avail_out_back;
        self->d_stream.avail_in=avail_in_back;
        return hpatch_TRUE;
    }
    static hpatch_BOOL __zlib_do_inflate(hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        uInt avail_out_back,avail_in_back;
        int ret;
        hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
        if ((self->d_stream.avail_in==0)&&(codeLen>0)) {
            size_t readLen=self->dec_buf_size;
            if (readLen>codeLen) readLen=(size_t)codeLen;
            self->d_stream.next_in=self->dec_buf;
            if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf,
                                        self->dec_buf+readLen)) return hpatch_FALSE;//error;
            self->d_stream.avail_in=(uInt)readLen;
            self->code_begin+=readLen;
            codeLen-=readLen;
        }
        
        avail_out_back=self->d_stream.avail_out;
        avail_in_back=self->d_stream.avail_in;
        ret=inflate(&self->d_stream,Z_NO_FLUSH);
        if (ret==Z_OK){
            if ((self->d_stream.avail_in==avail_in_back)&&(self->d_stream.avail_out==avail_out_back))
                return hpatch_FALSE;//error;
        }else if (ret==Z_STREAM_END){
            if (self->d_stream.avail_in+codeLen>0){ //next compress node!
                if (!_zlib_reset_for_next_node(self))
                    return hpatch_FALSE;//error;
            }else{//all end
                if (self->d_stream.avail_out!=0)
                    return hpatch_FALSE;//error;
            }
        }else{
            return hpatch_FALSE;//error;
        }
        return hpatch_TRUE;
    }
    static hpatch_BOOL _zlib_decompress_part(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        assert(out_part_data<=out_part_data_end);
        
        self->d_stream.next_out = out_part_data;
        self->d_stream.avail_out =(uInt)(out_part_data_end-out_part_data);
        while (self->d_stream.avail_out>0) {
            if (!__zlib_do_inflate(self))
                return hpatch_FALSE;//error;
        }
        return hpatch_TRUE;
    }
    static hpatch_inline int _zlib_is_decompress_finish(const hpatch_TDecompress* decompressPlugin,
                                                        hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        while (self->code_begin!=self->code_end){ //for end tag code
            unsigned char _empty;
            self->d_stream.next_out = &_empty;
            self->d_stream.avail_out=0;
            if (!__zlib_do_inflate(self))
                return hpatch_FALSE;//error;
        }
        return   (self->code_begin==self->code_end)
                &(self->d_stream.avail_in==0)
                &(self->d_stream.avail_out==0);
    }
    static hpatch_TDecompress zlibDecompressPlugin={_zlib_is_can_open,_zlib_decompress_open,
                                                    _zlib_decompress_close,_zlib_decompress_part};
    static hpatch_TDecompress zlibDecompressPlugin_deflate={_zlib_is_can_open,_zlib_decompress_open_deflate,
                                                    _zlib_decompress_close,_zlib_decompress_part};
#endif//_CompressPlugin_zlib
    
#ifdef  _CompressPlugin_bz2
#if (_IsNeedIncludeDefaultCompressHead)
#   include "bzlib.h" // http://www.bzip.org/  https://github.com/sisong/bzip2
#endif
    typedef struct _bz2_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        bz_stream       d_stream;
        unsigned char   dec_buf[kDecompressBufSize];
    } _bz2_TDecompress;
    static hpatch_BOOL _bz2_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"bz2"))||(0==strcmp(compressType,"bzip2"))
             ||(0==strcmp(compressType,"pbz2"))||(0==strcmp(compressType,"pbzip2"));
    }
    static hpatch_decompressHandle  _bz2_open(struct hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        int ret;
        _bz2_TDecompress* self=(_bz2_TDecompress*)malloc(sizeof(_bz2_TDecompress));
        if (!self) return 0;
        memset(self,0,sizeof(_bz2_TDecompress)-kDecompressBufSize);
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        ret=BZ2_bzDecompressInit(&self->d_stream,0,0);
        if (ret!=BZ_OK){ free(self); return 0; }
        return self;
    }
    static hpatch_BOOL _bz2_close(struct hpatch_TDecompress* decompressPlugin,
                                  hpatch_decompressHandle decompressHandle){
        hpatch_BOOL result=hpatch_TRUE;
        _bz2_TDecompress* self=(_bz2_TDecompress*)decompressHandle;
        if (!self) return result;
        _close_check(BZ_OK==BZ2_bzDecompressEnd(&self->d_stream));
        free(self);
        return result;
    }
    static hpatch_BOOL _bz2_reset_for_next_node(_bz2_TDecompress* self){
        //backup
        char*   next_out_back=self->d_stream.next_out;
        char*   next_in_back=self->d_stream.next_in;
        unsigned int avail_out_back=self->d_stream.avail_out;
        unsigned int avail_in_back=self->d_stream.avail_in;
        //reset
        if (BZ_OK!=BZ2_bzDecompressEnd(&self->d_stream)) return hpatch_FALSE;
        if (BZ_OK!=BZ2_bzDecompressInit(&self->d_stream,0,0)) return hpatch_FALSE;
        //restore
        self->d_stream.next_out=next_out_back;
        self->d_stream.next_in=next_in_back;
        self->d_stream.avail_out=avail_out_back;
        self->d_stream.avail_in=avail_in_back;
        return hpatch_TRUE;
    }

    static hpatch_BOOL _bz2_decompress_part_(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end,
                                             hpatch_BOOL isMustOutData){
        _bz2_TDecompress* self=(_bz2_TDecompress*)decompressHandle;
        assert(out_part_data<=out_part_data_end);
        
        self->d_stream.next_out =(char*)out_part_data;
        self->d_stream.avail_out =(unsigned int)(out_part_data_end-out_part_data);
        while (self->d_stream.avail_out>0) {
            unsigned int avail_out_back,avail_in_back;
            int ret;
            hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
            if ((self->d_stream.avail_in==0)&&(codeLen>0)) {
                size_t readLen=kDecompressBufSize;
                self->d_stream.next_in=(char*)self->dec_buf;
                if (readLen>codeLen) readLen=(size_t)codeLen;
                if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf,
                                            self->dec_buf+readLen)) return hpatch_FALSE;//error;
                self->d_stream.avail_in=(unsigned int)readLen;
                self->code_begin+=readLen;
                codeLen-=readLen;
            }
            
            avail_out_back=self->d_stream.avail_out;
            avail_in_back=self->d_stream.avail_in;
            ret=BZ2_bzDecompress(&self->d_stream);
            if (ret==BZ_OK){
                if ((self->d_stream.avail_in==avail_in_back)&&(self->d_stream.avail_out==avail_out_back))
                    return hpatch_FALSE;//error;
            }else if (ret==BZ_STREAM_END){
                if (self->d_stream.avail_in+codeLen>0){ //next compress node!
                    if (!_bz2_reset_for_next_node(self))
                        return hpatch_FALSE;//error;
                }else{//all end
                    if (self->d_stream.avail_out!=0){
                        if (isMustOutData){ //fill out 0
                            memset(self->d_stream.next_out,0,self->d_stream.avail_out);
                            self->d_stream.next_out+=self->d_stream.avail_out;
                            self->d_stream.avail_out=0;
                        }else{
                            return hpatch_FALSE;//error;
                        }
                    }
                }
            }else{
                return hpatch_FALSE;//error;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_BOOL _bz2_decompress_part(hpatch_decompressHandle decompressHandle,
                                            unsigned char* out_part_data,unsigned char* out_part_data_end){
        return _bz2_decompress_part_(decompressHandle,out_part_data,out_part_data_end,hpatch_FALSE);
    }
    static hpatch_BOOL _bz2_decompress_part_unsz(hpatch_decompressHandle decompressHandle,
                                                 unsigned char* out_part_data,unsigned char* out_part_data_end){
        return _bz2_decompress_part_(decompressHandle,out_part_data,out_part_data_end,hpatch_TRUE);
    }
    
    static hpatch_TDecompress bz2DecompressPlugin={_bz2_is_can_open,_bz2_open,
                                                   _bz2_close,_bz2_decompress_part};

    //unkown uncompress data size
    static hpatch_TDecompress _bz2DecompressPlugin_unsz={_bz2_is_can_open,_bz2_open,
                                                         _bz2_close,_bz2_decompress_part_unsz};
#endif//_CompressPlugin_bz2


#if (defined _CompressPlugin_lzma) || (defined _CompressPlugin_lzma2)
#if (_IsNeedIncludeDefaultCompressHead)
#   include "LzmaDec.h" // "lzma/C/LzmaDec.h" https://github.com/sisong/lzma
#   ifdef _CompressPlugin_lzma2
#       include "Lzma2Dec.h"
#   endif
#endif
static void * __lzma_dec_Alloc(ISzAllocPtr p, size_t size){
    return malloc(size);
}
static void __lzma_dec_Free(ISzAllocPtr p, void *address){
    if (address) free(address);
}
static ISzAlloc __lzma_dec_alloc={__lzma_dec_Alloc,__lzma_dec_Free};
#endif

#ifdef _CompressPlugin_lzma
    typedef struct _lzma_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        CLzmaDec        decEnv;
        SizeT           decCopyPos;
        SizeT           decReadPos;
        unsigned char   dec_buf[kDecompressBufSize];
    } _lzma_TDecompress;
    static hpatch_BOOL _lzma_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"lzma"));
    }
    static hpatch_decompressHandle  _lzma_open(hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        _lzma_TDecompress* self=0;
        SRes ret;
        unsigned char propsSize=0;
        unsigned char props[256];
        //load propsSize
        if (code_end-code_begin<1) return 0;
        if (!codeStream->read(codeStream,code_begin,&propsSize,&propsSize+1)) return 0;
        ++code_begin;
        if (propsSize>(code_end-code_begin)) return 0;
        //load props
        if (!codeStream->read(codeStream,code_begin,props,props+propsSize)) return 0;
        code_begin+=propsSize;

        self=(_lzma_TDecompress*)malloc(sizeof(_lzma_TDecompress));
        if (!self) return 0;
        memset(self,0,sizeof(_lzma_TDecompress)-kDecompressBufSize);
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        self->decCopyPos=0;
        self->decReadPos=kDecompressBufSize;
        
        LzmaDec_Construct(&self->decEnv);
        ret=LzmaDec_Allocate(&self->decEnv,props,propsSize,&__lzma_dec_alloc);
        if (ret!=SZ_OK){ free(self); return 0; }
        LzmaDec_Init(&self->decEnv);
        return self;
    }
    static hpatch_BOOL _lzma_close(struct hpatch_TDecompress* decompressPlugin,
                                   hpatch_decompressHandle decompressHandle){
        _lzma_TDecompress* self=(_lzma_TDecompress*)decompressHandle;
        if (!self) return hpatch_TRUE;
        LzmaDec_Free(&self->decEnv,&__lzma_dec_alloc);
        free(self);
        return hpatch_TRUE;
    }
    static hpatch_BOOL _lzma_decompress_part(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lzma_TDecompress* self=(_lzma_TDecompress*)decompressHandle;
        unsigned char* out_cur=out_part_data;
        assert(out_part_data<=out_part_data_end);
        while (out_cur<out_part_data_end){
            size_t copyLen=(self->decEnv.dicPos-self->decCopyPos);
            if (copyLen>0){
                if (copyLen>(size_t)(out_part_data_end-out_cur))
                    copyLen=(out_part_data_end-out_cur);
                memcpy(out_cur,self->decEnv.dic+self->decCopyPos,copyLen);
                out_cur+=copyLen;
                self->decCopyPos+=copyLen;
                if ((self->decEnv.dicPos==self->decEnv.dicBufSize)
                    &&(self->decEnv.dicPos==self->decCopyPos)){
                    self->decEnv.dicPos=0;
                    self->decCopyPos=0;
                }
            }else{
                ELzmaStatus status;
                SizeT inSize,dicPos_back;
                SRes res;
                hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
                if ((self->decReadPos==kDecompressBufSize)&&(codeLen>0)) {
                    size_t readLen=kDecompressBufSize;
                    if (readLen>codeLen) readLen=(size_t)codeLen;
                    self->decReadPos=kDecompressBufSize-readLen;
                    if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf+self->decReadPos,
                                                self->dec_buf+self->decReadPos+readLen)) return hpatch_FALSE;//error;
                    self->code_begin+=readLen;
                }

                inSize=kDecompressBufSize-self->decReadPos;
                dicPos_back=self->decEnv.dicPos;
                res=LzmaDec_DecodeToDic(&self->decEnv,self->decEnv.dicBufSize,
                                        self->dec_buf+self->decReadPos,&inSize,LZMA_FINISH_ANY,&status);
                if(res==SZ_OK){
                    if ((inSize==0)&&(self->decEnv.dicPos==dicPos_back))
                        return hpatch_FALSE;//error;
                }else{
                    return hpatch_FALSE;//error;
                }
                self->decReadPos+=inSize;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress lzmaDecompressPlugin={_lzma_is_can_open,_lzma_open,
                                                    _lzma_close,_lzma_decompress_part};
#endif//_CompressPlugin_lzma

#ifdef _CompressPlugin_lzma2
typedef struct _lzma2_TDecompress{
    const struct hpatch_TStreamInput* codeStream;
    hpatch_StreamPos_t code_begin;
    hpatch_StreamPos_t code_end;
    
    CLzma2Dec       decEnv;
    SizeT           decCopyPos;
    SizeT           decReadPos;
    unsigned char   dec_buf[kDecompressBufSize];
} _lzma2_TDecompress;
static hpatch_BOOL _lzma2_is_can_open(const char* compressType){
    return (0==strcmp(compressType,"lzma2"));
}
static hpatch_decompressHandle _lzma2_open(hpatch_TDecompress* decompressPlugin,
                                           hpatch_StreamPos_t dataSize,
                                           const hpatch_TStreamInput* codeStream,
                                           hpatch_StreamPos_t code_begin,
                                           hpatch_StreamPos_t code_end){
    _lzma2_TDecompress* self=0;
    SRes ret;
    unsigned char propsSize=0;
    //load propsSize
    if (code_end-code_begin<1) return 0;
    if (!codeStream->read(codeStream,code_begin,&propsSize,&propsSize+1)) return 0;
    ++code_begin;
    
    self=(_lzma2_TDecompress*)malloc(sizeof(_lzma2_TDecompress));
    if (!self) return 0;
    memset(self,0,sizeof(_lzma2_TDecompress)-kDecompressBufSize);
    self->codeStream=codeStream;
    self->code_begin=code_begin;
    self->code_end=code_end;
    
    self->decCopyPos=0;
    self->decReadPos=kDecompressBufSize;
    
    Lzma2Dec_Construct(&self->decEnv);
    ret=Lzma2Dec_Allocate(&self->decEnv,propsSize,&__lzma_dec_alloc);
    if (ret!=SZ_OK){ free(self); return 0; }
    Lzma2Dec_Init(&self->decEnv);
    return self;
}
static hpatch_BOOL _lzma2_close(struct hpatch_TDecompress* decompressPlugin,
                                hpatch_decompressHandle decompressHandle){
    _lzma2_TDecompress* self=(_lzma2_TDecompress*)decompressHandle;
    if (!self) return hpatch_TRUE;
    Lzma2Dec_Free(&self->decEnv,&__lzma_dec_alloc);
    free(self);
    return hpatch_TRUE;
}
static hpatch_BOOL _lzma2_decompress_part(hpatch_decompressHandle decompressHandle,
                                          unsigned char* out_part_data,unsigned char* out_part_data_end){
    _lzma2_TDecompress* self=(_lzma2_TDecompress*)decompressHandle;
    unsigned char* out_cur=out_part_data;
    assert(out_part_data<=out_part_data_end);
    while (out_cur<out_part_data_end){
        size_t copyLen=(self->decEnv.decoder.dicPos-self->decCopyPos);
        if (copyLen>0){
            if (copyLen>(size_t)(out_part_data_end-out_cur))
                copyLen=(out_part_data_end-out_cur);
            memcpy(out_cur,self->decEnv.decoder.dic+self->decCopyPos,copyLen);
            out_cur+=copyLen;
            self->decCopyPos+=copyLen;
            if ((self->decEnv.decoder.dicPos==self->decEnv.decoder.dicBufSize)
                &&(self->decEnv.decoder.dicPos==self->decCopyPos)){
                self->decEnv.decoder.dicPos=0;
                self->decCopyPos=0;
            }
        }else{
            ELzmaStatus status;
            SizeT inSize,dicPos_back;
            SRes res;
            hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
            if ((self->decReadPos==kDecompressBufSize)&&(codeLen>0)) {
                size_t readLen=kDecompressBufSize;
                if (readLen>codeLen) readLen=(size_t)codeLen;
                self->decReadPos=kDecompressBufSize-readLen;
                if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf+self->decReadPos,
                                            self->dec_buf+self->decReadPos+readLen)) return hpatch_FALSE;//error;
                self->code_begin+=readLen;
            }
            
            inSize=kDecompressBufSize-self->decReadPos;
            dicPos_back=self->decEnv.decoder.dicPos;
            res=Lzma2Dec_DecodeToDic(&self->decEnv,self->decEnv.decoder.dicBufSize,
                                     self->dec_buf+self->decReadPos,&inSize,LZMA_FINISH_ANY,&status);
            if(res==SZ_OK){
                if ((inSize==0)&&(self->decEnv.decoder.dicPos==dicPos_back))
                    return hpatch_FALSE;//error;
            }else{
                return hpatch_FALSE;//error;
            }
            self->decReadPos+=inSize;
        }
    }
    return hpatch_TRUE;
}
static hpatch_TDecompress lzma2DecompressPlugin={_lzma2_is_can_open,_lzma2_open,
                                                 _lzma2_close,_lzma2_decompress_part};
#endif//_CompressPlugin_lzma2


#if (defined(_CompressPlugin_lz4) || defined(_CompressPlugin_lz4hc))
#if (_IsNeedIncludeDefaultCompressHead)
#   include "lz4.h" // "lz4/lib/lz4.h" https://github.com/lz4/lz4
#endif
    typedef struct _lz4_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        LZ4_streamDecode_t *s;
        int                kLz4CompressBufSize;
        int                code_buf_size;
        int                data_begin;
        int                data_end;
        unsigned char      buf[1];
    } _lz4_TDecompress;
    static hpatch_BOOL _lz4_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"lz4"));
    }
    #define _lz4_read_len4(len,in_code,code_begin,code_end) { \
        unsigned char _temp_buf4[4];  \
        if (4>code_end-code_begin) return hpatch_FALSE; \
        if (!in_code->read(in_code,code_begin,_temp_buf4,_temp_buf4+4)) \
            return hpatch_FALSE; \
        len=_temp_buf4[0]|(_temp_buf4[1]<<8)|(_temp_buf4[2]<<16)|(_temp_buf4[3]<<24); \
        code_begin+=4; \
    }
    static hpatch_decompressHandle  _lz4_open(hpatch_TDecompress* decompressPlugin,
                                              hpatch_StreamPos_t dataSize,
                                              const hpatch_TStreamInput* codeStream,
                                              hpatch_StreamPos_t code_begin,
                                              hpatch_StreamPos_t code_end){
        const int kMaxLz4CompressBufSize=(1<<20)*64; //defence attack
        _lz4_TDecompress* self=0;
        int kLz4CompressBufSize=0;
        int code_buf_size=0;
        assert(code_begin<code_end);
        {//read kLz4CompressBufSize
            _lz4_read_len4(kLz4CompressBufSize,codeStream,code_begin,code_end);
            if ((kLz4CompressBufSize<0)||(kLz4CompressBufSize>=kMaxLz4CompressBufSize)) return 0;
            code_buf_size=LZ4_compressBound(kLz4CompressBufSize);
        }
        self=(_lz4_TDecompress*)malloc(sizeof(_lz4_TDecompress)+kLz4CompressBufSize+code_buf_size);
        if (!self) return 0;
        memset(self,0,sizeof(_lz4_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->kLz4CompressBufSize=kLz4CompressBufSize;
        self->code_buf_size=code_buf_size;
        self->data_begin=0;
        self->data_end=0;
        
        self->s = LZ4_createStreamDecode();
        if (!self->s){ free(self); return 0; }
        return self;
    }
    static hpatch_BOOL _lz4_close(struct hpatch_TDecompress* decompressPlugin,
                                  hpatch_decompressHandle decompressHandle){
        hpatch_BOOL result=hpatch_TRUE;
        _lz4_TDecompress* self=(_lz4_TDecompress*)decompressHandle;
        if (!self) return result;
        _close_check(0==LZ4_freeStreamDecode(self->s));
        free(self);
        return result;
    }
    static hpatch_BOOL _lz4_decompress_part(hpatch_decompressHandle decompressHandle,
                                            unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lz4_TDecompress* self=(_lz4_TDecompress*)decompressHandle;
        unsigned char* data_buf=self->buf;
        unsigned char* code_buf=self->buf+self->kLz4CompressBufSize;

        while (out_part_data<out_part_data_end) {
            size_t dataLen=self->data_end-self->data_begin;
            if (dataLen>0){
                if (dataLen>(out_part_data_end-out_part_data))
                    dataLen=(out_part_data_end-out_part_data);
                memcpy(out_part_data,data_buf+self->data_begin,dataLen);
                out_part_data+=(size_t)dataLen;
                self->data_begin+=dataLen;
            }else{
                int codeLen;
                _lz4_read_len4(codeLen,self->codeStream,self->code_begin,self->code_end);
                if ((codeLen<=0)||(codeLen>self->code_buf_size)
                    ||((size_t)codeLen>(self->code_end-self->code_begin))) return hpatch_FALSE;
                if (!self->codeStream->read(self->codeStream,self->code_begin,
                                            code_buf,code_buf+codeLen)) return hpatch_FALSE;
                self->code_begin+=codeLen;
                self->data_begin=0;
                self->data_end=LZ4_decompress_safe_continue(self->s,(const char*)code_buf,(char*)data_buf,
                                                            codeLen,self->kLz4CompressBufSize);
                if (self->data_end<=0) return hpatch_FALSE;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress lz4DecompressPlugin={_lz4_is_can_open,_lz4_open,
                                                   _lz4_close,_lz4_decompress_part};
#endif//_CompressPlugin_lz4 or _CompressPlugin_lz4hc

#ifdef  _CompressPlugin_zstd
#if (_IsNeedIncludeDefaultCompressHead)
#   include "zstd.h" // "zstd/lib/zstd.h" https://github.com/facebook/zstd
#endif
    typedef struct _zstd_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        ZSTD_inBuffer      s_input;
        ZSTD_outBuffer     s_output;
        size_t             data_begin;
        ZSTD_DStream*      s;
        unsigned char      buf[1];
    } _zstd_TDecompress;
    static hpatch_BOOL _zstd_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"zstd"));
    }
    static hpatch_decompressHandle  _zstd_open(hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        _zstd_TDecompress* self=0;
        size_t  ret;
        size_t _input_size=ZSTD_DStreamInSize();
        size_t _output_size=ZSTD_DStreamOutSize();
        assert(code_begin<code_end);
        self=(_zstd_TDecompress*)malloc(sizeof(_zstd_TDecompress)+_input_size+_output_size);
        if (!self) return 0;
        memset(self,0,sizeof(_zstd_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->s_input.src=self->buf;
        self->s_input.size=_input_size;
        self->s_input.pos=_input_size;
        self->s_output.dst=self->buf+_input_size;
        self->s_output.size=_output_size;
        self->s_output.pos=0;
        self->data_begin=0;
        
        self->s = ZSTD_createDStream();
        if (!self->s){ free(self); return 0; }
        ret=ZSTD_initDStream(self->s);
        if (ZSTD_isError(ret)) { ZSTD_freeDStream(self->s); free(self); return 0; }
        #define _ZSTD_WINDOWLOG_MAX ((sizeof(size_t)<=4)?30:31)
        ret=ZSTD_DCtx_setParameter(self->s,ZSTD_d_windowLogMax,_ZSTD_WINDOWLOG_MAX);
        //if (ZSTD_isError(ret)) { printf("WARNING: ZSTD_DCtx_setMaxWindowSize() error!"); }
        return self;
    }
    static hpatch_BOOL _zstd_close(struct hpatch_TDecompress* decompressPlugin,
                                   hpatch_decompressHandle decompressHandle){
        hpatch_BOOL result=hpatch_TRUE;
        _zstd_TDecompress* self=(_zstd_TDecompress*)decompressHandle;
        if (!self) return result;
        _close_check(0==ZSTD_freeDStream(self->s));
        free(self);
        return result;
    }
    static hpatch_BOOL _zstd_decompress_part(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end){
        _zstd_TDecompress* self=(_zstd_TDecompress*)decompressHandle;
        while (out_part_data<out_part_data_end) {
            size_t dataLen=(self->s_output.pos-self->data_begin);
            if (dataLen>0){
                if (dataLen>(size_t)(out_part_data_end-out_part_data))
                    dataLen=(out_part_data_end-out_part_data);
                memcpy(out_part_data,(const unsigned char*)self->s_output.dst+self->data_begin,dataLen);
                out_part_data+=dataLen;
                self->data_begin+=dataLen;
            }else{
                size_t ret;
                if (self->s_input.pos==self->s_input.size) {
                    self->s_input.pos=0;
                    if (self->s_input.size>self->code_end-self->code_begin)
                        self->s_input.size=(size_t)(self->code_end-self->code_begin);

                    if (self->s_input.size>0){
                        if (!self->codeStream->read(self->codeStream,self->code_begin,(unsigned char*)self->s_input.src,
                                                    (unsigned char*)self->s_input.src+self->s_input.size))
                            return hpatch_FALSE;
                        self->code_begin+=self->s_input.size;
                    }
                }
                self->s_output.pos=0;
                self->data_begin=0;
                ret=ZSTD_decompressStream(self->s,&self->s_output,&self->s_input);
                if (ZSTD_isError(ret)) return hpatch_FALSE;
                if (self->s_output.pos==self->data_begin) return hpatch_FALSE;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress zstdDecompressPlugin={_zstd_is_can_open,_zstd_open,
                                                    _zstd_close,_zstd_decompress_part};
#endif//_CompressPlugin_zstd


#ifdef  _CompressPlugin_brotli
#if (_IsNeedIncludeDefaultCompressHead)
#   include "brotli/decode.h" // "brotli/c/include/brotli/decode.h" https://github.com/google/brotli
#endif
    typedef struct _brotli_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        unsigned char*        input;
        unsigned char*        output;
        size_t                available_in;
        size_t                available_out;
        const unsigned char*  next_in;
        unsigned char*        next_out;
        unsigned char*        data_begin;
        BrotliDecoderState* s;
        unsigned char       buf[1];
    } _brotli_TDecompress;
    static hpatch_BOOL _brotli_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"brotli"));
    }
    static hpatch_decompressHandle  _brotli_open(hpatch_TDecompress* decompressPlugin,
                                                 hpatch_StreamPos_t dataSize,
                                                 const hpatch_TStreamInput* codeStream,
                                                 hpatch_StreamPos_t code_begin,
                                                 hpatch_StreamPos_t code_end){
        const size_t kBufSize=kDecompressBufSize;
        _brotli_TDecompress* self=0;
        assert(code_begin<code_end);
        self=(_brotli_TDecompress*)malloc(sizeof(_brotli_TDecompress)+kBufSize*2);
        if (!self) return 0;
        memset(self,0,sizeof(_brotli_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->input=self->buf;
        self->output=self->buf+kBufSize;
        self->code_end=code_end;
        self->available_in = 0;
        self->next_in = 0;
        self->available_out = (self->output-self->input);
        self->next_out  =self->output;
        self->data_begin=self->output;
        
        self->s = BrotliDecoderCreateInstance(0,0,0);
        if (!self->s){ free(self); return 0; }
        if (!BrotliDecoderSetParameter(self->s, BROTLI_DECODER_PARAM_LARGE_WINDOW, 1u))
            { BrotliDecoderDestroyInstance(self->s); free(self); return 0; }
        return self;
    }
    static hpatch_BOOL _brotli_close(struct hpatch_TDecompress* decompressPlugin,
                                     hpatch_decompressHandle decompressHandle){
        _brotli_TDecompress* self=(_brotli_TDecompress*)decompressHandle;
        if (!self) return hpatch_TRUE;
        BrotliDecoderDestroyInstance(self->s);
        free(self);
        return hpatch_TRUE;
    }
    static hpatch_BOOL _brotli_decompress_part(hpatch_decompressHandle decompressHandle,
                                               unsigned char* out_part_data,unsigned char* out_part_data_end){
        _brotli_TDecompress* self=(_brotli_TDecompress*)decompressHandle;
        while (out_part_data<out_part_data_end) {
            size_t dataLen=(self->next_out-self->data_begin);
            if (dataLen>0){
                if (dataLen>(size_t)(out_part_data_end-out_part_data))
                    dataLen=(out_part_data_end-out_part_data);
                memcpy(out_part_data,self->data_begin,dataLen);
                out_part_data+=dataLen;
                self->data_begin+=dataLen;
            }else{
                BrotliDecoderResult ret;
                if (self->available_in==0) {
                    self->available_in=(self->output-self->input);
                    if (self->available_in>self->code_end-self->code_begin)
                        self->available_in=(size_t)(self->code_end-self->code_begin);
                    if (self->available_in>0){
                        if (!self->codeStream->read(self->codeStream,self->code_begin,(unsigned char*)self->input,
                                                    self->input+self->available_in))
                            return hpatch_FALSE;
                        self->code_begin+=self->available_in;
                    }
                    self->next_in=self->input;
                }
                self->available_out = (self->output-self->input);
                self->next_out  =self->output;
                self->data_begin=self->output;
                ret=BrotliDecoderDecompressStream(self->s,&self->available_in,&self->next_in,
                                                  &self->available_out,&self->next_out, 0);
                switch (ret){
                    case BROTLI_DECODER_RESULT_SUCCESS:
                    case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {
                        if (self->next_out==self->data_begin) return hpatch_FALSE;
                    } break;  
                    case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {
                        if (self->code_end==self->code_begin) return hpatch_FALSE;
                    } break;            
                    default:
                        return hpatch_FALSE;
                }
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress brotliDecompressPlugin={_brotli_is_can_open,_brotli_open,
                                                      _brotli_close,_brotli_decompress_part};
#endif//_CompressPlugin_brotli


#ifdef  _CompressPlugin_lzham
#if (_IsNeedIncludeDefaultCompressHead)
#   include "lzham.h" // "lzham_codec/include/lzham.h" https://github.com/richgel999/lzham_codec
#endif
    typedef struct _lzham_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        unsigned char*        input;
        unsigned char*        output;
        size_t                available_in;
        size_t                available_out;
        const unsigned char*  next_in;
        unsigned char*        next_out;
        unsigned char*        data_begin;
        lzham_decompress_state_ptr s;
        unsigned char       buf[1];
    } _lzham_TDecompress;
    static hpatch_BOOL _lzham_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"lzham"));
    }
    static hpatch_decompressHandle  _lzham_open(hpatch_TDecompress* decompressPlugin,
                                                hpatch_StreamPos_t dataSize,
                                                const hpatch_TStreamInput* codeStream,
                                                hpatch_StreamPos_t code_begin,
                                                hpatch_StreamPos_t code_end){
        const size_t kBufSize=kDecompressBufSize;
        lzham_decompress_params params;
        unsigned char  dict_bits;
        _lzham_TDecompress* self=0;
        assert(code_begin<code_end);
        {//load head
            if (code_end-code_begin<1) return 0;
            if (!codeStream->read(codeStream,code_begin,&dict_bits,(&dict_bits)+1))
                return 0;
            ++code_begin;
        }

        self=(_lzham_TDecompress*)malloc(sizeof(_lzham_TDecompress)+kBufSize*2);
        if (!self) return 0;
        memset(self,0,sizeof(_lzham_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->input=self->buf;
        self->output=self->buf+kBufSize;
        self->code_end=code_end;
        self->available_in = 0;
        self->next_in = 0;
        self->available_out = (self->output-self->input);
        self->next_out  =self->output;
        self->data_begin=self->output;

        memset(&params, 0, sizeof(params));
        params.m_struct_size = sizeof(params);
        params.m_dict_size_log2 = dict_bits;

        self->s = lzham_decompress_init(&params);
        if (!self->s){ free(self); return 0; }

        return self;
    }
    static hpatch_BOOL _lzham_close(struct hpatch_TDecompress* decompressPlugin,
                                    hpatch_decompressHandle decompressHandle){
        _lzham_TDecompress* self=(_lzham_TDecompress*)decompressHandle;
        if (!self) return hpatch_TRUE;
        lzham_decompress_deinit(self->s);
        free(self);
        return hpatch_TRUE;
    }
    static hpatch_BOOL _lzham_decompress_part(hpatch_decompressHandle decompressHandle,
                                               unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lzham_TDecompress* self=(_lzham_TDecompress*)decompressHandle;
        while (out_part_data<out_part_data_end) {
            size_t dataLen=(self->next_out-self->data_begin);
            if (dataLen>0){
                if (dataLen>(size_t)(out_part_data_end-out_part_data))
                    dataLen=(out_part_data_end-out_part_data);
                memcpy(out_part_data,self->data_begin,dataLen);
                out_part_data+=dataLen;
                self->data_begin+=dataLen;
            }else{
                lzham_decompress_status_t ret;
                if (self->available_in==0) {
                    self->available_in=(self->output-self->input);
                    if (self->available_in>self->code_end-self->code_begin)
                        self->available_in=(size_t)(self->code_end-self->code_begin);
                    if (self->available_in>0){
                        if (!self->codeStream->read(self->codeStream,self->code_begin,(unsigned char*)self->input,
                                                    self->input+self->available_in))
                            return hpatch_FALSE;
                        self->code_begin+=self->available_in;
                    }
                    self->next_in=self->input;
                }
                {
                    size_t available_in_back=self->available_in;
                    self->available_out = (self->output-self->input);
                    ret=lzham_decompress(self->s,self->next_in,&self->available_in,
                                         self->output,&self->available_out,(self->code_begin==self->code_end));
                    self->next_out=self->output+self->available_out;
                    self->next_in+=self->available_in;
                    self->available_in=available_in_back-self->available_in;
                    self->available_out=(self->output-self->input) - self->available_out;
                    self->data_begin=self->output;
                }
                switch (ret){
                    case LZHAM_DECOMP_STATUS_SUCCESS:
                    case LZHAM_DECOMP_STATUS_HAS_MORE_OUTPUT:
                    case LZHAM_DECOMP_STATUS_NOT_FINISHED: {
                        if (self->next_out==self->data_begin) return hpatch_FALSE;
                    } break;
                    case LZHAM_DECOMP_STATUS_NEEDS_MORE_INPUT: {
                        if (self->code_end==self->code_begin) return hpatch_FALSE;
                    } break;            
                    default:
                        return hpatch_FALSE;
                }
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress lzhamDecompressPlugin={_lzham_is_can_open,_lzham_open,
                                                     _lzham_close,_lzham_decompress_part};
#endif//_CompressPlugin_lzham

#endif
