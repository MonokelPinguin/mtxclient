cmake_minimum_required(VERSION 3.11)

project(matrix_client VERSION 0.2.0 LANGUAGES CXX C)

option(ASAN "Compile with address sanitizers" OFF)
option(BUILD_LIB_TESTS "Build tests" ON)
option(BUILD_LIB_EXAMPLES "Build examples" ON)
option(COVERAGE "Calculate test coverage" OFF)
option(IWYU "Check headers with include-what-you-use" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DBOOST_MPL_LIMIT_LIST_SIZE=30)
add_definitions(-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS)

if(NOT MSVC)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
        -Wall \
        -Wextra \
        -pipe \
        -pedantic \
        -fsized-deallocation \
        -fdiagnostics-color=always \
        -Wunreachable-code"
    )
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj -bigobj")
endif()

if(ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif()

if(COVERAGE)
  include(CodeCoverage)
  add_custom_target(ctest COMMAND ${CMAKE_CTEST_COMMAND})
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
  setup_target_for_coverage(test_coverage ctest coverage)
endif()

if(NOT MSVC AND NOT APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

include(FeatureSummary)

find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES
    DESCRIPTION "Open source SSL and TLS implementation and cryptographic library"
    URL "https://www.openssl.org/"
    TYPE REQUIRED
)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "A free compression library unencumbered by patents"
    URL "https://www.zlib.net/"
    TYPE REQUIRED
)

find_package(Olm 2)
set_package_properties(Olm PROPERTIES
    DESCRIPTION "An implementation of the Double Ratchet cryptographic ratchet"
    URL "https://git.matrix.org/git/olm/about/"
    TYPE REQUIRED
)

find_package(sodium 1.0.14)
set_package_properties(sodium PROPERTIES
    DESCRIPTION "A modern, portable, easy to use crypto library"
    URL "https://github.com/jedisct1/libsodium"
    TYPE REQUIRED
)

find_package(nlohmann_json 3.2.0)
set_package_properties(nlohmann_json PROPERTIES
    DESCRIPTION "JSON for Modern C++, a C++11 header-only JSON class"
    URL "https://nlohmann.github.io/json/"
    TYPE REQUIRED
)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66
             COMPONENTS atomic
                        chrono
                        date_time
                        iostreams
                        random
                        regex
                        system
                        thread)
set_package_properties(Boost PROPERTIES
    DESCRIPTION "Free peer-reviewed portable C++ source libraries"
    URL "https://www.boost.org/"
    TYPE REQUIRED
)

add_library(matrix_client
            lib/http/client.cpp
            lib/http/session.cpp
            lib/crypto/client.cpp
            lib/utils.cpp
            lib/structs/common.cpp
            lib/structs/errors.cpp
            lib/structs/events.cpp
            lib/structs/requests.cpp
            lib/structs/events/aliases.cpp
            lib/structs/events/avatar.cpp
            lib/structs/events/canonical_alias.cpp
            lib/structs/events/common.cpp
            lib/structs/events/create.cpp
            lib/structs/events/encrypted.cpp
            lib/structs/events/encryption.cpp
            lib/structs/events/guest_access.cpp
            lib/structs/events/history_visibility.cpp
            lib/structs/events/join_rules.cpp
            lib/structs/events/member.cpp
            lib/structs/events/name.cpp
            lib/structs/events/pinned_events.cpp
            lib/structs/events/power_levels.cpp
            lib/structs/events/redaction.cpp
            lib/structs/events/tag.cpp
            lib/structs/events/topic.cpp
            lib/structs/events/messages/audio.cpp
            lib/structs/events/messages/emote.cpp
            lib/structs/events/messages/file.cpp
            lib/structs/events/messages/image.cpp
            lib/structs/events/messages/notice.cpp
            lib/structs/events/messages/text.cpp
            lib/structs/events/messages/video.cpp
            lib/structs/responses/common.cpp
            lib/structs/responses/create_room.cpp
            lib/structs/responses/crypto.cpp
            lib/structs/responses/empty.cpp
            lib/structs/responses/login.cpp
            lib/structs/responses/media.cpp
            lib/structs/responses/messages.cpp
            lib/structs/responses/notifications.cpp
            lib/structs/responses/profile.cpp
            lib/structs/responses/register.cpp
            lib/structs/responses/sync.cpp
            lib/structs/responses/version.cpp)
