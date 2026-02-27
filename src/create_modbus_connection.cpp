#include "caparoc_commander/create_modbus_connection.hpp"
#include "libmodbus_cpp/modbus_connection.hpp"

#include <format>
#include <stdexcept>

namespace cli {

libmodbus_cpp::ModbusConnection create_modbus_connection(const std::string& ip_address, int port, int timeout_seconds)
{
    libmodbus_cpp::ModbusConnection conn(ip_address, port);
    conn.set_response_timeout(timeout_seconds, 0);
    
    if (!conn.connect())
    {
        throw std::runtime_error(
            std::format(
                "Failed to connect to device: {}\n\nat {}:{}\n",
                conn.get_last_error(), ip_address, port));
    }
    
    return conn;
}

} // namespace cli
