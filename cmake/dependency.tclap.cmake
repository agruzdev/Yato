# TCLAP dependency
# https://sourceforge.net/projects/tclap/

if(NOT TARGET Tclap)

    include(ExternalProject)

    set(TCLAP_ROOT "${CMAKE_SOURCE_DIR}/dependencies/tclap")

    ExternalProject_Add(Tclap
        PREFIX "${TCLAP_ROOT}"
        URL "https://sourceforge.net/projects/tclap/files/tclap-1.2.2.tar.gz/download"
        URL_HASH "SHA1=e07cb13a6849b21189865f74e447d373b325d577"
        CONFIGURE_COMMAND ""  # Skip
        BUILD_COMMAND ""      # Skip
        INSTALL_COMMAND ""    # Skip
        TEST_COMMAND ""       # Skip
    )

    ExternalProject_Get_Property(Tclap SOURCE_DIR)
    set(DEPENDS_TCLAP_INCLUDE_DIR "${SOURCE_DIR}/include")

    unset(SOURCE_DIR)
    unset(TCLAP_ROOT)

endif()


