#ifndef CAPAROC_COMMANDER_NETWORK_CONFIG_HPP
#define CAPAROC_COMMANDER_NETWORK_CONFIG_HPP

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

// The network settings of a CAPAROC module, read and written through the module's web interface:
// GET and POST on http://<module>/webif/network with the JSON object
// {"mode":"static","ipv4":"…","subnet":"…","gateway":"…"}. Plain HTTP, no authentication.
namespace webif {

struct NetworkConfig {
    std::string mode;     // "static" for a fixed address
    std::string ipv4;
    std::string subnet;
    std::string gateway;  // "0.0.0.0" for none

    friend bool operator==(const NetworkConfig&, const NetworkConfig&) = default;
};

// Where a module's web interface listens.
struct Endpoint {
    std::string host;
    int port = 80;
};

// The module could not be reached, or answered with something other than its network settings.
class WebInterfaceError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// A dotted quad as a host-order number. Four decimal parts of 0..255 without leading zeros.
// Throws std::invalid_argument.
std::uint32_t parse_ipv4(std::string_view text);

// Checks a static address before it is sent: a contiguous mask; a unicast address that is neither the
// network nor the broadcast address of its subnet; a gateway that is "0.0.0.0" or another usable
// address in the same subnet. Throws std::invalid_argument naming the violated rule.
void check_static_address(const NetworkConfig& config);

// Checks that `wanted.ipv4` lies in the subnet the module is in now (`current.ipv4` and
// `current.subnet`), so the computer that reaches the module now still reaches it afterwards.
// Throws std::invalid_argument.
void check_reachable_after_change(const NetworkConfig& current, const NetworkConfig& wanted);

// The module's JSON answer as settings. Throws WebInterfaceError when a field is missing or not a string.
NetworkConfig network_config_from_json(std::string_view body);

// The JSON body the module accepts, with the fields in the module's own order.
std::string to_json(const NetworkConfig& config);

// One line: "mode static, address …, mask …, gateway …".
std::string describe(const NetworkConfig& config);

// GET /webif/network. Throws WebInterfaceError.
NetworkConfig read_network_config(const Endpoint& endpoint, std::chrono::milliseconds timeout);

// True when anything answers HTTP on `endpoint` within `timeout`.
bool answers_http(const Endpoint& endpoint, std::chrono::milliseconds timeout);

// POSTs `wanted` to the module at `current`, then asks `wanted.ipv4` on the same port until it answers
// with `wanted` or `timeout` has passed since the POST, and returns what it answered. A POST whose
// connection drops after the request was sent counts as sent: a module that has just taken a new
// address may not answer on the old one. Throws WebInterfaceError when the module is not reached for
// the POST, refuses it, or does not answer at the new address with `wanted` in time.
NetworkConfig apply_network_config(const Endpoint& current, const NetworkConfig& wanted,
                                   std::chrono::milliseconds timeout);

} // namespace webif

#endif // CAPAROC_COMMANDER_NETWORK_CONFIG_HPP
