#pragma once

#include "../Utils/MDShmem.h"
#include "../Utils/MDupdate.h"
#include "../Utils/simdjson/simdjson.h"
#include "ShmemManager.h"
#include "../Utils/SymbolIDManager.h"

class MDProcessor {
private:

    ShmemManager* mShmemManager;
    SymbolIDManager* mSymIDManager;

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