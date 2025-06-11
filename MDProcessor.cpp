#include "MDProcessor.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

MDProcessor* MDProcessor::uniqueInstance = nullptr;

MDProcessor* MDProcessor::getInstance(){
    if(uniqueInstance == nullptr){
        uniqueInstance = new MDProcessor();
    }
    return uniqueInstance;
}

void MDProcessor::startUp(){
    mShmemManager = ShmemManager::getInstance();
}

void MDProcessor::shutDown(){
    
}

void MDProcessor::process_quote(simdjson::dom::object _obj){

    mShmemManager->write_MD(currentMD);
}
    
void MDProcessor::process_trade(simdjson::dom::object _obj){
    
    mShmemManager->write_MD(currentMD);
}
