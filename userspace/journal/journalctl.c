/* journalctl: minimal filtering for klog format lines: <lvl>[ticks] TAG: msg */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifndef ROTATE_FILES
#define ROTATE_FILES 5
#endif
#define LOG_DIR "/var/log/journal"

typedef struct { int have; uint64_t since; uint64_t until; int prio_min; char unit[32]; } filters_t;

static void usage(void){
    puts("Usage: journalctl [--since TICKS] [--until TICKS] [--priority N] [--unit TAG]");
}

static int parse_u64(const char* s, uint64_t* out){ uint64_t v=0; if(!s||!*s) return -1; while(*s && *s>='0'&&*s<='9'){ v = v*10 + (uint64_t)(*s - '0'); s++; } *out=v; return 0; }

static int parse_args(int argc, char** argv, filters_t* f){ memset(f,0,sizeof(*f)); f->prio_min=-1; for(int i=1;i<argc;i++){ if(strcmp(argv[i],"--since")==0 && i+1<argc){ if(parse_u64(argv[++i], &f->since)==0) f->have|=1; }
        else if(strcmp(argv[i],"--until")==0 && i+1<argc){ if(parse_u64(argv[++i], &f->until)==0) f->have|=2; }
        else if(strcmp(argv[i],"--priority")==0 && i+1<argc){ f->prio_min = atoi(argv[++i]); }
        else if(strcmp(argv[i],"--unit")==0 && i+1<argc){ strncpy(f->unit, argv[++i], sizeof(f->unit)-1); }
        else { usage(); return -1; } } return 0; }

static int match_line(const char* line, const filters_t* f){
    /* Expect format: <N>[TTTTTTTT] TAG: msg */
    int prio = -1; uint64_t ticks=0; char tag[32]={0};
    if(line[0]=='<' && line[2]=='>'){ prio = line[1]-'0'; }
    const char* lb = strchr(line, '['); const char* rb = lb? strchr(lb, ']') : NULL;
    if(lb && rb){ for(const char* p=lb+1; p<rb; ++p){ if(*p>='0'&&*p<='9'){ ticks = ticks*10 + (uint64_t)(*p - '0'); } else break; } }
    const char* sp = strchr(line, ' '); const char* colon = sp? strchr(sp+1, ':') : NULL;
    if(sp && colon && colon-sp-1 < (int)sizeof(tag)) { strncpy(tag, sp+1, (size_t)(colon-sp-1)); tag[colon-sp-1]=0; }
    if (f->prio_min >= 0 && prio >= 0 && prio > f->prio_min) return 0; /* lower number = higher priority */
    if ((f->have&1) && ticks < f->since) return 0;
    if ((f->have&2) && ticks > f->until) return 0;
    if (f->unit[0] && strcmp(tag, f->unit)!=0) return 0;
    return 1;
}

int main(int argc, char** argv){
    filters_t flt; if (parse_args(argc, argv, &flt)!=0) return 1;
    const char* files[ROTATE_FILES+2]; int fc=0; /* include active + archives */
    files[fc++] = LOG_DIR "/current.log";
    static char archs[ROTATE_FILES][64];
    for (int i=0;i<ROTATE_FILES;i++){ snprintf(archs[i],sizeof(archs[i]), LOG_DIR "/archive.%d.log", i); files[fc++] = archs[i]; }
    char line[1024];
    for (int i=0;i<fc;i++){
        FILE* f = fopen(files[i], "rb"); if(!f) continue;
        while (fgets(line, sizeof(line), f)){
            if (match_line(line, &flt)) fputs(line, stdout);
        }
        fclose(f);
    }
    return 0;
}
