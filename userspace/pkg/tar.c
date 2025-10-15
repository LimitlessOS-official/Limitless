#include "tar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tar_ctx {
    FILE* f;
    uint64_t entry_size;
    uint64_t entry_pos;
    long data_start; // ftell position for current entry data
    int   at_eof;
};

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} tar_header_t;

static uint64_t octal_to_u64(const char* s, size_t n){
    uint64_t v=0; size_t i=0; while(i<n && (s[i]==' '||s[i]=='\0')) i++; for(;i<n && s[i]>='0'&&s[i]<='7';++i){ v=(v<<3)|(uint64_t)(s[i]-'0'); } return v; }

static int is_zero_block(const unsigned char* b){ for(int i=0;i<512;i++) if(b[i]!=0) return 0; return 1; }

tar_ctx_t* tar_open(const char* tar_path){
    FILE* f = fopen(tar_path, "rb");
    if(!f) return NULL;
    tar_ctx_t* ctx = (tar_ctx_t*)calloc(1,sizeof(tar_ctx_t));
    if(!ctx){ fclose(f); return NULL; }
    ctx->f = f; ctx->at_eof = 0; return ctx;
}

void tar_close(tar_ctx_t* ctx){ if(!ctx) return; if(ctx->f) fclose(ctx->f); free(ctx); }

int tar_next(tar_ctx_t* ctx, tar_entry_t* out){
    if(!ctx || !out || ctx->at_eof) return 0;

    // If previous entry had remaining data, skip to next header block boundary
    if(ctx->entry_pos < ctx->entry_size){
        uint64_t remain = ctx->entry_size - ctx->entry_pos;
        uint64_t to_skip = remain;
        const size_t buf_sz = 4096; char buf[buf_sz];
        while(to_skip){ size_t n = to_skip>buf_sz?buf_sz:(size_t)to_skip; size_t r=fread(buf,1,n,ctx->f); if(r==0) break; to_skip -= r; }
        // align to 512
        long cur = ftell(ctx->f); long aligned = (long)((cur + 511L) & ~511L); if(aligned>cur) fseek(ctx->f, aligned - cur, SEEK_CUR);
    }

    unsigned char block[512]; size_t r = fread(block,1,512,ctx->f); if(r!=512) return 0;
    if(is_zero_block(block)){
        // Expect second zero block then EOF
        r = fread(block,1,512,ctx->f); (void)r; ctx->at_eof = 1; return 0;
    }

    const tar_header_t* h = (const tar_header_t*)block;
    static char fullpath[256];
    if(h->prefix[0]){ snprintf(fullpath,sizeof(fullpath),"%.*s/%.*s", (int)sizeof(h->prefix), h->prefix, (int)sizeof(h->name), h->name); }
    else{ snprintf(fullpath,sizeof(fullpath),"%.*s", (int)sizeof(h->name), h->name); }
    // trim trailing nulls
    for(int i=(int)strlen(fullpath)-1;i>=0;i--){ if(fullpath[i]=='\0') fullpath[i]=0; else break; }

    out->path = fullpath;
    out->type = h->typeflag ? h->typeflag : '0';
    out->size = octal_to_u64(h->size, sizeof(h->size));
    out->linkname = h->linkname[0] ? h->linkname : NULL;

    ctx->entry_size = out->size;
    ctx->entry_pos = 0;
    ctx->data_start = ftell(ctx->f);
    return 1;
}

size_t tar_read_data(tar_ctx_t* ctx, void* buf, size_t len){
    if(!ctx || !buf) return 0; if(ctx->entry_pos >= ctx->entry_size) return 0;
    uint64_t remain = ctx->entry_size - ctx->entry_pos;
    if(len > remain) len = (size_t)remain;
    size_t r = fread(buf,1,len,ctx->f);
    ctx->entry_pos += r;
    return r;
}

int tar_skip_entry(tar_ctx_t* ctx){
    if(!ctx) return -1; if(ctx->entry_pos < ctx->entry_size){ uint64_t rem = ctx->entry_size - ctx->entry_pos; fseek(ctx->f, (long)rem, SEEK_CUR); ctx->entry_pos = ctx->entry_size; }
    long cur = ftell(ctx->f); long aligned = (long)((cur + 511L) & ~511L); if(aligned>cur) fseek(ctx->f, aligned - cur, SEEK_CUR);
    return 0;
}
