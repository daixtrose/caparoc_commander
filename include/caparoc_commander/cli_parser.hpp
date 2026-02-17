#ifndef DEMO_CLI_PARSER_HPP
#define DEMO_CLI_PARSER_HPP

#include <list>
#include <string>
#include <vector>

namespace cli  {

enum class CommandLineAction {
    NONE,
    LIST_REGISTERS,
    REGISTER_INFO,
    SEARCH_REGISTERS,
    READ_UINT16,
    READ_UINT32,
    READ_STRING32,
    WRITE_UINT16,
    WRITE_UINT32,
    RESET_APPLICATION_PARAMS_POWER_AND_CB,
    GLOBAL_CHANNEL_ERROR_RESET_ALL_CB,
    ERROR_COUNTER_RESET_ALL_CB,
    RESET_APPLICATION_PARAMS_QUINT,
    GET_PRODUCT_NAME_POWER_MODULE,
    GET_PRODUCT_NAME_MODULE,
    GET_PRODUCT_NAME_QUINT,
    GET_NUM_CONNECTED_MODULES,
    GET_NOMINAL_CURRENT,
    SET_NOMINAL_CURRENT,
    UNLOCK_NOMINAL_CURRENT,
    PRINT_DEVICE_INFO,
    GET_SYSTEM_STATUS,
    GET_CHANNEL_STATUS,
    GET_LOAD_CURRENT,
    CONTROL_CHANNEL,
    READ_COIL,
    WRITE_COIL
};

struct Uint16Args {
    std::string address;
    std::string value;
};

struct Uint32Args {
    std::string address;
    std::string value;
};

struct NominalCurrentArgs {
    std::string module_number;
    std::string channel_number;
};

struct NominalCurrentSetArgs {
    std::string module_number;
    std::string channel_number;
    std::string value;
};

struct NominalCurrentLockArgs {
    std::string module_number;
    std::string channel_number;
};

struct ChannelControlArgs {
    std::string module_number;
    std::string channel_number;
    std::string state;  // "on" or "off"
};

struct CoilArgs {
    std::string address;
};

struct CoilWriteArgs {
    std::string address;
    std::string state;  // "on", "off", "true", "false", "1", "0"
};

struct CommandLineOptions {
    std::string ip_address; 
    int port;
    int timeout_seconds;

    std::list<CommandLineAction> actions;

    std::string register_info_address;  
    std::string search_filter;

    std::string read_uint16_address;
    std::string read_uint32_address;
    std::string read_string32_address;
    std::vector<Uint16Args> write_uint16_args;
    std::vector<Uint32Args> write_uint32_args;

    std::vector<int> product_module_numbers;
    std::vector<NominalCurrentArgs> get_nominal_current_args;
    std::vector<NominalCurrentSetArgs> set_nominal_current_args;
    std::vector<NominalCurrentLockArgs> unlock_nominal_current_args;
    std::vector<NominalCurrentArgs> get_channel_status_args;
    std::vector<NominalCurrentArgs> get_load_current_args;
    std::vector<ChannelControlArgs> control_channel_args;
    std::vector<CoilArgs> read_coil_args;
    std::vector<CoilWriteArgs> write_coil_args;
    bool debug = false;
}; 

CommandLineOptions parse_command_line(int argc, char* argv[]);
std::string dump_command_line_options(const CommandLineOptions& options);
    
} // namespace cli

#endif  // DEMO_CLI_PARSER_HPP
