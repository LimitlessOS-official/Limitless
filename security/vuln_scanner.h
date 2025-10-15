#ifndef LIMITLESS_VULN_SCANNER_H
#define LIMITLESS_VULN_SCANNER_H

#define VULN_ID_MAX 32
#define VULN_DESC_MAX 128

typedef struct {
    char id[VULN_ID_MAX];
    char description[VULN_DESC_MAX];
} vuln_entry_t;

int vuln_scanner_scan_system(void);
int vuln_scanner_report(vuln_entry_t* out, int max);
int vuln_scanner_patch(const char* vuln_id);
int vuln_scanner_add(const char* vuln_id, const char* description);

#endif // LIMITLESS_VULN_SCANNER_H
