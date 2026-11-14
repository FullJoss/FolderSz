#include "threadManager.h"
#include "feature.h"

static unsigned __stdcall Worker_MainTask(void* arg);

void CreateWorker(ThrdWorker* wrk) {
    wrk->arg = NULL;
    wrk->isBusy = FALSE;
    wrk->hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    wrk->task = THRD_TASK_IDLE;

    HANDLE h = (HANDLE)_beginthreadex(NULL, 0, Worker_MainTask, wrk, 0, NULL);
    if (h) CloseHandle(h);
    
    return wrk;
}

static unsigned __stdcall Worker_MainTask(void* arg) {
    ThrdWorker* thrd_wrk = (ThrdWorker*)arg;

    while (thrd_wrk->task != THRD_TASK_EXIT) {
        WaitForSingleObject(thrd_wrk->hEvent, INFINITE);

        switch (thrd_wrk->task) {
        case THRD_TASK_SCAN:
            AsyncScanFolders(thrd_wrk->arg);
        }

        thrd_wrk->isBusy = FALSE;
    }

    _endthreadex(0);
    return 0;
}