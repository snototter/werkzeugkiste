function(setup_dependency_eigen)
    # Checks if Eigen is installed. If not, it will be fetched and linked
    find_package(Eigen3 QUIET)
    if(Eigen3_FOUND)
        message(STATUS "[werkzeugkiste] Using locally available Eigen.")
    else()
        # Fetch the library:
        message(STATUS "[werkzeugkiste] Including Eigen via FetchContent.")
        include(FetchContent)
        FetchContent_Declare(
            Eigen3
            https://gitlab.com/libeigen/eigen.git
            GIT_TAG refactor-cmake-init
            GIT_TAG master)
        FetchContent_MakeAvailable(Eigen3)
    endif()
endfunction()

