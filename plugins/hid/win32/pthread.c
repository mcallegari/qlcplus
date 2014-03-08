#include <windows.h>
#include "pthread.h"

void pthread_create(pthread_t * thread_pointer, void * null, void * (*function) (void*), void * pointer) {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) function, pointer, 0, NULL);
}

