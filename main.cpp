#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include "simdjson.h"
#include <string>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

void on_open(websocketpp::connection_hdl hdl, client* c) {
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

void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    std::cout << "Received message: " << msg->get_payload() << std::endl;

    simdjson::dom::parser parser;

    simdjson::padded_string padded_json_string(msg->get_payload());

    for(simdjson::dom::object obj : parser.parse(padded_json_string)){
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

void on_fail(websocketpp::connection_hdl hdl) {
    std::cout << "WebSocket connection failed!" << std::endl;
}

void on_close(websocketpp::connection_hdl hdl) {
    std::cout << "WebSocket connection closed!" << std::endl;
}

context_ptr on_tls_init(const char * hostname, websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cout << "TLS Initialization Error: " << e.what() << std::endl;
    }

    return ctx;
}

int main(int argc, char* argv[]) {
    client c;
    std::string hostname = "stream.data.alpaca.markets/v2/test";
    std::string uri = "wss://" + hostname;

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
            return 0;
        }
        c.connect(con);
       
        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "WebSocket Exception: " << e.what() << std::endl;
    }
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <signal.h>
// #include <libwebsockets.h>
// #include <unistd.h>
// #include "simdjson.h"
// #include <getopt.h>

// // Global variable to check if the program is interrupted (e.g., by Ctrl+C)
// static int interrupted = 0;

// // WebSocket callback function for Alpaca’s API
// static int callback_alpaca(
// struct lws *wsi,
// enum lws_callback_reasons reason,
// void *user,
// void *in,
// size_t len) {

//   switch (reason) {
//     // Connection established
//     case LWS_CALLBACK_CLIENT_ESTABLISHED: {
//       puts("Connected to Alpaca WebSocket server.");

//       // Get the environment variables for the API key and secret
//       const char *apca_api_key_id = "PKV7EJYKTZ61PJFW19I0";
//       const char *apca_api_secret_key = "OrN7TeuRsE0TtuyjywZoiPbq77V5SdFfUftcxaGM";

//       // Create the authentication JSON message
//       auto auth_message_json = R"(
//         {"action": "auth", "key": "PKV7EJYKTZ61PJFW19I0", "secret": "OrN7TeuRsE0TtuyjywZoiPbq77V5SdFfUftcxaGM"})"_padded;

//     //   cJSON *auth_message_json = cJSON_CreateObject();
//     //   cJSON_AddStringToObject(auth_message_json, "action", "auth");
//     //   cJSON_AddStringToObject(auth_message_json, "key", apca_api_key_id);
//     //   cJSON_AddStringToObject(auth_message_json, "secret", apca_api_secret_key);
//       char *auth_message = auth_message_json.data();

//     //   if (!auth_message_json) {
//     //     fprintf(stderr, "Error creating cJSON object for auth_message_json.\n");
//     //     return -1;
//     //   }

//       // Send the authentication message
//       unsigned char buf[LWS_PRE + strlen(auth_message)];
//       memcpy(&buf[LWS_PRE], auth_message, strlen(auth_message));
//       lws_write(wsi, &buf[LWS_PRE], strlen(auth_message), LWS_WRITE_TEXT);

//       // Create the subscription JSON message
//       auto subscription_message_json = R"(
//         {"action":"subscribe","trades":["FAKEPACA"],"quotes":["FAKEPACA"]})"_padded;

//     //   cJSON *subscription_message_json = cJSON_CreateObject();
//     //   cJSON_AddStringToObject(subscription_message_json, "action", "subscribe");
//     //   if (user) {
//     //     cJSON *params = (cJSON *)user;
//     //     cJSON *trades = cJSON_GetObjectItem(params, "trades");
//     //     cJSON *quotes = cJSON_GetObjectItem(params, "quotes");
//     //     cJSON *bars = cJSON_GetObjectItem(params, "bars");

//     //     if (trades && cJSON_GetArraySize(trades) > 0) {
//     //       cJSON_AddItemToObject(subscription_message_json, "trades", cJSON_Duplicate(trades, 1));
//     //     }
//     //     if (quotes && cJSON_GetArraySize(quotes) > 0) {
//     //       cJSON_AddItemToObject(subscription_message_json, "quotes", cJSON_Duplicate(quotes, 1));
//     //     }
//     //     if (bars && cJSON_GetArraySize(bars) > 0) {
//     //       cJSON_AddItemToObject(subscription_message_json, "bars", cJSON_Duplicate(bars, 1));
//     //     }
//     //   }

