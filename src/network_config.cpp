#include "caparoc_commander/network_config.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <charconv>
#include <format>
#include <thread>

namespace webif
{
    namespace
    {
        constexpr const char *network_path = "/webif/network";

        // While waiting for the module at its new address: how long one request may take, and the
        // pause before the next one.
        constexpr std::chrono::milliseconds read_back_attempt{1000};
        constexpr std::chrono::milliseconds read_back_pause{500};

        std::string url(const Endpoint &endpoint)
        {
            return std::format("http://{}:{}{}", endpoint.host, endpoint.port, network_path);
        }

        std::string dotted(std::uint32_t address)
        {
            return std::format("{}.{}.{}.{}", address >> 24, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF);
        }

        std::string shortened(std::string_view body)
        {
            constexpr std::size_t shown = 200;
            return body.size() <= shown ? std::string{body} : std::format("{}…", body.substr(0, shown));
        }

        // `text` as an address, with `what` ("address", "mask", "gateway") in the error message.
        std::uint32_t parse_field(std::string_view what, std::string_view text)
        {
            try
            {
                return parse_ipv4(text);
            }
            catch (const std::invalid_argument &e)
            {
                throw std::invalid_argument(std::format("{}: {}", what, e.what()));
            }
        }

        // Not in 0.0.0.0/8, 127.0.0.0/8 or 224.0.0.0 and above.
        bool is_unicast(std::uint32_t address)
        {
            const auto first = address >> 24;
            return first != 0 && first != 127 && first < 224;
        }

        httplib::Client make_client(const Endpoint &endpoint, std::chrono::milliseconds timeout)
        {
            httplib::Client client(endpoint.host, endpoint.port);
            client.set_connection_timeout(timeout);
            client.set_read_timeout(timeout);
            client.set_write_timeout(timeout);
            return client;
        }
    }

    std::uint32_t parse_ipv4(std::string_view text)
    {
        const auto invalid = [text]
        { return std::invalid_argument(std::format("'{}' is not an IPv4 address (four numbers 0..255 separated by dots)", text)); };

        std::uint32_t address = 0;
        int parts = 0;
        std::size_t begin = 0;
        while (true)
        {
            const auto dot = text.find('.', begin);
            const auto part = text.substr(begin, dot == std::string_view::npos ? std::string_view::npos : dot - begin);
            const bool digits_only = std::ranges::all_of(part, [](char c)
                                                         { return c >= '0' && c <= '9'; });
            if (part.empty() || part.size() > 3 || !digits_only || (part.size() > 1 && part.front() == '0'))
            {
                throw invalid();
            }
            unsigned octet = 0;
            std::from_chars(part.data(), part.data() + part.size(), octet);
            if (octet > 255 || ++parts > 4)
            {
                throw invalid();
            }
            address = (address << 8) | octet;
            if (dot == std::string_view::npos)
            {
                break;
            }
            begin = dot + 1;
        }
        if (parts != 4)
        {
            throw invalid();
        }
        return address;
    }

    void check_static_address(const NetworkConfig &config)
    {
        if (config.mode != "static")
        {
            throw std::invalid_argument(std::format("mode '{}': only 'static' is supported", config.mode));
        }
        const auto address = parse_field("address", config.ipv4);
        const auto mask = parse_field("mask", config.subnet);
        const auto gateway = parse_field("gateway", config.gateway);

        const std::uint32_t host_bits = ~mask;
        if (mask == 0 || (host_bits & (host_bits + 1)) != 0)
        {
            throw std::invalid_argument(std::format("mask {} is not a subnet mask (ones, then zeros)", config.subnet));
        }
        const auto network = address & mask;
        const auto broadcast = network | host_bits;
        const auto subnet = std::format("{}/{}", dotted(network), config.subnet);

        if (!is_unicast(address))
        {
            throw std::invalid_argument(std::format("address {} is not a unicast address", config.ipv4));
        }
        if (address == network)
        {
            throw std::invalid_argument(std::format("address {} is the network address of {}", config.ipv4, subnet));
        }
        if (address == broadcast)
        {
            throw std::invalid_argument(std::format("address {} is the broadcast address of {}", config.ipv4, subnet));
        }
        if (gateway == 0)
        {
            return;
        }
        if ((gateway & mask) != network)
        {
            throw std::invalid_argument(std::format("gateway {} is outside the subnet {}", config.gateway, subnet));
        }
        if (gateway == network || gateway == broadcast)
        {
            throw std::invalid_argument(std::format("gateway {} is the network or broadcast address of {}", config.gateway, subnet));
        }
        if (gateway == address)
        {
            throw std::invalid_argument(std::format("gateway {} is the module's own address", config.gateway));
        }
    }

