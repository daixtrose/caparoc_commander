#include "caparoc_commander/cli_parser.hpp"
#include "CLI/CLI.hpp"

#include <format>

namespace cli
{
    namespace
    {
        std::string action_to_string(CommandLineAction action)
        {
            switch (action)
            {
            case CommandLineAction::NONE:
                return "NONE";
            case CommandLineAction::LIST_REGISTERS:
                return "LIST_REGISTERS";
            case CommandLineAction::REGISTER_INFO:
                return "REGISTER_INFO";
            case CommandLineAction::SEARCH_REGISTERS:
                return "SEARCH_REGISTERS";
            case CommandLineAction::READ_UINT16:
                return "READ_UINT16";
            case CommandLineAction::READ_UINT32:
                return "READ_UINT32";
            case CommandLineAction::READ_STRING32:
                return "READ_STRING32";
            case CommandLineAction::WRITE_UINT16:
                return "WRITE_UINT16";
            case CommandLineAction::WRITE_UINT32:
                return "WRITE_UINT32";
            case CommandLineAction::RESET_APPLICATION_PARAMS_POWER_AND_CB:
                return "RESET_APPLICATION_PARAMS_POWER_AND_CB";
            case CommandLineAction::GLOBAL_CHANNEL_ERROR_RESET_ALL_CB:
                return "GLOBAL_CHANNEL_ERROR_RESET_ALL_CB";
            case CommandLineAction::ERROR_COUNTER_RESET_ALL_CB:
                return "ERROR_COUNTER_RESET_ALL_CB";
            case CommandLineAction::RESET_APPLICATION_PARAMS_QUINT:
                return "RESET_APPLICATION_PARAMS_QUINT";
            case CommandLineAction::GET_PRODUCT_NAME_POWER_MODULE:
                return "GET_PRODUCT_NAME_POWER_MODULE";
            case CommandLineAction::GET_PRODUCT_NAME_MODULE:
                return "GET_PRODUCT_NAME_MODULE";
            case CommandLineAction::GET_PRODUCT_NAME_QUINT:
                return "GET_PRODUCT_NAME_QUINT";
            case CommandLineAction::GET_NUM_CONNECTED_MODULES:
                return "GET_NUM_CONNECTED_MODULES";
            case CommandLineAction::GET_NOMINAL_CURRENT:
                return "GET_NOMINAL_CURRENT";
            case CommandLineAction::SET_NOMINAL_CURRENT:
                return "SET_NOMINAL_CURRENT";
            case CommandLineAction::UNLOCK_NOMINAL_CURRENT:
                return "UNLOCK_NOMINAL_CURRENT";
            case CommandLineAction::PRINT_DEVICE_INFO:
                return "PRINT_DEVICE_INFO";
            case CommandLineAction::GET_SYSTEM_STATUS:
                return "GET_SYSTEM_STATUS";
            case CommandLineAction::GET_CHANNEL_STATUS:
                return "GET_CHANNEL_STATUS";
            case CommandLineAction::GET_LOAD_CURRENT:
                return "GET_LOAD_CURRENT";
            case CommandLineAction::CONTROL_CHANNEL:
                return "CONTROL_CHANNEL";
            case CommandLineAction::READ_COIL:
                return "READ_COIL";
            case CommandLineAction::WRITE_COIL:
                return "WRITE_COIL";
            }

            return "UNKNOWN";
        }
    }

