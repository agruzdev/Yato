#
# YATO library
#
# The MIT License (MIT)
# Copyright (c) 2018 Alexey Gruzdev
#

function(dependency_download_and_unzip URL_ HASH_MD5_ FILE_)
    message(STATUS "Downloading: ${URL_}")
    file(DOWNLOAD ${URL_} ${FILE_}
        SHOW_PROGRESS
        EXPECTED_MD5 ${HASH_MD5_}
        STATUS ret_
    )
    if(NOT ret_)
        message(FATAL_ERROR "Failed to download: ${URL_}")
    endif()

    message(STATUS "Unzipping: ${FILE_}")
    get_filename_component(workdir_ ${FILE_} DIRECTORY)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar -xzf ${FILE_}
        WORKING_DIRECTORY ${workdir_}
        RESULT_VARIABLE ret_
    )
    if(ret_)
        message(FATAL_ERROR "Failed to extract: ${FILE_}")
    endif()
endfunction()



#function(dependency_find_or_download
#    DEPENDENCY_NAME           # Short dependency name used for cache variables
#    DEPENDENCY_VERBOSE_NAME   # Verbose name for messages and hints
#    DEP_URL           # Url for downloading.
#    DEP_HASH_MD5      # Hash of downloaded file
#    FILE_NAME         # Downloaded file name
#    
#    DOWNLOADED_PREFIX     # Additonal path prefix after extracting archive
#
#    OUT_INVALIDATED       # Out flag. ON if dependency should be configured. OFF if cache is valid.
#)

function(dependency_find_or_download)
    set(oneValueArgs
        NAME            # Short dependency name used for cache variables
        VERBOSE_NAME    # Verbose name for messages and tooltips
        URL             # Url for downloading.
        HASH_MD5        # Hash of downloaded file
        FILE_NAME       # [Optional] specify downloaded file name
        PREFIX          # [Optional] Additonal path prefix after extracting archive
    )
    cmake_parse_arguments(DEPENDENCY "" "${oneValueArgs}" "" ${ARGN})

    option(${DEPENDENCY_NAME}_DOWNLOAD "Download ${DEPENDENCY_VERBOSE_NAME} sources" OFF)
    if(${DEPENDENCY_NAME}_DOWNLOAD)
        string(TOLOWER ${DEPENDENCY_NAME} folder_name_)
        if(DEPENDENCY_FILE_NAME)
            set(file_name_ ${DEPENDENCY_FILE_NAME})
        else()
            set(file_name_ "package.zip")
        endif()
        dependency_download_and_unzip(${DEPENDENCY_URL} ${DEPENDENCY_HASH_MD5} 
            "${CMAKE_SOURCE_DIR}/dependencies/${folder_name_}/${file_name_}"
        )
        set(root_ ${CMAKE_SOURCE_DIR}/dependencies/${folder_name_})
        if(DEPENDENCY_PREFIX)
            set(root_ ${root_}/${DEPENDENCY_PREFIX})
        endif()
    else()
        if(${DEPENDENCY_NAME}_ROOT)
            set(root_ ${${DEPENDENCY_NAME}_ROOT})
        else()
            # Check environment
            set(root_ $ENV{${DEPENDENCY_NAME}_ROOT})
        endif()
    endif()

    if(NOT EXISTS ${root_})
        message(FATAL_ERROR "${DEPENDENCY_VERBOSE_NAME} is not found! Please set ${DEPENDENCY_NAME}_ROOT variable to source directory or enable flag ${DEPENDENCY_NAME}_DOWNLOAD for automatic downloading of the dependency.")
    endif()

    set(${DEPENDENCY_NAME}_FOUND_ROOT ${root_} CACHE PATH "Found ${DEPENDENCY_VERBOSE_NAME} root directory. Read-only variable. Set ${DEPENDENCY_NAME}_ROOT to change the root directory." FORCE)
endfunction()

