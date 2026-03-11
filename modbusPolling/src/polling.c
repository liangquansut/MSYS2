// src/polling.c
#include "polling.h"

#ifdef _WIN32
int start_thread(thread_t *t, thread_ret_t (THREAD_CALL *func)(void*), void *arg) {
    *t = CreateThread(NULL, 0, func, arg, 0, NULL);
    return *t ? 0 : -1;
}
void join_thread(thread_t t) {
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}
#else
int start_thread(thread_t *t, thread_ret_t (THREAD_CALL *func)(void*), void *arg) {
    return pthread_create(t, NULL, func, arg);
}
void join_thread(thread_t t) {
    pthread_join(t, NULL);
}
#endif

void stop_flag(poll_config_t *cfg) {
    cfg->running = 0;
}