#include "caparoc_commander/network_config.hpp"

#include <httplib.h>

#include <boost/ut.hpp>

#include <chrono>
#include <format>
#include <mutex>
#include <string>
#include <thread>

using namespace boost::ut;
using namespace std::chrono_literals;

namespace
{
    // A module's web interface on 127.0.0.1: GET answers with the stored settings in the module's
    // format, POST stores the body (unless `takes_settings` is false) and answers with an empty 200
    // after `post_delay`.
    class FakeModule
    {
    public:
        explicit FakeModule(webif::NetworkConfig initial, bool takes_settings = true,
                            std::chrono::milliseconds post_delay = 0ms)
            : stored_{std::move(initial)}, takes_settings_{takes_settings}, post_delay_{post_delay}
        {
            server_.Get("/webif/network", [this](const httplib::Request &, httplib::Response &response)
                        {
                std::lock_guard lock{mutex_};
                response.set_content(std::format(R"({{"mode":"{}","ipv4":"{}","subnet":"{}","gateway":"{}"}})",
                                                 stored_.mode, stored_.ipv4, stored_.subnet, stored_.gateway),
                                     "application/json"); });
            server_.Post("/webif/network", [this](const httplib::Request &request, httplib::Response &)
                         {
                {
                    std::lock_guard lock{mutex_};
                    posted_ = request.body;
                    if (takes_settings_)
                    {
                        stored_ = webif::network_config_from_json(request.body);
                    }
                }
                std::this_thread::sleep_for(post_delay_); });
            port_ = server_.bind_to_any_port("127.0.0.1");
            thread_ = std::thread{[this]
                                  { server_.listen_after_bind(); }};
            server_.wait_until_ready();
        }

        ~FakeModule()
        {
            server_.stop();
            thread_.join();
        }

        webif::Endpoint endpoint() const { return {"127.0.0.1", port_}; }

        std::string posted()
        {
            std::lock_guard lock{mutex_};
            return posted_;
        }

    private:
        httplib::Server server_;
        std::mutex mutex_;
        webif::NetworkConfig stored_;
        std::string posted_;
        bool takes_settings_;
        std::chrono::milliseconds post_delay_;
        int port_ = 0;
        std::thread thread_;
    };

    const webif::NetworkConfig on_loopback{"static", "127.0.0.1", "255.0.0.0", "0.0.0.0"};

    // A port on 127.0.0.1 on which nothing listens any more.
    int closed_port()
    {
        const FakeModule gone{on_loopback};
        return gone.endpoint().port;
    }
}

