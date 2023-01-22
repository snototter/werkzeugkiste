#TODO couldn't set it up properly, there are still deprecation warnings / errors
# libeigen must be installed on the system!
#function(setup_dependency_eigen)
    # Checks if Eigen is installed. If not, it will be fetched and linked
    #find_package(Eigen3 3.3 QUIET NO_MODULE)
    #if (TARGET Eigen3::Eigen)
    #    message(STATUS "[werkzeugkiste] Using locally available Eigen3.")
    #else(TARGET Eigen3::Eigen)
    #    # Fetch the library:
    #    message(STATUS "[werkzeugkiste] Including Eigen3 via FetchContent.")
    #    set(EIGEN_BUILD_DOC OFF)
    #    set(EIGEN_BUILD_PKGCONFIG OFF)
    #    set(EIGEN_BUILD_SPBENCH OFF)
    #    set(EIGEN_BUILD_TESTING OFF)
    #    set(EIGEN_LEAVE_TEST_IN_ALL_TARGET OFF)
    #    # Disable qt-based demos (there is no option to disable all demos).
    #    # Otherwise, the deprecated qt4 package will raise a CMake error
    #    set(EIGEN_TEST_NOQT ON)
    #    include(FetchContent)
    #    FetchContent_Declare(
    #      Eigen
    #      GIT_REPOSITORY  https://gitlab.com/libeigen/eigen.git
    #      GIT_TAG         master
    #    )
    #    FetchContent_MakeAvailable(Eigen)
    #endif(TARGET Eigen3::Eigen)
#endfunction()