//     //   // Convert the subscription_message_json object to a string
//       char *subscription_message = subscription_message_json.data();

//       // Print the subscription message
//       printf("Sending subscription message: %s\n", subscription_message);

//       // Send the subscription message
//       unsigned char sub_buf[LWS_PRE + strlen(subscription_message)];
//       memcpy(&sub_buf[LWS_PRE], subscription_message, strlen(subscription_message));
//       lws_write(wsi, &sub_buf[LWS_PRE], strlen(subscription_message), LWS_WRITE_TEXT);

//       // Free the allocated JSON objects and strings
//     //   cJSON_Delete(auth_message_json);
//     //   cJSON_Delete(subscription_message_json);
//       free(auth_message);
//       free(subscription_message);

//       break;
//     }
//     // Data received
//     case LWS_CALLBACK_CLIENT_RECEIVE: {
//       printf("Received data: %s\n", (char *)in);
//       break;
//     }
//     // Connection closed
//     case LWS_CALLBACK_CLIENT_CLOSED: {
//       puts("Connection closed.");
//       interrupted = 1;
//       break;
//     }
//     default:
//       break;
//   }
//   return 0;
// }

// // WebSocket protocols
// static struct lws_protocols protocols[] = {
// {"alpaca", callback_alpaca, 0, 0},
// {NULL, NULL, 0, 0}};

// // Signal handler for SIGINT (e.g., Ctrl+C)
// static void sigint_handler(int sig) {
// interrupted = 1;
// }

// // Function to convert a string to uppercase
// void to_upper(char *str) {
//   for (int i = 0; str[i]; i++) {
//   str[i] = toupper((unsigned char) str[i]);
//   }
// }

// // Function to print the help message with usage instructions
// void print_help(const char program_name) {
//   fprintf(stderr, "Usage: %s [-t trades] [-q quotes] [-b bars]\n", program_name);
//   fprintf(stderr, "\n");
//   fprintf(stderr, "Options:\n");
//   fprintf(stderr, " -t trades : Comma-separated list of trade symbols, or "" for all trades (with quotes).\n");
//   fprintf(stderr, " -q quotes : Comma-separated list of quote symbols, or "" for all quotes (with quotes).\n");
//   fprintf(stderr, " -b bars : Comma-separated list of bar symbols, or "" for all bars (with quotes).\n");
//   fprintf(stderr, " -s sip : Choose the data source. Allowed values are ‘sip’ (default) or ‘iex’.\n");
//   fprintf(stderr, "\n");
// }

// int main(int argc, char **argv) {
//   int opt;

//   // Default WebSocket path for SIP data source
//   const char *path = "/v2/test";

//   // Set the SIGINT signal handler
//   signal(SIGINT, sigint_handler);

//   // Create a WebSocket context
//   struct lws_context_creation_info info;
//   memset(&info, 0, sizeof(info));
//   info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
//   info.port = CONTEXT_PORT_NO_LISTEN;
//   info.protocols = protocols;
//   info.fd_limit_per_thread = 1;

//   struct lws_context *context = lws_create_context(&info);
//   if (!context) {
//   fprintf(stderr, "Error creating WebSocket context.\n");
//   return -1;
//   }

//   // Set up the connection information
//   struct lws_client_connect_info ccinfo;
//   memset(&ccinfo, 0, sizeof(ccinfo));
//   ccinfo.context = context;
//   ccinfo.address = "stream.data.alpaca.markets";
//   ccinfo.port = 443;
//   ccinfo.path = path;
//   ccinfo.host = ccinfo.address;
//   ccinfo.origin = ccinfo.address;
//   ccinfo.protocol = "alpaca";
//   ccinfo.ssl_connection = LCCSCF_USE_SSL;
// //   ccinfo.userdata = params;

//   // Connect to the WebSocket server
//   struct lws *wsi = lws_client_connect_via_info(&ccinfo);
//   if (!wsi) {
//     fprintf(stderr, "Error connecting to WebSocket server.\n");
//     lws_context_destroy(context);
//     return -1;
//   }

//   // Main event loop: process WebSocket events until interrupted
//   while (!interrupted) {
//     lws_service(context, 1000);
//   }

//   // Clean up: destroy the WebSocket context and delete the cJSON object
//   lws_context_destroy(context);

//   // Exit the program
//   return 0;
// }