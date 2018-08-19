#
# YATO library
#
# The MIT License (MIT)
# Copyright (c) 2018 Alexey Gruzdev
#

# Check that directory exists and is not empty
function(dependency_folder_not_empty PATH_ RESULT_)
    set(res_ FALSE)
    if(EXISTS ${PATH_})
        file(GLOB files_ "${PATH_}/*")
        if(files_)
            set(res_ TRUE)
        endif()
        unset(files_)
    endif()
    set(${RESULT_} ${res_} PARENT_SCOPE)
endfunction()

# Check that file exists and has valid hash
function(file_exists STATUS_ FILE_ HASH_MD5_)
    set(STATUS_ FALSE)
    if(EXISTS ${FILE_})
        file(MD5 ${FILE_} hash_)
        if(${hash_} STREQUAL ${HASH_MD5_})
            set(STATUS_ TRUE)
        endif()
    endif()
endfunction()


function(dependency_download_and_unzip URL_ HASH_MD5_ FILE_ UNZIPPED_DIR_)
    get_filename_component(workdir_ ${FILE_} DIRECTORY)
    set(unzipped_directory ${workdir_}/unzipped)

    dependency_folder_not_empty(${unzipped_directory} unzip_exists_)
    if(NOT ${unzip_exists_})
        file_exists(archive_is_valid_ ${FILE_} ${HASH_MD5_})
        if(NOT ${archive_is_valid_})
            message(STATUS "Downloading: ${URL_}")
            file(DOWNLOAD ${URL_} ${FILE_}
                SHOW_PROGRESS
                EXPECTED_MD5 ${HASH_MD5_}
                STATUS ret_
            )
            if(NOT ret_)
                message(FATAL_ERROR "Failed to download: ${URL_}")
            endif()
        endif()

        message(STATUS "Unzipping: ${FILE_}")
        file(MAKE_DIRECTORY ${unzipped_directory})
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar -xzf ${FILE_}
            WORKING_DIRECTORY ${unzipped_directory}
            RESULT_VARIABLE ret_
        )
        if(ret_)
            message(FATAL_ERROR "Failed to extract: ${FILE_}")
        endif()
    endif()

    set(${UNZIPPED_DIR_} ${unzipped_directory} PARENT_SCOPE)
endfunction()



#function(dependency_find_or_download
#    DEPENDENCY_NAME           # Short dependency name used for cache variables
#    DEPENDENCY_VERBOSE_NAME   # Verbose name for messages and hints
#    DEP_URL                   # Url for downloading.
#    DEP_HASH_MD5              # Hash of downloaded file
#    FILE_NAME                 # Downloaded file name
#    
#    DOWNLOADED_PREFIX         # Additonal path prefix after extracting archive
#   
#    OUT_INVALIDATED           # Out flag. ON if dependency should be configured. OFF if cache is valid.
#)
#
# Use global flag DOWNLOAD_ALL to enable downloading by default

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
    if(${DEPENDENCY_NAME}_DOWNLOAD OR DOWNLOAD_ALL)
        string(TOLOWER ${DEPENDENCY_NAME} folder_name_)
        dependency_download_and_unzip(${DEPENDENCY_URL} ${DEPENDENCY_HASH_MD5} 
            "${YATO_SOURCE_DIR}/dependencies/${folder_name_}/package.zip"
            unzipped_location_
        )
        set(root_ ${unzipped_location_})
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

