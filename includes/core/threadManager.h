#pragma once

#include "pch.h"
#include "type.h"

static inline void WakeWorker(ThrdWorker* wrk, ThrdTask task, void* arg) {
    wrk->task = task;
    wrk->arg = arg;
    wrk->isBusy = TRUE;
    SetEvent(wrk->hEvent);
}

void CreateWorker(ThrdWorker* wrk);