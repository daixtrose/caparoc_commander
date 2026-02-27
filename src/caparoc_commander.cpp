#include "caparoc/caparoc.hpp"
#include "libmodbus_cpp/modbus_connection.hpp"
#include "caparoc_commander/cli_parser.hpp"
#include "caparoc_commander/create_modbus_connection.hpp"

#include <format>
#include <print>
#include <stdexcept>
#include <cstdlib>

int main(int argc, char *argv[])
{
    try
    {
        auto options = cli::parse_command_line(argc, argv);

        if (options.debug)
        {
            std::println("========================");
            std::println("CAPAROC Commander");
            std::println("========================");
            std::println("Connecting to {}:{}", options.ip_address, options.port);
            std::println("");

            std::println("Command Line Options:");
            std::println("{}", cli::dump_command_line_options(options));
            std::println("");
        }

        // Create and connect to device
        auto conn = cli::create_modbus_connection(options.ip_address, options.port, options.timeout_seconds);

        if (options.debug)
        {
            std::println("Connected successfully!");
            std::println("");
        }

        // Process all requested actions
        for (const auto &action : options.actions)
        {
            switch (action)
            {
            case cli::CommandLineAction::LIST_REGISTERS:
                std::println("=== All Registers ===");
                std::println("{}", caparoc::list_all_registers());
                break;

            case cli::CommandLineAction::REGISTER_INFO:
                std::println("=== Register Information ===");
                std::println("Address: {}", options.register_info_address);
                // Parse hex address and get info
                try
                {
                    auto addr = std::stoi(options.register_info_address, nullptr, 16);
                    std::println("{}", caparoc::get_register_info(static_cast<uint16_t>(addr)));
                }
                catch (const std::exception &e)
                {
                    std::println("Error parsing address: {}", e.what());
                }
                break;

            case cli::CommandLineAction::SEARCH_REGISTERS:
                std::println("=== Search Results for '{}' ===", options.search_filter);
                {
                    auto results = caparoc::find_registers(options.search_filter);
                    std::println("Found {} registers", results.size());
                    for (const auto &reg : results)
                    {
                        std::string access_str;
                        switch (reg.access)
                        {
                        case caparoc::RegisterAccess::READ_ONLY:
                            access_str = "RO";
                            break;
                        case caparoc::RegisterAccess::WRITE_ONLY:
                            access_str = "WO";
                            break;
                        case caparoc::RegisterAccess::READ_WRITE:
                            access_str = "RW";
                            break;
                        }
                        std::println("  [0x{:04X}] {} | {} - {}", reg.address, access_str, reg.name, reg.description);
                    }
                }
                break;

            case cli::CommandLineAction::READ_UINT16:
                std::println("=== Read UINT16 Register ===");
                std::println("Address: {}", options.read_uint16_address);
                try
                {
                    auto addr = std::stoi(options.read_uint16_address, nullptr, 16);
                    auto val = caparoc::read_uint16(conn, static_cast<uint16_t>(addr));
                    if (val)
                    {
                        std::println("Value: {}", *val);
                    }
                    else
                    {
                        std::println("Failed to read register");
                    }
                }
                catch (const std::exception &e)
                {
                    std::println("Error: {}", e.what());
                }
                break;

            case cli::CommandLineAction::READ_UINT32:
                std::println("=== Read UINT32 Register ===");
                std::println("Address: {}", options.read_uint32_address);
                try
                {
                    auto addr = std::stoi(options.read_uint32_address, nullptr, 16);
                    auto val = caparoc::read_uint32(conn, static_cast<uint16_t>(addr));
                    if (val)
                    {
                        std::println("Value: {}", *val);
                    }
                    else
                    {
                        std::println("Failed to read register");
                    }
                }
                catch (const std::exception &e)
                {
                    std::println("Error: {}", e.what());
                }
                break;

            case cli::CommandLineAction::READ_STRING32:
                std::println("=== Read STRING32 Register ===");
                std::println("Address: {}", options.read_string32_address);
                try
                {
                    auto addr = std::stoi(options.read_string32_address, nullptr, 16);
                    auto val = caparoc::read_string32(conn, static_cast<uint16_t>(addr));
                    if (val)
                    {
                        std::println("Value: \"{}\"", *val);
                    }
                    else
                    {
                        std::println("Failed to read register");
                    }
                }
                catch (const std::exception &e)
                {
                    std::println("Error: {}", e.what());
                }
                break;

            case cli::CommandLineAction::WRITE_UINT16:
                std::println("=== Write UINT16 Registers ===");
                for (const auto &args : options.write_uint16_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 16);
                        auto val = std::stoi(args.value, nullptr, 0);
                        if (caparoc::write_uint16(conn, static_cast<uint16_t>(addr), static_cast<uint16_t>(val)))
                        {
                            std::println("  0x{:04X} = {} (SUCCESS)", addr, val);
                        }
                        else
                        {
                            std::println("  0x{:04X} = {} (FAILED)", addr, val);
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("  Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::WRITE_UINT32:
                std::println("=== Write UINT32 Registers ===");
                for (const auto &args : options.write_uint32_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 16);
                        auto val = std::stoll(args.value, nullptr, 0);
                        if (caparoc::write_uint32(conn, static_cast<uint16_t>(addr), static_cast<uint32_t>(val)))
                        {
                            std::println("  0x{:04X} = {} (SUCCESS)", addr, val);
                        }
                        else
                        {
                            std::println("  0x{:04X} = {} (FAILED)", addr, val);
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("  Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::RESET_APPLICATION_PARAMS_POWER_AND_CB:
                std::println("=== Reset Application Parameters (Power Module and Circuit Breakers) ===");
                if (caparoc::reset_application_params_power_and_cb(conn))
                {
                    std::println("SUCCESS");
                }
                else
                {
                    std::println("FAILED");
                }
                break;

            case cli::CommandLineAction::GLOBAL_CHANNEL_ERROR_RESET_ALL_CB:
                std::println("=== Global Channel Error Reset (All Circuit Breakers) ===");
                if (caparoc::global_channel_error_reset_all_cb(conn))
                {
                    std::println("SUCCESS");
                }
                else
                {
                    std::println("FAILED");
                }
                break;

            case cli::CommandLineAction::ERROR_COUNTER_RESET_ALL_CB:
                std::println("=== Error Counter Reset (All Circuit Breakers) ===");
                if (caparoc::error_counter_reset_all_cb(conn))
                {
                    std::println("SUCCESS");
                }
                else
                {
                    std::println("FAILED");
                }
                break;

            case cli::CommandLineAction::RESET_APPLICATION_PARAMS_QUINT:
                std::println("=== Reset Application Parameters (QUINT Power Supply) ===");
                if (caparoc::reset_application_params_quint(conn))
                {
                    std::println("SUCCESS");
                }
                else
                {
                    std::println("FAILED");
                }
                break;

            case cli::CommandLineAction::GET_PRODUCT_NAME_POWER_MODULE:
                std::println("=== Product Name (Power Module) ===");
                {
                    auto name = caparoc::get_product_name_power_module(conn);
                    if (name)
                    {
                        std::println("Name: {}", *name);
                    }
                    else
                    {
                        std::println("Failed to read product name");
                    }
                }
                break;

            case cli::CommandLineAction::GET_PRODUCT_NAME_MODULE:
                for (const auto &module_num : options.product_module_numbers)
                {
                    std::println("=== Product Name (Module {}) ===", module_num);
                    auto name = caparoc::get_product_name_module(conn, static_cast<uint8_t>(module_num));
                    if (name)
                    {
                        std::println("Name: {}", *name);
                    }
                    else
                    {
                        std::println("Failed to read product name (module might not be installed)");
                    }
                }
                break;

            case cli::CommandLineAction::GET_PRODUCT_NAME_QUINT:
                std::println("=== Product Name (QUINT Power Supply) ===");
                {
                    auto name = caparoc::get_product_name_quint(conn);
                    if (name)
                    {
                        std::println("Name: {}", *name);
                    }
                    else
                    {
                        std::println("Failed to read product name");
                    }
                }
                break;

            case cli::CommandLineAction::GET_NUM_CONNECTED_MODULES:
                std::println("=== Number of Currently Connected Modules ===");
                {
                    auto num = caparoc::read_uint16(conn, 0x2000);
                    if (num)
                    {
                        std::println("Connected modules: {}", *num);
                    }
                    else
                    {
                        std::println("Failed to read number of connected modules");
                    }
                }
                break;

            case cli::CommandLineAction::PRINT_DEVICE_INFO:
                std::println("=== Device Information ===");
                {
                    try
                    {
                        std::println("{}", caparoc::print_device_info(conn));
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error reading device information: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::GET_SYSTEM_STATUS:
                std::println("=== System Status ===");
                {
                    auto global_status = caparoc::get_global_status(conn);
                    if (global_status)
                    {
                        std::println("Global Status Bits:");
                        std::println("  Undervoltage: {}", global_status->undervoltage ? "YES" : "no");
                        std::println("  Overvoltage: {}", global_status->overvoltage ? "YES" : "no");
                        std::println("  Cumulative Channel Error: {}", global_status->cumulative_channel_error ? "YES" : "no");
                        std::println("  Cumulative 80% Warning: {}", global_status->cumulative_80_warning ? "YES" : "no");
                        std::println("  System Current Too High: {}", global_status->system_current_too_high ? "YES" : "no");
                    }
                    else
                    {
                        std::println("Failed to read global status");
                    }

                    auto total_current = caparoc::get_total_system_current(conn);
                    if (total_current)
                    {
                        std::println("Total System Current: {} A", *total_current);
                    }

                    auto input_voltage = caparoc::get_input_voltage(conn);
                    if (input_voltage)
                    {
                        std::println("Input Voltage: {:.2f} V", *input_voltage / 100.0);
                    }

                    auto sum_nominal = caparoc::get_sum_of_nominal_currents(conn);
                    if (sum_nominal)
                    {
                        std::println("Sum of Nominal Currents: {} A", *sum_nominal);
                    }

                    auto temperature = caparoc::get_internal_temperature(conn);
                    if (temperature)
                    {
                        std::println("Internal Temperature: {} °C", *temperature);
                    }
                }
                break;

            case cli::CommandLineAction::GET_CHANNEL_STATUS:
                for (const auto &args : options.get_channel_status_args)
                {
                    try
                    {
                        auto module = std::stoi(args.module_number);
                        auto channel = std::stoi(args.channel_number);
                        std::println("=== Channel Status (Module {}, Channel {}) ===", module, channel);

                        auto status = caparoc::get_channel_status(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel));
                        if (status)
                        {
                            std::println("  80% Warning: {}", status->warning_80_percent ? "YES" : "no");
                            std::println("  Overload: {}", status->overload ? "YES" : "no");
                            std::println("  Short Circuit: {}", status->short_circuit ? "YES" : "no");
                            std::println("  Hardware Error: {}", status->hardware_error ? "YES" : "no");
                            std::println("  Voltage Error: {}", status->voltage_error ? "YES" : "no");
                            std::println("  Module Current Too High: {}", status->module_current_too_high ? "YES" : "no");
                            std::println("  System Current Too High: {}", status->system_current_too_high ? "YES" : "no");
                        }
                        else
                        {
                            std::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::GET_LOAD_CURRENT:
                for (const auto &args : options.get_load_current_args)
                {
                    try
                    {
                        auto module = std::stoi(args.module_number);
                        auto channel = std::stoi(args.channel_number);
                        std::println("=== Load Current (Module {}, Channel {}) ===", module, channel);

                        auto current = caparoc::get_load_current(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel));
                        if (current)
                        {
                            double amps = *current / 1000.0;
                            std::println("{:.1f} A ({} mA)", amps, *current);
                        }
                        else
                        {
                            std::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::CONTROL_CHANNEL:
                for (const auto &args : options.control_channel_args)
                {
                    try
                    {
                        auto module = std::stoi(args.module_number);
                        auto channel = std::stoi(args.channel_number);
                        bool on = (args.state == "on" || args.state == "ON" || args.state == "1");
                        std::println("=== Control Channel (Module {}, Channel {} -> {}) ===", module, channel, on ? "ON" : "OFF");

                        if (caparoc::control_channel(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel), on))
                        {
                            std::println("SUCCESS");
                        }
                        else
                        {
                            std::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::READ_COIL:
                std::println("=== Read Coil ===");
                for (const auto &args : options.read_coil_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 0);
                        bool value;

                        if (!conn.set_slave_id(1)) {  // Waveshare default is usually 1
                            std::println("Failed to set slave ID: {}", conn.get_last_error());    
                        }    

                        if (conn.read_coil(static_cast<uint16_t>(addr), value))
                        {
                            std::println("Coil 0x{:04X}: {} ({})", addr, value ? "ON" : "OFF", value);
                        }
                        else
                        {
                            std::println("Failed to read coil 0x{:04X}: {}", addr, conn.get_last_error());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::WRITE_COIL:
                std::println("=== Write Coil ===");
                for (const auto &args : options.write_coil_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 0);
                        bool state = (args.state == "on" || args.state == "ON" || 
                                     args.state == "true" || args.state == "TRUE" || 
                                     args.state == "1");
                        
                        if (conn.write_coil(static_cast<uint16_t>(addr), state))
                        {
                            std::println("Coil 0x{:04X} = {} (SUCCESS)", addr, state ? "ON" : "OFF");
                        }
                        else
                        {
                            std::println("Coil 0x{:04X} = {} (FAILED): {}", addr, state ? "ON" : "OFF", conn.get_last_error());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::GET_NOMINAL_CURRENT:
                for (const auto &args : options.get_nominal_current_args)
                {
                    try
                    {
                        auto module = std::stoi(args.module_number);
                        auto channel = std::stoi(args.channel_number);
                        std::println("=== Get Nominal Current (Module {}, Channel {}) ===", module, channel);
                        auto value = caparoc::get_nominal_current(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel));
                        if (value)
                        {
                            std::println("Nominal current: {} A", *value);
                        }
                        else
                        {
                            std::println("Failed to read nominal current");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::SET_NOMINAL_CURRENT:
                for (const auto &args : options.set_nominal_current_args)
                {
                    try
                    {
                        auto module = std::stoi(args.module_number);
                        auto channel = std::stoi(args.channel_number);
                        auto value = std::stoi(args.value);
                        std::println("=== Set Nominal Current (Module {}, Channel {} to {} A) ===", module, channel, value);
                        if (caparoc::set_nominal_current(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel), static_cast<uint16_t>(value)))
                        {
                            std::println("SUCCESS");
                        }
                        else
                        {
                            std::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::UNLOCK_NOMINAL_CURRENT:
                for (const auto &args : options.unlock_nominal_current_args)
                {
                    try
                    {
                        auto module = std::stoi(args.module_number);
                        auto channel = std::stoi(args.channel_number);
                        std::println("=== Unlock Nominal Current (Module {}, Channel {}) ===", module, channel);

                        const uint16_t global_lock_address = 0xC001;
                        const uint16_t channel_lock_address = static_cast<uint16_t>(0xC090 + (module - 1) * 4 + (channel - 1));

                        if (!caparoc::write_uint16(conn, global_lock_address, 0))
                        {
                            std::println("FAILED (global lock)");
                            continue;
                        }
                        if (!caparoc::write_uint16(conn, channel_lock_address, 0))
                        {
                            std::println("FAILED (channel lock)");
                            continue;
                        }

                        std::println("SUCCESS");
                    }
                    catch (const std::exception &e)
                    {
                        std::println("Error parsing arguments: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::NONE:
            default:
                break;
            }
        }

        // // Example: Read product information (if device is connected)
        // std::println("\n=== Product Information ===");
        // auto power_module_name = caparoc::get_product_name_power_module(conn);
        // if (power_module_name)
        // {
        //     std::println("Power Module: {}", *power_module_name);
        // }
        // else
        // {
        //     std::println("Could not read Power Module product name");
        // }

        // auto module1_name = caparoc::get_product_name_module(conn, 1);
        // if (module1_name)
        // {
        //     std::println("Module 1: {}", *module1_name);
        // }
        // else
        // {
        //     std::println("Could not read Module 1 product name (might not be installed)");
        // }

        // auto quint_name = caparoc::get_product_name_quint(conn);
        // if (quint_name)
        // {
        //     std::println("QUINT Power Supply: {}", *quint_name);
        // }
        // else
        // {
        //     std::println("Could not read QUINT product name");
        // }

        // // Display register statistics
        // // std::println("=== Available MODBUS Registers ===");
        // // std::print("{}", caparoc::list_all_registers());

        // // Demonstrate searching for specific registers
        // std::println("\n=== Example: Searching for 'voltage' registers ===");
        // auto voltage_regs = caparoc::find_registers("voltage");
        // std::println("Found {} voltage-related registers", voltage_regs.size());
        // if (!voltage_regs.empty())
        // {
        //     std::println("First few:");
        //     for (size_t i = 0; i < std::min(size_t(5), voltage_regs.size()); ++i)
        //     {
        //         std::println("  0x{:04X}", voltage_regs[i]);
        //     }
        // }

        // // Example: Get info about a specific register
        // std::println("\n=== Register Information Example ===");
        // std::println("{}", caparoc::get_register_info(0x0010));

        // // Example: Generic register read
        // std::println("\n=== Generic Register Access Example ===");
        // std::println("Reading register 0x1000 (Product Name Power Module)...");
        // auto string_val = caparoc::read_string32(conn, 0x1000);
        // if (string_val)
        // {
        //     std::println("  Value: \"{}\"", *string_val);
        // }
        // else
        // {
        //     std::println("  Read failed");
        // }

        // std::println("\n=== Control Functions Demo ===");
        // std::println("Note: These are WRITE-ONLY functions that reset/configure the device");
        // std::println("");

        // std::println("Available control functions:");
        // std::println("  - reset_application_params_power_and_cb()");
        // std::println("  - global_channel_error_reset_all_cb()");
        // std::println("  - error_counter_reset_all_cb()");
        // std::println("  - reset_application_params_quint()");
        // std::println("");

        // // Demonstrate writing to a register (commented out for safety)
        // std::println("Example usage (commented out for safety):");
        // std::println("  // Reset all application parameters:");
        // std::println("  // if (caparoc::reset_application_params_power_and_cb(conn)) {{");
        // std::println("  //     std::println(\"Reset successful\");");
        // std::println("  // }}");
        // std::println("");
        // std::println("  // Generic register write:");
        // std::println("  // if (caparoc::write_uint16(conn, 0x0010, 1)) {{");
        // std::println("  //     std::println(\"Write successful\");");
        // std::println("  // }}");
        // std::println("");

        // std::println("=== Library Statistics ===");
        // std::println("Total registers: 771");
        // std::println("  Read-Only (RO): 464");
        // std::println("  Write-Only (WO): 101");
        // std::println("  Read-Write (RW): 206");
        // std::println("");
        // std::println("Register types:");
        // std::println("  UINT16: 678");
        // std::println("  UINT32: 2");
        // std::println("  INT16: 1");
        // std::println("  STRING32: 90");
        // std::println("");

        // std::println("Demo completed successfully!");
        return 0;
    }
    catch (const std::exception &e)
    {
        std::println("ERROR: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
