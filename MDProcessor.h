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

    const uint64_t NANOS_PER_DAY = 24ULL * 60 * 60 *1000000000ULL;

    MDupdate    currentMD;

    static MDProcessor* uniqueInstance;
    MDProcessor(){;}

public:
    static MDProcessor* getInstance();
    void startUp();
    void shutDown();
    void process_quote(const simdjson::dom::object& obj);
    void process_trade(const simdjson::dom::object& obj);
};