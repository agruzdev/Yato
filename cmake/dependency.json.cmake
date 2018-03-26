# JSON dependency
# https://github.com/nlohmann/json

if(NOT TARGET Json)

    include(ExternalProject)

    set(JSON_ROOT "${CMAKE_SOURCE_DIR}/dependencies/json")

    ExternalProject_Add(Json
        PREFIX "${JSON_ROOT}"
        URL "https://github.com/nlohmann/json/releases/download/v3.1.1/include.zip"
        URL_HASH "SHA256=fde771d4b9e4f222965c00758a2bdd627d04fb7b59e09b7f3d1965abdc848505"
        CONFIGURE_COMMAND ""  # Skip
        BUILD_COMMAND ""      # Skip
        INSTALL_COMMAND ""    # Skip
        TEST_COMMAND ""       # Skip
    )

    ExternalProject_Get_Property(Json SOURCE_DIR)
    set(DEPENDS_JSON_INCLUDE_DIR ${SOURCE_DIR})

    unset(SOURCE_DIR)
    unset(JSON_ROOT)

endif()


