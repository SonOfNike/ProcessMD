#include "MDConnector.h"

simdjson::dom::parser MDConnector::parser;

void MDConnector::on_open(websocketpp::connection_hdl hdl, client* c) {
    std::cout << "WebSocket connection opened!" << std::endl;
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c->get_con_from_hdl(hdl, ec);
 
    if (ec) {
        std::cout << "Failed to get connection pointer: " << ec.message() << std::endl;
        return;
    }
    auto auth_message_json = R"({"action": "auth", "key": "PKV7EJYKTZ61PJFW19I0", "secret": "OrN7TeuRsE0TtuyjywZoiPbq77V5SdFfUftcxaGM"})"_padded;
    char *auth_message = auth_message_json.data();
    c->send(con, auth_message, websocketpp::frame::opcode::text);

    auto subscription_message_json = R"({"action":"subscribe","trades":["FAKEPACA"],"quotes":["FAKEPACA"]})"_padded;
    char *subscription_message = subscription_message_json.data();
    c->send(con, subscription_message, websocketpp::frame::opcode::text);
}

void MDConnector::on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    std::cout << "Received message: " << msg->get_payload() << std::endl;

    simdjson::padded_string padded_json_string(msg->get_payload());

    for(simdjson::dom::object obj : MDConnector::parser.parse(padded_json_string)){
        bool is_MD = false;
        for(const auto& key_value : obj) {
            if(key_value.key == "T"){
                std::string_view value = obj["T"].get_string();
                if(value == "t"){
                    is_MD = true;
                    std::cout << "Received trade" << std::endl;
                    continue;
                }
                else if(value == "q"){
                    is_MD = true;
                    std::cout << "Received quote" << std::endl;
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