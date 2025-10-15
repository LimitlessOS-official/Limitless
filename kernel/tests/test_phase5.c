#include "kernel.h"
#include "enterprise_phase5.h"

int test_phase5_basic(void){
    if(enterprise_phase5_init_all()!=STATUS_OK) return -1;
    enterprise_phase5_metrics_t m; 
    if(enterprise_phase5_get_metrics(&m)!=STATUS_OK) return -2;
    /* Ensure counters start at zero */
    if(m.firewall_rules!=0) return -3;
    return 0;
}
