#include "MDProcessor.h"
#include "../Utils/Time_functions.h"
#include "../Utils/math_functions.h"
#include "glog/logging.h"
#include <string>
#include <string_view>
#include <ctime>
#include <cstdint>
#include <iostream>

MDProcessor* MDProcessor::uniqueInstance = nullptr;

MDProcessor* MDProcessor::getInstance(){
    if(uniqueInstance == nullptr){
        uniqueInstance = new MDProcessor();
    }
    return uniqueInstance;
}

void MDProcessor::startUp(){
    mShmemManager = ShmemManager::getInstance();
    mSymIDManager = SymbolIDManager::getInstance();
}

void MDProcessor::shutDown(){
    
}

void MDProcessor::process_quote(const simdjson::dom::object& _obj){
    currentMD.m_type = md_type::QUOTE;
    currentMD.m_symbolId = mSymIDManager->getID(_obj["S"].get_string());
    currentMD.m_bid_price = roundToNearestCent(Price(_obj["bp"].get_double() * DOLLAR));
    currentMD.m_ask_price = roundToNearestCent(Price(_obj["ap"].get_double() * DOLLAR));
    currentMD.m_bid_quant = Shares(_obj["bs"].get_int64() * 100);
    currentMD.m_ask_quant = Shares(_obj["as"].get_int64() * 100);

    //Timestamp conversion
    currentMD.m_timestamp = parse_timestring(_obj["t"].get_string());
    mShmemManager->write_MD(currentMD);

    // timespec ts;
    // clock_gettime(CLOCK_REALTIME, &ts);
    // uint64_t nanos = uint64_t(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;

    // if((nanos % NANOS_PER_DAY) - currentMD.m_timestamp > 400000000)
    //     std::cout << "Latency=" << (nanos % NANOS_PER_DAY) - currentMD.m_timestamp << 
    //     "|CurrentTime=" << currentMD.m_timestamp << "\n"; 
    // DLOG(INFO) << "MD|TYPE=QUOTE|TIMESTAMP=" << currentMD.m_timestamp;
}
    
void MDProcessor::process_trade(const simdjson::dom::object& _obj){
    currentMD.m_type = md_type::PRINT;
    currentMD.m_symbolId = mSymIDManager->getID(_obj["S"].get_string());
    currentMD.m_bid_price = roundToNearestCent(Price(_obj["p"].get_double() * DOLLAR));
    currentMD.m_bid_quant = Shares(_obj["s"].get_int64() * 100);

    //Timestamp conversion
    currentMD.m_timestamp = parse_timestring(_obj["t"].get_string());
    mShmemManager->write_MD(currentMD);

    // timespec ts;
    // clock_gettime(CLOCK_REALTIME, &ts);
    // uint64_t nanos = uint64_t(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;

    // if((nanos % NANOS_PER_DAY) - currentMD.m_timestamp > 400000000)
    //     std::cout << "Latency=" << (nanos % NANOS_PER_DAY) - currentMD.m_timestamp << 
    //     "|CurrentTime=" << currentMD.m_timestamp << "\n"; 
    // DLOG(INFO) << "MD|TYPE=TRADE|TIMESTAMP=" << currentMD.m_timestamp;
}