    void check_reachable_after_change(const NetworkConfig &current, const NetworkConfig &wanted)
    {
        const auto mask = parse_field("current mask", current.subnet);
        const auto network = parse_field("current address", current.ipv4) & mask;
        if ((parse_field("address", wanted.ipv4) & mask) != network)
        {
            throw std::invalid_argument(std::format(
                "address {} is outside the subnet {}/{} the module is in now; it could no longer be reached from here",
                wanted.ipv4, dotted(network), current.subnet));
        }
    }

    NetworkConfig network_config_from_json(std::string_view body)
    {
        const auto json = nlohmann::json::parse(body.begin(), body.end(), nullptr, false);
        if (!json.is_object())
        {
            throw WebInterfaceError(std::format("not a JSON object: '{}'", shortened(body)));
        }
        const auto field = [&](const char *key)
        {
            const auto it = json.find(key);
            if (it == json.end() || !it->is_string())
            {
                throw WebInterfaceError(std::format("no text field \"{}\" in '{}'", key, shortened(body)));
            }
            return it->get<std::string>();
        };
        return {field("mode"), field("ipv4"), field("subnet"), field("gateway")};
    }

    std::string to_json(const NetworkConfig &config)
    {
        nlohmann::ordered_json json;
        json["mode"] = config.mode;
        json["ipv4"] = config.ipv4;
        json["subnet"] = config.subnet;
        json["gateway"] = config.gateway;
        return json.dump();
    }

    std::string describe(const NetworkConfig &config)
    {
        return std::format("mode {}, address {}, mask {}, gateway {}", config.mode, config.ipv4, config.subnet, config.gateway);
    }

    NetworkConfig read_network_config(const Endpoint &endpoint, std::chrono::milliseconds timeout)
    {
        auto client = make_client(endpoint, timeout);
        const auto result = client.Get(network_path);
        if (!result)
        {
            throw WebInterfaceError(std::format("{}: no answer ({})", url(endpoint), httplib::to_string(result.error())));
        }
        if (result->status / 100 != 2)
        {
            throw WebInterfaceError(std::format("{}: HTTP {}", url(endpoint), result->status));
        }
        try
        {
            return network_config_from_json(result->body);
        }
        catch (const WebInterfaceError &e)
        {
            throw WebInterfaceError(std::format("{}: {}", url(endpoint), e.what()));
        }
    }

    bool answers_http(const Endpoint &endpoint, std::chrono::milliseconds timeout)
    {
        auto client = make_client(endpoint, timeout);
        return static_cast<bool>(client.Get(network_path));
    }

    NetworkConfig apply_network_config(const Endpoint &current, const NetworkConfig &wanted,
                                       std::chrono::milliseconds timeout)
    {
        {
            auto client = make_client(current, timeout);
            const auto result = client.Post(network_path, to_json(wanted), "application/json");
            if (!result)
            {
                const auto error = result.error();
                if (error == httplib::Error::Connection || error == httplib::Error::ConnectionTimeout)
                {
                    throw WebInterfaceError(std::format("{}: not reached, nothing sent ({})", url(current), httplib::to_string(error)));
                }
            }
            else if (result->status / 100 != 2)
            {
                throw WebInterfaceError(std::format("{}: HTTP {}, the module refused the settings", url(current), result->status));
            }
        }

        const Endpoint target{wanted.ipv4, current.port};
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        std::string last = "no answer";
        while (true)
        {
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
            if (remaining <= std::chrono::milliseconds::zero())
            {
                break;
            }
            try
            {
                const auto answered = read_network_config(target, std::min(remaining, read_back_attempt));
                if (answered == wanted)
                {
                    return answered;
                }
                last = std::format("{} answers with {}", url(target), describe(answered));
            }
            catch (const WebInterfaceError &e)
            {
                last = e.what();
            }
            std::this_thread::sleep_for(std::min(read_back_pause, std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                      deadline - std::chrono::steady_clock::now())));
        }
        throw WebInterfaceError(std::format("settings not confirmed within {} ms after sending {}; last: {}",
                                            timeout.count(), describe(wanted), last));
    }

} // namespace webif
