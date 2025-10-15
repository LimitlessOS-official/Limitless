#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../include/syscall.h"

static int kvsnprintf(char* buf, size_t sz, const char* fmt, va_list ap){
    // Very tiny formatter: supports %s %d %u %x %p
    char* out = buf; size_t rem = sz?sz-1:0; const char* p=fmt;
    while (*p && rem){
        if (*p!='%'){ *out++=*p++; rem--; continue; }
        p++;
        if (*p=='s'){ const char* s = va_arg(ap,const char*); if(!s) s="(null)"; size_t l=strlen(s); if(l>rem) l=rem; memcpy(out,s,l); out+=l; rem-=l; p++; continue; }
        if (*p=='d' || *p=='u' || *p=='x' || *p=='p'){
            unsigned long long v = 0; int neg=0; if(*p=='d'){ long long t=va_arg(ap,long long); if(t<0){neg=1; v=(unsigned long long)(-t);} else v=(unsigned long long)t; }
            else v = va_arg(ap, unsigned long long);
            char tmp[32]; const char* hx="0123456789abcdef"; int base=(*p=='x'||*p=='p')?16:10; int i=0; if (v==0) tmp[i++]='0'; while(v && i<(int)sizeof(tmp)){ int d=(int)(v%base); tmp[i++]=hx[d]; v/=base; }
            if (*p=='p'){ tmp[i++]='x'; tmp[i++]='0'; }
            if (neg) tmp[i++]='-';
            while(i && rem){ *out++=tmp[--i]; rem--; }
            p++; continue;
        }
        // fallback
        *out++='%'; rem--; 
    }
    if (sz) *out=0; return (int)(out-buf);
}

int vsnprintf(char* buf, size_t sz, const char* fmt, va_list ap){ return kvsnprintf(buf,sz,fmt,ap); }
int snprintf(char* buf, size_t sz, const char* fmt, ...){ va_list ap; va_start(ap,fmt); int n=kvsnprintf(buf,sz,fmt,ap); va_end(ap); return n; }
int printf(const char* fmt, ...){ char b[512]; va_list ap; va_start(ap,fmt); int n=kvsnprintf(b,sizeof(b),fmt,ap); va_end(ap); sys_write(1,b,(size_t)n); return n; }
