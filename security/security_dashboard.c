/*
 * LimitlessOS Security Dashboard Implementation
 * Aggregates vulnerability, patch, compliance, and audit status for enterprise monitoring
 */

#include "security_dashboard.h"
#include "vuln_scanner.h"
#include <stdio.h>

void security_dashboard_show(void) {
    printf("==== LimitlessOS Security Dashboard ====");
    vuln_entry_t vulns[32];
    int n = vuln_scanner_report(vulns, 32);
    printf("\nVulnerabilities: %d\n", n);
    for (int i = 0; i < n; i++) {
        printf("- [%s] %s\n", vulns[i].id, vulns[i].description);
    }
    // TODO: Show patch status, compliance, audit logs
}
