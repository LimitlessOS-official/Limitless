#ifndef LIMITLESS_NETWORKING_H
#define LIMITLESS_NETWORKING_H

int networking_enable_vpn(const char* profile);
int networking_configure_firewall(const char* rule);
int networking_show_analytics(void);
int networking_set_route(const char* destination, const char* gateway);

#endif // LIMITLESS_NETWORKING_H
