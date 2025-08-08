#include "MDConnector.h"

simdjson::dom::parser MDConnector::parser;
MDProcessor*  MDConnector::mMDProcessor;
SymbolIDManager*  MDConnector::mSymIDManager;

void MDConnector::on_open(websocketpp::connection_hdl hdl, client* c) {
    std::cout << "WebSocket connection opened!" << std::endl;
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c->get_con_from_hdl(hdl, ec);
 
    if (ec) {
        std::cout << "Failed to get connection pointer: " << ec.message() << std::endl;
        return;
    }
    simdjson::dom::element doc = MDConnector::parser.load("/home/git_repos/Confs/MD_auth.json");
    std::stringstream ss;
    ss << R"({"action": "auth", "key": ")" << doc["key"].get_string() << R"(", "secret": ")" << doc["secret"].get_string() << R"("})"_padded;
    auto auth_message_json = ss.str();
    char *auth_message = auth_message_json.data();
    c->send(con, auth_message, websocketpp::frame::opcode::text);

    std::stringstream sub_ss;
    sub_ss << R"({"action":"subscribe","trades":[)";

    int size_of_map = mSymIDManager->string_to_id.size();
    
    int symbol_number = 1;
    for (const auto & [ key, value ] : mSymIDManager->string_to_id){
        sub_ss << R"(")" << key << R"(")";
        if(symbol_number != size_of_map) sub_ss << R"(,)";
        symbol_number++;
    }

    sub_ss << R"(],"quotes":[)";
    symbol_number = 1;
    for(const auto& pair : mSymIDManager->string_to_id){
        sub_ss << R"(")" << pair.first << R"(")";
        if(symbol_number != size_of_map) sub_ss << R"(,)";
        symbol_number++;
    }

    sub_ss << R"(]})"_padded;

    auto subscription_message_json = sub_ss.str();

    char *subscription_message = subscription_message_json.data();
    c->send(con, subscription_message, websocketpp::frame::opcode::text);
}

void MDConnector::on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    // std::cout << "Received message: " << msg->get_payload() << std::endl;

    simdjson::padded_string padded_json_string(msg->get_payload());

    for(simdjson::dom::object obj : MDConnector::parser.parse(padded_json_string)){
        for(const auto& key_value : obj) {
            if(key_value.key == "T"){
                std::string_view value = obj["T"].get_string();
                if(value == "t"){
                    // std::cout << "Received trade" << std::endl;
                    mMDProcessor->process_trade(obj);
                    continue;
                }
                else if(value == "q"){
                    // std::cout << "Received quote" << std::endl;
                    mMDProcessor->process_quote(obj);
                    continue;
                }
                else{
                    continue;
                }
            }
        }
    }
}

void MDConnector::on_fail(websocketpp::connection_hdl hdl) {
    std::cout << "WebSocket connection failed!" << std::endl;
}

void MDConnector::on_close(websocketpp::connection_hdl hdl) {
    std::cout << "WebSocket connection closed!" << std::endl;
}

void MDConnector::on_init() {

    mShmemManager = ShmemManager::getInstance();
    mSymIDManager = SymbolIDManager::getInstance();
    mMDProcessor = MDProcessor::getInstance();

    mShmemManager->startUp();
    mSymIDManager->startUp();
    mMDProcessor->startUp();
    
    try {
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);
        c.init_asio();
      
        c.set_message_handler(&on_message);
        c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));
       
        c.set_open_handler(bind(&on_open, ::_1, &c));
        c.set_fail_handler(bind(&on_fail, ::_1));
        c.set_close_handler(bind(&on_close, ::_1));
        c.set_error_channels(websocketpp::log::elevel::all);  // Enable detailed error logging
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "Could not create connection because: " << ec.message() << std::endl;
            return;
        }
        c.connect(con);
       
        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "WebSocket Exception: " << e.what() << std::endl;
    }
}