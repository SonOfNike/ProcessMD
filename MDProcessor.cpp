#include "MDProcessor.h"
#include "../Utils/Time_functions.h"
// #include <iostream>
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>

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

void MDProcessor::process_quote(simdjson::dom::object _obj){
    currentMD.m_type = md_type::QUOTE;
    currentMD.m_symbolId = mSymIDManager->getID(_obj["S"].get_string());
    currentMD.m_bid_price = Price(_obj["bp"].get_double() * 1000);
    currentMD.m_ask_price = Price(_obj["ap"].get_double() * 1000);
    currentMD.m_bid_quant = Shares(_obj["bs"].get_int64() * 100);
    currentMD.m_ask_quant = Shares(_obj["as"].get_int64() * 100);

    //Timestamp conversion
    currentMD.m_timestamp = parse_timestring(_obj["t"].get_string());
    mShmemManager->write_MD(currentMD);
}
    
void MDProcessor::process_trade(simdjson::dom::object _obj){
    currentMD.m_type = md_type::PRINT;
    currentMD.m_symbolId = mSymIDManager->getID(_obj["S"].get_string());
    currentMD.m_bid_price = Price(_obj["p"].get_double() * 1000);
    currentMD.m_bid_quant = Shares(_obj["s"].get_int64() * 100);

    //Timestamp conversion
    currentMD.m_timestamp = parse_timestring(_obj["t"].get_string());
    mShmemManager->write_MD(currentMD);
}
