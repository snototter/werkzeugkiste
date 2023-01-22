function(setup_dependency_eigen)
    # Checks if Eigen is installed. If not, it will be fetched and linked
    find_package(Eigen3 3.3 QUIET NO_MODULE)
    if (TARGET Eigen3::Eigen)
        message(STATUS "[werkzeugkiste] Using locally available Eigen3.")
    else(TARGET Eigen3::Eigen)
        # Fetch the library:
        message(STATUS "[werkzeugkiste] Including Eigen3 via FetchContent.")
        include(FetchContent)
        FetchContent_Declare(
          Eigen
          GIT_REPOSITORY  https://gitlab.com/libeigen/eigen.git
          GIT_TAG         master
        )
        FetchContent_MakeAvailable(Eigen)
    endif(TARGET Eigen3::Eigen)
endfunction()

