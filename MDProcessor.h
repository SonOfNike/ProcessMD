#pragma once

#include "../Utils/MDShmem.h"
#include "../Utils/MDupdate.h"
#include "simdjson.h"
#include "ShmemManager.h"

class MDProcessor {
private:

    ShmemManager* mShmemManager;

    MDupdate    currentMD;

    static MDProcessor* uniqueInstance;
    MDProcessor(){;}

public:
    static MDProcessor* getInstance();
    void startUp();
    void shutDown();
    void process_quote(simdjson::dom::object obj);
    void process_trade(simdjson::dom::object obj);
};