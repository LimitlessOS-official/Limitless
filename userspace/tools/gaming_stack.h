#ifndef LIMITLESS_GAMING_STACK_H
#define LIMITLESS_GAMING_STACK_H

int gaming_launch_game(const char* game_name, const char* args);
int gaming_list_games(char* out, int max);
int gaming_monitor_performance(void);

#endif // LIMITLESS_GAMING_STACK_H