int main()
{
    "parse_ipv4 reads four decimal parts"_test = []
    {
        expect(webif::parse_ipv4("192.168.1.241") == 0xC0A801F1u);
        expect(webif::parse_ipv4("0.0.0.0") == 0u);
        expect(webif::parse_ipv4("255.255.255.255") == 0xFFFFFFFFu);
    };

    "parse_ipv4 refuses anything else"_test = []
    {
        for (const auto *text : {"", "192.168.1", "192.168.1.241.1", "192.168.1.256", "192.168.01.2", "192.168..2",
                                 "192.168.1.", ".192.168.1", "192.168.1.+2", " 192.168.1.2", "192.168.1.2 ", "a.b.c.d",
                                 "1000.1.1.1"})
        {
            expect(throws<std::invalid_argument>([&]
                                                 { (void)webif::parse_ipv4(text); }))
                << text;
        }
    };

    "check_static_address accepts the vehicle settings"_test = []
    {
        expect(nothrow([]
                       { webif::check_static_address({"static", "192.168.1.241", "255.255.255.0", "192.168.1.1"}); }));
        expect(nothrow([]
                       { webif::check_static_address({"static", "192.168.1.242", "255.255.255.0", "0.0.0.0"}); }));
        expect(nothrow([]
                       { webif::check_static_address({"static", "10.0.0.5", "255.255.255.252", "10.0.0.6"}); }));
    };

    "check_static_address refuses what the module could not use"_test = []
    {
        const webif::NetworkConfig refused[] = {
            {"dhcp", "192.168.1.241", "255.255.255.0", "192.168.1.1"},     // not static
            {"static", "192.168.1.241", "255.255.0.255", "192.168.1.1"},   // mask not contiguous
            {"static", "192.168.1.241", "0.0.0.0", "0.0.0.0"},             // no mask
            {"static", "192.168.1.0", "255.255.255.0", "192.168.1.1"},     // network address
            {"static", "192.168.1.255", "255.255.255.0", "192.168.1.1"},   // broadcast address
            {"static", "192.168.1.241", "255.255.255.255", "0.0.0.0"},     // /32: the address is the network
            {"static", "127.0.0.1", "255.0.0.0", "0.0.0.0"},               // loopback
            {"static", "224.0.0.1", "255.255.255.0", "0.0.0.0"},           // multicast
            {"static", "0.1.2.3", "255.255.255.0", "0.0.0.0"},             // 0.0.0.0/8
            {"static", "192.168.1.241", "255.255.255.0", "192.168.2.1"},   // gateway in another subnet
            {"static", "192.168.1.241", "255.255.255.0", "192.168.1.255"}, // gateway is the broadcast address
            {"static", "192.168.1.241", "255.255.255.0", "192.168.1.241"}, // gateway is the module itself
            {"static", "192.168.1.241", "255.255.255.0", "192.168.1"},     // gateway not an address
        };
        for (const auto &config : refused)
        {
            expect(throws<std::invalid_argument>([&]
                                                 { webif::check_static_address(config); }))
                << webif::describe(config);
        }
    };

    "check_reachable_after_change keeps the module in its current subnet"_test = []
    {
        const webif::NetworkConfig factory{"static", "192.168.1.2", "255.255.255.0", "0.0.0.0"};
        expect(nothrow([&]
                       { webif::check_reachable_after_change(factory, {"static", "192.168.1.241", "255.255.255.0", "192.168.1.1"}); }));
        expect(throws<std::invalid_argument>([&]
                                             { webif::check_reachable_after_change(factory, {"static", "192.168.2.241", "255.255.255.0", "0.0.0.0"}); }));
    };

    "the module's answer maps to the settings and back"_test = []
    {
        const std::string answer = R"({"mode":"static","ipv4":"192.168.1.2","subnet":"255.255.255.0","gateway":"0.0.0.0"})";
        const auto config = webif::network_config_from_json(answer);
        expect(config == webif::NetworkConfig{"static", "192.168.1.2", "255.255.255.0", "0.0.0.0"});
        expect(webif::to_json(config) == answer) << "the field order the module uses";
    };

    "an answer without the four text fields is an error"_test = []
    {
        for (const auto *body : {"", "not json", "[]", R"({"mode":"static","ipv4":"192.168.1.2","subnet":"255.255.255.0"})",
                                 R"({"mode":"static","ipv4":3232235778,"subnet":"255.255.255.0","gateway":"0.0.0.0"})"})
        {
            expect(throws<webif::WebInterfaceError>([&]
                                                    { (void)webif::network_config_from_json(body); }))
                << body;
        }
    };

    "read_network_config reads the module"_test = []
    {
        FakeModule module{on_loopback};
        expect(webif::read_network_config(module.endpoint(), 2s) == on_loopback);
        expect(webif::answers_http(module.endpoint(), 2s));
    };

    "nothing listening is an error, not settings"_test = []
    {
        const webif::Endpoint nowhere{"127.0.0.1", closed_port()};
        expect(throws<webif::WebInterfaceError>([&]
                                                { (void)webif::read_network_config(nowhere, 2s); }));
        expect(!webif::answers_http(nowhere, 2s));
        expect(throws<webif::WebInterfaceError>([&]
                                                { (void)webif::apply_network_config(nowhere, on_loopback, 2s); }));
    };

    "apply_network_config sends the settings and confirms them"_test = []
    {
        FakeModule module{on_loopback};
        const webif::NetworkConfig wanted{"static", "127.0.0.1", "255.0.0.0", "127.0.0.254"};
        expect(webif::apply_network_config(module.endpoint(), wanted, 3s) == wanted);
        expect(module.posted() == R"({"mode":"static","ipv4":"127.0.0.1","subnet":"255.0.0.0","gateway":"127.0.0.254"})");
    };

    "a POST that is never answered still counts as sent"_test = []
    {
        FakeModule module{on_loopback, true, 2500ms};
        const webif::NetworkConfig wanted{"static", "127.0.0.1", "255.0.0.0", "127.0.0.254"};
        expect(webif::apply_network_config(module.endpoint(), wanted, 2s) == wanted);
    };

    "a module that keeps its old settings is an error"_test = []
    {
        FakeModule module{on_loopback, false};
        const webif::NetworkConfig wanted{"static", "127.0.0.1", "255.0.0.0", "127.0.0.254"};
        expect(throws<webif::WebInterfaceError>([&]
                                                { (void)webif::apply_network_config(module.endpoint(), wanted, 2s); }));
    };
}
