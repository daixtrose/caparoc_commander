#ifndef CREATE_MODBUS_CONNECTION_HPP
#define CREATE_MODBUS_CONNECTION_HPP

#include <string>

namespace libmodbus_cpp {
class ModbusConnection;
}

namespace cli {

/**
 * @brief Create and establish a Modbus TCP connection
 * 
 * @param ip_address IP address of the device
 * @param port Modbus TCP port
 * @param timeout_seconds Connection timeout in seconds
 * @return libmodbus_cpp::ModbusConnection Connected ModbusConnection object
 * @throws std::runtime_error if connection fails
 */
libmodbus_cpp::ModbusConnection create_modbus_connection(const std::string& ip_address, int port, int timeout_seconds);

} // namespace cli

#endif  // CREATE_MODBUS_CONNECTION_HPP
