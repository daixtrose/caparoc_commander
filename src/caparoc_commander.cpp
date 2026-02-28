#include "caparoc/caparoc.hpp"
#include "libmodbus_cpp/modbus_connection.hpp"
#include "caparoc_commander/cli_parser.hpp"
#include "caparoc_commander/create_modbus_connection.hpp"
#include "caparoc_commander/portable_print.hpp"

#include <format>
#include <stdexcept>
#include <cstdlib>

int main(int argc, char *argv[])
{
    try
    {
        auto options = cli::parse_command_line(argc, argv);

        if (options.debug)
        {
            portable::println("========================");
            portable::println("CAPAROC Commander");
            portable::println("========================");
            portable::println("Connecting to {}:{}", options.ip_address, options.port);
            portable::println("");

            portable::println("Command Line Options:");
            portable::println("{}", cli::dump_command_line_options(options));
            portable::println("");
        }

        // Create and connect to device
        auto conn = cli::create_modbus_connection(options.ip_address, options.port, options.timeout_seconds);

        if (options.debug)
        {
            portable::println("Connected successfully!");
            portable::println("");
        }

        // Process all requested actions
        for (const auto &action : options.actions)
        {
            switch (action)
            {
            case cli::CommandLineAction::LIST_REGISTERS:
                portable::println("=== All Registers ===");
                portable::println("{}", caparoc::list_all_registers());
                break;

            case cli::CommandLineAction::REGISTER_INFO:
                portable::println("=== Register Information ===");
                portable::println("Address: {}", options.register_info_address);
                // Parse hex address and get info
                try
                {
                    auto addr = std::stoi(options.register_info_address, nullptr, 16);
                    portable::println("{}", caparoc::get_register_info(static_cast<uint16_t>(addr)));
                }
                catch (const std::exception &e)
                {
                    portable::println("Error parsing address: {}", e.what());
                }
                break;

            case cli::CommandLineAction::SEARCH_REGISTERS:
                portable::println("=== Search Results for '{}' ===", options.search_filter);
                {
                    auto results = caparoc::find_registers(options.search_filter);
                    portable::println("Found {} registers", results.size());
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
                        portable::println("  [0x{:04X}] {} | {} - {}", reg.address, access_str, reg.name, reg.description);
                    }
                }
                break;

            case cli::CommandLineAction::READ_UINT16:
                portable::println("=== Read UINT16 Register ===");
                portable::println("Address: {}", options.read_uint16_address);
                try
                {
                    auto addr = std::stoi(options.read_uint16_address, nullptr, 16);
                    auto val = caparoc::read_uint16(conn, static_cast<uint16_t>(addr));
                    if (val)
                    {
                        portable::println("Value: {}", *val);
                    }
                    else
                    {
                        portable::println("Failed to read register");
                    }
                }
                catch (const std::exception &e)
                {
                    portable::println("Error: {}", e.what());
                }
                break;

            case cli::CommandLineAction::READ_UINT32:
                portable::println("=== Read UINT32 Register ===");
                portable::println("Address: {}", options.read_uint32_address);
                try
                {
                    auto addr = std::stoi(options.read_uint32_address, nullptr, 16);
                    auto val = caparoc::read_uint32(conn, static_cast<uint16_t>(addr));
                    if (val)
                    {
                        portable::println("Value: {}", *val);
                    }
                    else
                    {
                        portable::println("Failed to read register");
                    }
                }
                catch (const std::exception &e)
                {
                    portable::println("Error: {}", e.what());
                }
                break;

            case cli::CommandLineAction::READ_STRING32:
                portable::println("=== Read STRING32 Register ===");
                portable::println("Address: {}", options.read_string32_address);
                try
                {
                    auto addr = std::stoi(options.read_string32_address, nullptr, 16);
                    auto val = caparoc::read_string32(conn, static_cast<uint16_t>(addr));
                    if (val)
                    {
                        portable::println("Value: \"{}\"", *val);
                    }
                    else
                    {
                        portable::println("Failed to read register");
                    }
                }
                catch (const std::exception &e)
                {
                    portable::println("Error: {}", e.what());
                }
                break;

            case cli::CommandLineAction::WRITE_UINT16:
                portable::println("=== Write UINT16 Registers ===");
                for (const auto &args : options.write_uint16_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 16);
                        auto val = std::stoi(args.value, nullptr, 0);
                        if (caparoc::write_uint16(conn, static_cast<uint16_t>(addr), static_cast<uint16_t>(val)))
                        {
                            portable::println("  0x{:04X} = {} (SUCCESS)", addr, val);
                        }
                        else
                        {
                            portable::println("  0x{:04X} = {} (FAILED)", addr, val);
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("  Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::WRITE_UINT32:
                portable::println("=== Write UINT32 Registers ===");
                for (const auto &args : options.write_uint32_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 16);
                        auto val = std::stoll(args.value, nullptr, 0);
                        if (caparoc::write_uint32(conn, static_cast<uint16_t>(addr), static_cast<uint32_t>(val)))
                        {
                            portable::println("  0x{:04X} = {} (SUCCESS)", addr, val);
                        }
                        else
                        {
                            portable::println("  0x{:04X} = {} (FAILED)", addr, val);
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("  Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::RESET_APPLICATION_PARAMS_POWER_AND_CB:
                portable::println("=== Reset Application Parameters (Power Module and Circuit Breakers) ===");
                if (caparoc::reset_application_params_power_and_cb(conn))
                {
                    portable::println("SUCCESS");
                }
                else
                {
                    portable::println("FAILED");
                }
                break;

            case cli::CommandLineAction::GLOBAL_CHANNEL_ERROR_RESET_ALL_CB:
                portable::println("=== Global Channel Error Reset (All Circuit Breakers) ===");
                if (caparoc::global_channel_error_reset_all_cb(conn))
                {
                    portable::println("SUCCESS");
                }
                else
                {
                    portable::println("FAILED");
                }
                break;

            case cli::CommandLineAction::ERROR_COUNTER_RESET_ALL_CB:
                portable::println("=== Error Counter Reset (All Circuit Breakers) ===");
                if (caparoc::error_counter_reset_all_cb(conn))
                {
                    portable::println("SUCCESS");
                }
                else
                {
                    portable::println("FAILED");
                }
                break;

            case cli::CommandLineAction::RESET_APPLICATION_PARAMS_QUINT:
                portable::println("=== Reset Application Parameters (QUINT Power Supply) ===");
                if (caparoc::reset_application_params_quint(conn))
                {
                    portable::println("SUCCESS");
                }
                else
                {
                    portable::println("FAILED");
                }
                break;

            case cli::CommandLineAction::GET_PRODUCT_NAME_POWER_MODULE:
                portable::println("=== Product Name (Power Module) ===");
                {
                    auto name = caparoc::get_product_name_power_module(conn);
                    if (name)
                    {
                        portable::println("Name: {}", *name);
                    }
                    else
                    {
                        portable::println("Failed to read product name");
                    }
                }
                break;

            case cli::CommandLineAction::GET_PRODUCT_NAME_MODULE:
                for (const auto &module_num : options.product_module_numbers)
                {
                    portable::println("=== Product Name (Module {}) ===", module_num);
                    auto name = caparoc::get_product_name_module(conn, static_cast<uint8_t>(module_num));
                    if (name)
                    {
                        portable::println("Name: {}", *name);
                    }
                    else
                    {
                        portable::println("Failed to read product name (module might not be installed)");
                    }
                }
                break;

            case cli::CommandLineAction::GET_PRODUCT_NAME_QUINT:
                portable::println("=== Product Name (QUINT Power Supply) ===");
                {
                    auto name = caparoc::get_product_name_quint(conn);
                    if (name)
                    {
                        portable::println("Name: {}", *name);
                    }
                    else
                    {
                        portable::println("Failed to read product name");
                    }
                }
                break;

            case cli::CommandLineAction::GET_NUM_CONNECTED_MODULES:
                portable::println("=== Number of Currently Connected Modules ===");
                {
                    auto num = caparoc::read_uint16(conn, 0x2000);
                    if (num)
                    {
                        portable::println("Connected modules: {}", *num);
                    }
                    else
                    {
                        portable::println("Failed to read number of connected modules");
                    }
                }
                break;

            case cli::CommandLineAction::PRINT_DEVICE_INFO:
                portable::println("=== Device Information ===");
                {
                    try
                    {
                        portable::println("{}", caparoc::print_device_info(conn));
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error reading device information: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::GET_SYSTEM_STATUS:
                portable::println("=== System Status ===");
                {
                    auto global_status = caparoc::get_global_status(conn);
                    if (global_status)
                    {
                        portable::println("Global Status Bits:");
                        portable::println("  Undervoltage: {}", global_status->undervoltage ? "YES" : "no");
                        portable::println("  Overvoltage: {}", global_status->overvoltage ? "YES" : "no");
                        portable::println("  Cumulative Channel Error: {}", global_status->cumulative_channel_error ? "YES" : "no");
                        portable::println("  Cumulative 80% Warning: {}", global_status->cumulative_80_warning ? "YES" : "no");
                        portable::println("  System Current Too High: {}", global_status->system_current_too_high ? "YES" : "no");
                    }
                    else
                    {
                        portable::println("Failed to read global status");
                    }

                    auto total_current = caparoc::get_total_system_current(conn);
                    if (total_current)
                    {
                        portable::println("Total System Current: {} A", *total_current);
                    }

                    auto input_voltage = caparoc::get_input_voltage(conn);
                    if (input_voltage)
                    {
                        portable::println("Input Voltage: {:.2f} V", *input_voltage / 100.0);
                    }

                    auto sum_nominal = caparoc::get_sum_of_nominal_currents(conn);
                    if (sum_nominal)
                    {
                        portable::println("Sum of Nominal Currents: {} A", *sum_nominal);
                    }

                    auto temperature = caparoc::get_internal_temperature(conn);
                    if (temperature)
                    {
                        portable::println("Internal Temperature: {} °C", *temperature);
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
                        portable::println("=== Channel Status (Module {}, Channel {}) ===", module, channel);

                        auto status = caparoc::get_channel_status(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel));
                        if (status)
                        {
                            portable::println("  80% Warning: {}", status->warning_80_percent ? "YES" : "no");
                            portable::println("  Overload: {}", status->overload ? "YES" : "no");
                            portable::println("  Short Circuit: {}", status->short_circuit ? "YES" : "no");
                            portable::println("  Hardware Error: {}", status->hardware_error ? "YES" : "no");
                            portable::println("  Voltage Error: {}", status->voltage_error ? "YES" : "no");
                            portable::println("  Module Current Too High: {}", status->module_current_too_high ? "YES" : "no");
                            portable::println("  System Current Too High: {}", status->system_current_too_high ? "YES" : "no");
                        }
                        else
                        {
                            portable::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
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
                        portable::println("=== Load Current (Module {}, Channel {}) ===", module, channel);

                        auto current = caparoc::get_load_current(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel));
                        if (current)
                        {
                            double amps = *current / 1000.0;
                            portable::println("{:.1f} A ({} mA)", amps, *current);
                        }
                        else
                        {
                            portable::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
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
                        portable::println("=== Control Channel (Module {}, Channel {} -> {}) ===", module, channel, on ? "ON" : "OFF");

                        if (caparoc::control_channel(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel), on))
                        {
                            portable::println("SUCCESS");
                        }
                        else
                        {
                            portable::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::READ_COIL:
                portable::println("=== Read Coil ===");
                for (const auto &args : options.read_coil_args)
                {
                    try
                    {
                        auto addr = std::stoi(args.address, nullptr, 0);
                        bool value;

                        if (!conn.set_slave_id(1)) {  // Waveshare default is usually 1
                            portable::println("Failed to set slave ID: {}", conn.get_last_error());    
                        }    

                        if (conn.read_coil(static_cast<uint16_t>(addr), value))
                        {
                            portable::println("Coil 0x{:04X}: {} ({})", addr, value ? "ON" : "OFF", value);
                        }
                        else
                        {
                            portable::println("Failed to read coil 0x{:04X}: {}", addr, conn.get_last_error());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::WRITE_COIL:
                portable::println("=== Write Coil ===");
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
                            portable::println("Coil 0x{:04X} = {} (SUCCESS)", addr, state ? "ON" : "OFF");
                        }
                        else
                        {
                            portable::println("Coil 0x{:04X} = {} (FAILED): {}", addr, state ? "ON" : "OFF", conn.get_last_error());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
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
                        portable::println("=== Get Nominal Current (Module {}, Channel {}) ===", module, channel);
                        auto value = caparoc::get_nominal_current(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel));
                        if (value)
                        {
                            portable::println("Nominal current: {} A", *value);
                        }
                        else
                        {
                            portable::println("Failed to read nominal current");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
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
                        portable::println("=== Set Nominal Current (Module {}, Channel {} to {} A) ===", module, channel, value);
                        if (caparoc::set_nominal_current(conn, static_cast<uint8_t>(module), static_cast<uint8_t>(channel), static_cast<uint16_t>(value)))
                        {
                            portable::println("SUCCESS");
                        }
                        else
                        {
                            portable::println("FAILED");
                        }
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error: {}", e.what());
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
                        portable::println("=== Unlock Nominal Current (Module {}, Channel {}) ===", module, channel);

                        const uint16_t global_lock_address = 0xC001;
                        const uint16_t channel_lock_address = static_cast<uint16_t>(0xC090 + (module - 1) * 4 + (channel - 1));

                        if (!caparoc::write_uint16(conn, global_lock_address, 0))
                        {
                            portable::println("FAILED (global lock)");
                            continue;
                        }
                        if (!caparoc::write_uint16(conn, channel_lock_address, 0))
                        {
                            portable::println("FAILED (channel lock)");
                            continue;
                        }

                        portable::println("SUCCESS");
                    }
                    catch (const std::exception &e)
                    {
                        portable::println("Error parsing arguments: {}", e.what());
                    }
                }
                break;

            case cli::CommandLineAction::NONE:
            default:
                break;
            }
        }

        // // Example: Read product information (if device is connected)
        // portable::println("\n=== Product Information ===");
        // auto power_module_name = caparoc::get_product_name_power_module(conn);
        // if (power_module_name)
        // {
        //     portable::println("Power Module: {}", *power_module_name);
        // }
        // else
        // {
        //     portable::println("Could not read Power Module product name");
        // }

        // auto module1_name = caparoc::get_product_name_module(conn, 1);
        // if (module1_name)
        // {
        //     portable::println("Module 1: {}", *module1_name);
        // }
        // else
        // {
        //     portable::println("Could not read Module 1 product name (might not be installed)");
        // }

        // auto quint_name = caparoc::get_product_name_quint(conn);
        // if (quint_name)
        // {
        //     portable::println("QUINT Power Supply: {}", *quint_name);
        // }
        // else
        // {
        //     portable::println("Could not read QUINT product name");
        // }

        // // Display register statistics
        // // portable::println("=== Available MODBUS Registers ===");
        // // std::print("{}", caparoc::list_all_registers());

        // // Demonstrate searching for specific registers
        // portable::println("\n=== Example: Searching for 'voltage' registers ===");
        // auto voltage_regs = caparoc::find_registers("voltage");
        // portable::println("Found {} voltage-related registers", voltage_regs.size());
        // if (!voltage_regs.empty())
        // {
        //     portable::println("First few:");
        //     for (size_t i = 0; i < std::min(size_t(5), voltage_regs.size()); ++i)
        //     {
        //         portable::println("  0x{:04X}", voltage_regs[i]);
        //     }
        // }

        // // Example: Get info about a specific register
        // portable::println("\n=== Register Information Example ===");
        // portable::println("{}", caparoc::get_register_info(0x0010));

        // // Example: Generic register read
        // portable::println("\n=== Generic Register Access Example ===");
        // portable::println("Reading register 0x1000 (Product Name Power Module)...");
        // auto string_val = caparoc::read_string32(conn, 0x1000);
        // if (string_val)
        // {
        //     portable::println("  Value: \"{}\"", *string_val);
        // }
        // else
        // {
        //     portable::println("  Read failed");
        // }

        // portable::println("\n=== Control Functions Demo ===");
        // portable::println("Note: These are WRITE-ONLY functions that reset/configure the device");
        // portable::println("");

        // portable::println("Available control functions:");
        // portable::println("  - reset_application_params_power_and_cb()");
        // portable::println("  - global_channel_error_reset_all_cb()");
        // portable::println("  - error_counter_reset_all_cb()");
        // portable::println("  - reset_application_params_quint()");
        // portable::println("");

        // // Demonstrate writing to a register (commented out for safety)
        // portable::println("Example usage (commented out for safety):");
        // portable::println("  // Reset all application parameters:");
        // portable::println("  // if (caparoc::reset_application_params_power_and_cb(conn)) {{");
        // portable::println("  //     portable::println(\"Reset successful\");");
        // portable::println("  // }}");
        // portable::println("");
        // portable::println("  // Generic register write:");
        // portable::println("  // if (caparoc::write_uint16(conn, 0x0010, 1)) {{");
        // portable::println("  //     portable::println(\"Write successful\");");
        // portable::println("  // }}");
        // portable::println("");

        // portable::println("=== Library Statistics ===");
        // portable::println("Total registers: 771");
        // portable::println("  Read-Only (RO): 464");
        // portable::println("  Write-Only (WO): 101");
        // portable::println("  Read-Write (RW): 206");
        // portable::println("");
        // portable::println("Register types:");
        // portable::println("  UINT16: 678");
        // portable::println("  UINT32: 2");
        // portable::println("  INT16: 1");
        // portable::println("  STRING32: 90");
        // portable::println("");

        // portable::println("Demo completed successfully!");
        return 0;
    }
    catch (const std::exception &e)
    {
        portable::println("ERROR: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
