#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "runtime.h"
#include "service_manager.h"
#include "persona.h"
#include "syscall.h"

/* Phase 6: mount real root/tmp and launch shell */
static void mount_filesystems(void) {
    /* Prefer virtio-blk as vda */
    if (sys_mount("ext4", "vda", "/", "") != 0) {
        if (sys_mount("fat32", "vda", "/", "") != 0) {
            sys_mount("tmpfs", "tmpfs", "/", "");
        }
    }
    sys_mount("tmpfs", "tmpfs", "/tmp", "");
}

static const service_desc_t SERVICES[] = {
    { .name="journald", .path="/bin/journald", .deps={NULL}, .restart_on_crash=1 },
    { .name=NULL }
};

/* Load firewall policy from /etc/policy.conf (line-based):
   - default in|out ACCEPT|DROP
   - rule in|out proto udp|tcp|icmp|any [sport N] [dport N] [src A.B.C.D/M] [dst A.B.C.D/M] ACCEPT|DROP */
static uint32_t parse_ip(const char* s){ unsigned a=0,b=0,c=0,d=0; (void)sscanf(s, "%u.%u.%u.%u", &a,&b,&c,&d); return (a<<24)|(b<<16)|(c<<8)|d; }
static uint32_t mask_from_bits(unsigned m){ if(m==0) return 0; if(m>=32) return 0xFFFFFFFFu; return (m==0)?0:(0xFFFFFFFFu << (32 - m)); }
static unsigned atoi_u(const char* s){ unsigned v=0; while(*s>='0'&&*s<='9'){ v = v*10 + (unsigned)(*s - '0'); s++; } return v; }

static void load_firewall_policy(void){
    /* Open config */
#ifndef O_RDONLY
#define O_RDONLY 0x0000
#endif
    int fd = (int)sys_open("/etc/policy.conf", O_RDONLY, 0);
    if (fd < 0) return; /* optional */
    /* Read entire file */
    char buf[4096]; long n = sys_read(fd, buf, sizeof(buf)-1); sys_close(fd); if (n<=0) return; buf[n]=0;
    /* Parse line by line */
    char* s = buf;
    while (*s){
        /* isolate line */
        char* line = s; while(*s && *s!='\n' && *s!='\r') s++; char ch=*s; if(*s) *s++=0; while(*s=='\n'||*s=='\r') s++;
        /* skip comments and empty */
        char* p=line; while(*p==' '||*p=='\t') p++; if(*p=='#' || *p==0) continue;
        if (strncmp(p, "default", 7)==0){
            p+=7; while(*p==' '||*p=='\t') p++;
            int dir = 0; if (strncmp(p,"out",3)==0){ dir=1; p+=3; } else if (strncmp(p,"in",2)==0){ dir=0; p+=2; } else continue;
            while(*p==' '||*p=='\t') p++;
            int verdict = 0; if (strncmp(p,"ACCEPT",6)==0){ verdict=1; } else if (strncmp(p,"DROP",4)==0){ verdict=0; } else continue;
            sys_fw_set_default((uint8_t)dir, (uint8_t)verdict);
        } else if (strncmp(p, "rule", 4)==0){
            p+=4; while(*p==' '||*p=='\t') p++;
            u_fw_rule_t r; memset(&r,0,sizeof(r));
            if (strncmp(p,"out",3)==0){ r.direction=1; p+=3; } else if (strncmp(p,"in",2)==0){ r.direction=0; p+=2; } else { continue; }
            while(*p){ while(*p==' '||*p=='\t') p++; if(!*p) break;
                if (strncmp(p,"proto",5)==0){ p+=5; while(*p==' '||*p=='\t') p++; if (strncmp(p,"udp",3)==0){ r.proto=17; p+=3; } else if (strncmp(p,"tcp",3)==0){ r.proto=6; p+=3; } else if (strncmp(p,"icmp",4)==0){ r.proto=1; p+=4; } else if (strncmp(p,"any",3)==0){ r.proto=0; p+=3; } }
                else if (strncmp(p,"sport",5)==0){ p+=5; while(*p==' '||*p=='\t') p++; r.src_port_be=(uint16_t)atoi_u(p); while(*p && *p!=' '&&*p!='\t') p++; }
                else if (strncmp(p,"dport",5)==0){ p+=5; while(*p==' '||*p=='\t') p++; r.dst_port_be=(uint16_t)atoi_u(p); while(*p && *p!=' '&&*p!='\t') p++; }
                else if (strncmp(p,"src",3)==0){ p+=3; while(*p==' '||*p=='\t') p++; unsigned a=0; unsigned m=32; const char* q=p; while(*q && *q!=' '&&*q!='\t'&&*q!='/') q++; if(*q=='/'){ a = parse_ip(p); q++; m = (unsigned)atoi_u(q); } else { a = parse_ip(p); m=32; }
                    r.src_ip_be = a; r.src_mask_be = mask_from_bits(m); while(*p && *p!=' '&&*p!='\t') p++; }
                else if (strncmp(p,"dst",3)==0){ p+=3; while(*p==' '||*p=='\t') p++; unsigned a=0; unsigned m=32; const char* q=p; while(*q && *q!=' '&&*q!='\t'&&*q!='/') q++; if(*q=='/'){ a = parse_ip(p); q++; m = (unsigned)atoi_u(q); } else { a = parse_ip(p); m=32; }
                    r.dst_ip_be = a; r.dst_mask_be = mask_from_bits(m); while(*p && *p!=' '&&*p!='\t') p++; }
                else if (strncmp(p,"ACCEPT",6)==0){ r.action=1; p+=6; }
                else if (strncmp(p,"DROP",4)==0){ r.action=0; p+=4; }
                else { /* skip token */ while(*p && *p!=' '&&*p!='\t') p++; }
            }
            sys_fw_add_rule(&r);
        }
    }
}

int main(void) {
    const char* argv[] = { "/bin/login", NULL };
    /* Load firewall policy if available */
    load_firewall_policy();
    sys_write(1, "Init: starting services\n", 26);
    sm_init();
    sm_register(SERVICES);
    (void)sm_start_all();
    sys_update_mark_success();
    sys_write(1, "Init: launching /bin/login\n", 28);
    long rc = sys_exec("/bin/login", argv);
    if (rc != 0) {
        sys_write(2, "Init: failed to exec /bin/sh\n", 30);
    }
    /* If exec returned, keep system alive */
    for (;;)
        sys_sleep_ms(1000);
    return 0;
}