    CommandLineOptions parse_command_line(int argc, char *argv[])
    {
        // Default values
        CommandLineOptions options;

        CLI::App app{"Caparoc Commander"};
        app.set_help_flag("-h,--help", "Show all available options");

        app.add_option("-i,--ip", options.ip_address, "IP address of the CAPAROC device")
            ->default_val("192.168.1.2");
        app.add_option("-p,--port", options.port, "Modbus TCP port")
            ->default_val(502);
        app.add_flag_callback("-l,--list", [&options]()
                              { options.actions.push_back(CommandLineAction::LIST_REGISTERS); }, "List all registers");

        auto register_option = app.add_option("-r,--register", options.register_info_address,
                                              "Get info about a specific register (e.g. 0x0010)");
        auto search_option = app.add_option("-s,--search", options.search_filter,
                                            "Search for registers by name (case-insensitive substring match)");
        auto read_uint16_option = app.add_option("--read-uint16", options.read_uint16_address,
                                                 "Read UINT16 register (e.g. 0x0010)");
        auto read_uint32_option = app.add_option("--read-uint32", options.read_uint32_address,
                                                 "Read UINT32 register (e.g. 0x0100)");
        auto read_string32_option = app.add_option("--read-string32", options.read_string32_address,
                                                   "Read STRING32 register (e.g. 0x1000)");
        auto product_name_module_option = app.add_option("--product-name-module", options.product_module_numbers,
                                                         "Get product name for a specific module (1-16)");

        std::vector<std::string> write_uint16_args_raw;
        std::vector<std::string> write_uint32_args_raw;
        std::vector<std::string> get_nominal_current_args_raw;
        std::vector<std::string> set_nominal_current_args_raw;
        std::vector<std::string> unlock_nominal_current_args_raw;
        std::vector<std::string> read_coil_args_raw;
        std::vector<std::string> write_coil_args_raw;

        auto write_uint16_option = app.add_option("--write-uint16", write_uint16_args_raw,
                                                  "Write UINT16 register (address value)")
                                       ->expected(2);
        auto write_uint32_option = app.add_option("--write-uint32", write_uint32_args_raw,
                                                  "Write UINT32 register (address value)")
                                       ->expected(2);
        auto get_nominal_current_option = app.add_option("--get-nominal-current", get_nominal_current_args_raw,
                                                         "Get nominal current (module_number channel_number)")
                                               ->expected(2);
        auto set_nominal_current_option = app.add_option("--set-nominal-current", set_nominal_current_args_raw,
                                                         "Set nominal current (module_number channel_number value)")
                                               ->expected(3);
        auto unlock_nominal_current_option = app.add_option("--unlock-nominal-current", unlock_nominal_current_args_raw,
                                    "Unlock nominal current parametrization (module_number channel_number)")
                              ->expected(2);
        
        auto read_coil_option = app.add_option("--read-coil", read_coil_args_raw,
                                               "Read coil status (address)")
                                    ->expected(1);
        auto write_coil_option = app.add_option("--write-coil", write_coil_args_raw,
                                                "Write coil (address state) - state can be on|off|true|false|1|0")
                                     ->expected(2);

        app.add_flag_callback("--reset-application-params-power-and-cb", [&options]()
                              { options.actions.push_back(CommandLineAction::RESET_APPLICATION_PARAMS_POWER_AND_CB); }, "Reset application parameters for Power Module and Circuit Breakers");
        app.add_flag_callback("--global-channel-error-reset-all-cb", [&options]()
                              { options.actions.push_back(CommandLineAction::GLOBAL_CHANNEL_ERROR_RESET_ALL_CB); }, "Global channel error reset for all Circuit Breakers");
        app.add_flag_callback("--error-counter-reset-all-cb", [&options]()
                              { options.actions.push_back(CommandLineAction::ERROR_COUNTER_RESET_ALL_CB); }, "Reset error counters for all Circuit Breakers");
        app.add_flag_callback("--reset-application-params-quint", [&options]()
                              { options.actions.push_back(CommandLineAction::RESET_APPLICATION_PARAMS_QUINT); }, "Reset application parameters for QUINT Power Supply");
        app.add_flag_callback("--product-name-power-module", [&options]()
                              { options.actions.push_back(CommandLineAction::GET_PRODUCT_NAME_POWER_MODULE); }, "Get product name for Power Module");
        app.add_flag_callback("--product-name-quint", [&options]()
                              { options.actions.push_back(CommandLineAction::GET_PRODUCT_NAME_QUINT); }, "Get product name for QUINT Power Supply");
        app.add_flag_callback("--num-connected-modules", [&options]()
                              { options.actions.push_back(CommandLineAction::GET_NUM_CONNECTED_MODULES); }, "Get number of currently connected modules");
        app.add_flag_callback("--print-device-info", [&options]()
                              { options.actions.push_back(CommandLineAction::PRINT_DEVICE_INFO); }, "Print device information (modules, product names, channels)");
        app.add_flag_callback("--get-system-status", [&options]()
                              { options.actions.push_back(CommandLineAction::GET_SYSTEM_STATUS); }, "Get system-level status (voltage, current, temperature)");
        
        std::vector<std::string> get_channel_status_args_raw;
        auto get_channel_status_option = app.add_option("--get-channel-status", get_channel_status_args_raw,
                                                        "Get status for specific channel (module_number channel_number)")
                                              ->expected(2);
        
        std::vector<std::string> get_load_current_args_raw;
        auto get_load_current_option = app.add_option("--get-load-current", get_load_current_args_raw,
                                                      "Get actual load current for channel (module_number channel_number)")
                                             ->expected(2);
        
        std::vector<std::string> control_channel_args_raw;
        auto control_channel_option = app.add_option("--control-channel", control_channel_args_raw,
                                                     "Control channel on/off (module_number channel_number on|off)")
                                           ->expected(3);
        
        app.add_flag("-d,--debug", options.debug, "Enable debug output")
            ->default_val(false);
        app.add_flag("-t,--timeout", options.timeout_seconds, "Connection timeout in seconds")
            ->default_val(3);

        try
        {
            app.parse(argc, argv);
        }
        catch (const CLI::ParseError &e)
        {
            app.exit(e);
            exit(e.get_exit_code());
        }

        if (register_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::REGISTER_INFO);
        }
        if (search_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::SEARCH_REGISTERS);
        }
        if (read_uint16_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::READ_UINT16);
        }
        if (read_uint32_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::READ_UINT32);
        }
        if (read_string32_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::READ_STRING32);
        }
        if (write_uint16_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::WRITE_UINT16);
            auto results = write_uint16_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.write_uint16_args.push_back({results[i], results[i + 1]});
            }
        }
        if (write_uint32_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::WRITE_UINT32);
            auto results = write_uint32_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.write_uint32_args.push_back({results[i], results[i + 1]});
            }
        }
        if (product_name_module_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::GET_PRODUCT_NAME_MODULE);
        }
        if (get_nominal_current_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::GET_NOMINAL_CURRENT);
            auto results = get_nominal_current_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.get_nominal_current_args.push_back({results[i], results[i + 1]});
            }
        }
        if (set_nominal_current_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::SET_NOMINAL_CURRENT);
            auto results = set_nominal_current_option->results();
            for (std::size_t i = 0; i + 2 < results.size(); i += 3)
            {
                options.set_nominal_current_args.push_back({results[i], results[i + 1], results[i + 2]});
            }
        }
        if (unlock_nominal_current_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::UNLOCK_NOMINAL_CURRENT);
            auto results = unlock_nominal_current_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.unlock_nominal_current_args.push_back({results[i], results[i + 1]});
            }
        }
        if (get_channel_status_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::GET_CHANNEL_STATUS);
            auto results = get_channel_status_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.get_channel_status_args.push_back({results[i], results[i + 1]});
            }
        }
        if (get_load_current_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::GET_LOAD_CURRENT);
            auto results = get_load_current_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.get_load_current_args.push_back({results[i], results[i + 1]});
            }
        }
        if (control_channel_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::CONTROL_CHANNEL);
            auto results = control_channel_option->results();
            for (std::size_t i = 0; i + 2 < results.size(); i += 3)
            {
                options.control_channel_args.push_back({results[i], results[i + 1], results[i + 2]});
            }
        }
        if (read_coil_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::READ_COIL);
            auto results = read_coil_option->results();
            for (std::size_t i = 0; i < results.size(); i++)
            {
                options.read_coil_args.push_back({results[i]});
            }
        }
        if (write_coil_option->count() > 0)
        {
            options.actions.push_back(CommandLineAction::WRITE_COIL);
            auto results = write_coil_option->results();
            for (std::size_t i = 0; i + 1 < results.size(); i += 2)
            {
                options.write_coil_args.push_back({results[i], results[i + 1]});
            }
        }
        return options;
    }

    std::string dump_command_line_options(const CommandLineOptions &options)
    {
        std::string output;
        output += std::format("ip_address: {}\n", options.ip_address);
        output += std::format("port: {}\n", options.port);
        output += std::format("timeout_seconds: {}\n", options.timeout_seconds);
        output += "actions:\n";
        if (options.actions.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &action : options.actions)
            {
                output += std::format("  - {}\n", action_to_string(action));
            }
        }

        output += std::format("register_info_address: {}\n", options.register_info_address);
        output += std::format("search_filter: {}\n", options.search_filter);
        output += std::format("read_uint16_address: {}\n", options.read_uint16_address);
        output += std::format("read_uint32_address: {}\n", options.read_uint32_address);
        output += std::format("read_string32_address: {}\n", options.read_string32_address);

        output += "write_uint16_args:\n";
        if (options.write_uint16_args.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &args : options.write_uint16_args)
            {
                output += std::format("  - address: {}, value: {}\n", args.address, args.value);
            }
        }

        output += "write_uint32_args:\n";
        if (options.write_uint32_args.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &args : options.write_uint32_args)
            {
                output += std::format("  - address: {}, value: {}\n", args.address, args.value);
            }
        }

        output += "product_module_numbers:\n";
        if (options.product_module_numbers.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &num : options.product_module_numbers)
            {
                output += std::format("  - {}\n", num);
            }
        }

        output += "get_nominal_current_args:\n";
        if (options.get_nominal_current_args.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &args : options.get_nominal_current_args)
            {
                output += std::format("  - module: {}, channel: {}\n", args.module_number, args.channel_number);
            }
        }

        output += "set_nominal_current_args:\n";
        if (options.set_nominal_current_args.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &args : options.set_nominal_current_args)
            {
                output += std::format("  - module: {}, channel: {}, value: {}\n", args.module_number, args.channel_number, args.value);
            }
        }

        output += "unlock_nominal_current_args:\n";
        if (options.unlock_nominal_current_args.empty())
        {
            output += "  (none)\n";
        }
        else
        {
            for (const auto &args : options.unlock_nominal_current_args)
            {
                output += std::format("  - module: {}, channel: {}\n", args.module_number, args.channel_number);
            }
        }

        return output;
    }

} // namespace cli