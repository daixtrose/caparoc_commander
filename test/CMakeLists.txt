FetchContent_Declare(
    ut
    GIT_REPOSITORY https://github.com/boost-ext/ut.git
    GIT_TAG        v2.1.0
    GIT_SHALLOW    TRUE
    SYSTEM
)
set(BOOST_UT_DISABLE_MODULE ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ut)

# The address checks, the JSON mapping, and a round trip against an in-process stand-in for the
# module's web interface on 127.0.0.1.
add_executable(network_config_test network_config_test.cpp)
target_link_libraries(network_config_test PRIVATE caparoc_commander_webif httplib::httplib Boost::ut)
target_compile_features(network_config_test PRIVATE cxx_std_23)
if(MINGW)
    target_link_options(network_config_test PRIVATE -static)
endif()
add_test(NAME network_config_test COMMAND network_config_test)