add_library(MatrixClient::MatrixClient ALIAS matrix_client)
target_include_directories(matrix_client
                           SYSTEM
                           PUBLIC
                           ${Boost_INCLUDE_DIRS}
                           ${OPENSSL_INCLUDE_DIR}
                           ${sodium_INCLUDE_DIR})
target_include_directories(
  matrix_client
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(matrix_client
                      PUBLIC
                      ${Boost_LIBRARIES}
                      ${sodium_LIBRARY_RELEASE}
                      OpenSSL::Crypto
                      OpenSSL::SSL
                      Olm::Olm
                      ZLIB::ZLIB
                      nlohmann_json::nlohmann_json)

if(NOT MSVC AND NOT APPLE)
  target_link_libraries(matrix_client PUBLIC Threads::Threads)
endif()

if(IWYU)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(iwyu_path)
    set_property(TARGET matrix_client
                 PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
  else()
    message(WARNING "Could not find the program include-what-you-use")
  endif()
endif()

if(BUILD_LIB_EXAMPLES)
  add_subdirectory(examples)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

#
# Installation & Target configuration.
#
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/MatrixClient)

install(TARGETS matrix_client
        EXPORT matrix_client-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(matrix_client PROPERTIES EXPORT_NAME MatrixClient)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT matrix_client-targets
               FILE
               MatrixClientTargets.cmake
               NAMESPACE
               MatrixClient::
               DESTINATION
               ${INSTALL_CONFIGDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfigVersion.cmake
  VERSION
  ${PROJECT_VERSION}
  COMPATIBILITY
  AnyNewerVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/MatrixClientConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfig.cmake
  INSTALL_DESTINATION
  ${INSTALL_CONFIGDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfigVersion.cmake
              DESTINATION
              ${INSTALL_CONFIGDIR})

export(EXPORT
       matrix_client-targets
       FILE
       ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientTargets.cmake
       NAMESPACE
       MatrixClient::)
export(PACKAGE MatrixClient)

set_property(TARGET matrix_client PROPERTY SOVERSION ${PROJECT_VERSION})

if(BUILD_LIB_TESTS)
  enable_testing()

  find_package(GTest REQUIRED)

  file(COPY tests/fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  add_executable(client_api tests/client_api.cpp)
  target_link_libraries(client_api
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)
  target_include_directories(client_api PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(media_api tests/media_api.cpp)
  target_link_libraries(media_api
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)
  target_include_directories(media_api PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(e2ee tests/e2ee.cpp)
  target_link_libraries(e2ee
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)
  target_include_directories(e2ee PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(utils tests/utils.cpp)
  target_link_libraries(utils
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)
  target_include_directories(utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(connection tests/connection.cpp)
  target_link_libraries(connection
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)
  target_include_directories(connection PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(identifiers tests/identifiers.cpp)
  target_link_libraries(identifiers
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_executable(events tests/events.cpp)
  target_link_libraries(events
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_executable(messages tests/messages.cpp)
  target_link_libraries(messages
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_executable(responses tests/responses.cpp)
  target_link_libraries(responses
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_executable(requests tests/requests.cpp)
  target_link_libraries(requests
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_executable(errors tests/errors.cpp)
  target_link_libraries(errors
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_executable(crypto tests/crypto.cpp)
  target_link_libraries(crypto
                        MatrixClient::MatrixClient
                        GTest::GTest
                        GTest::Main)

  add_test(BasicConnectivity connection)
  add_test(ClientAPI client_api)
  add_test(MediaAPI media_api)
  add_test(Encryption e2ee)
  add_test(Utilities utils)
  add_test(Identifiers identifiers)
  add_test(Errors errors)
  add_test(CryptoStructs crypto)
  add_test(StateEvents events)
  add_test(RoomEvents messages)
  add_test(Responses responses)
  add_test(Requests requests)
endif()
