# MacOS needs 'ar' fix

<details>
<summary>TODO: Cannot build on MacOS</summary>
Can't dive into this yet. The CI runner (see (.github/workflows/ci.yml)[./.github/workflows/ci.yml]) aborts with:

```bash
/bin/bash -c brew install eigen

# Run cmake "--preset=ci-$("macos-12".split("-")[0])" -D BUILD_SHARED_LIBS=NO
  cmake "--preset=ci-$("macos-12".split("-")[0])" -D BUILD_SHARED_LIBS=NO
  shell: /usr/local/bin/pwsh -command ". '{0}'"
  env:
    CMAKE_BUILD_PARALLEL_LEVEL: 3
    ARCHFLAGS: -arch x86_64
    LD_LIBRARY_PATH: /usr/local/lib:
Preset CMake variables:

  CMAKE_BUILD_TYPE="Release"
  CMAKE_CXX_EXTENSIONS="OFF"
  CMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wcast-qual -Wformat=2 -Wundef -Werror=float-equal -Wshadow -Wcast-align -Wunused -Wnull-dereference -Wdouble-promotion -Wimplicit-fallthrough -Woverloaded-virtual -Wnon-virtual-dtor -Wold-style-cast"
  CMAKE_CXX_STANDARD="17"
  CMAKE_CXX_STANDARD_REQUIRED="ON"
  werkzeugkiste_DEVELOPER_MODE="ON"

-- The CXX compiler identification is AppleClang 14.0.0.14000029
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Applications/Xcode_14.2.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Performing Test COMPILER_HAS_HIDDEN_VISIBILITY
-- Performing Test COMPILER_HAS_HIDDEN_VISIBILITY - Success
-- Performing Test COMPILER_HAS_HIDDEN_INLINE_VISIBILITY
-- Performing Test COMPILER_HAS_HIDDEN_INLINE_VISIBILITY - Success
-- Performing Test COMPILER_HAS_DEPRECATED_ATTR
-- Performing Test COMPILER_HAS_DEPRECATED_ATTR - Success
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/runner/work/werkzeugkiste/werkzeugkiste/build

# Run cmake --build build --config Release -j 2
cmake --build build --config Release -j 2
  shell: /bin/bash -e {0}
  env:
    CMAKE_BUILD_PARALLEL_LEVEL: 3
    ARCHFLAGS: -arch x86_64
    LD_LIBRARY_PATH: /usr/local/lib:
[  2%] Building CXX object CMakeFiles/werkzeugkiste-geometry.dir/src/geometry/primitives.cpp.o
[  5%] Linking CXX static library libwerkzeugkiste-container.a
ar: no archive members specified
usage:  ar -d [-TLsv] archive file ...
	ar -m [-TLsv] archive file ...
	ar -m [-abiTLsv] position archive file ...
	ar -p [-TLsv] archive [file ...]
	ar -q [-cTLsv] archive file ...
	ar -r [-cuTLsv] archive file ...
	ar -r [-abciuTLsv] position archive file ...
	ar -t [-TLsv] archive [file ...]
	ar -x [-ouTLsv] archive [file ...]
make[2]: *** [libwerkzeugkiste-container.a] Error 1
make[1]: *** [CMakeFiles/werkzeugkiste-container.dir/all] Error 2
make[1]: *** Waiting for unfinished jobs....
[  8%] Building CXX object CMakeFiles/werkzeugkiste-geometry.dir/src/geometry/vector.cpp.o
[ 10%] Linking CXX static library libwerkzeugkiste-geometry.a
[ 10%] Built target werkzeugkiste-geometry
make: *** [all] Error 2
Error: Process completed with exit code 2.
```
</details>

# Windows needs fix for M_PI

<details>
<summary>TODO: Cannot build on Windows </summary>

CI fails:

```bash
Run cmake "--preset=ci-$("windows-2022".split("-")[0])" -D BUILD_SHARED_LIBS=NO
  cmake "--preset=ci-$("windows-2022".split("-")[0])" -D BUILD_SHARED_LIBS=NO
  shell: C:\Program Files\PowerShell\7\pwsh.EXE -command ". '{0}'"
  env:
    CMAKE_BUILD_PARALLEL_LEVEL: 2
    BOOST_ROOT: C:\hostedtoolcache\windows\Boost\1.77.0\x86_64
    PATH: C:\devel\install\bin;C:\ProgramData\chocolatey\lib\mingw\tools\install\mingw64\bin;C:\hostedtoolcache\windows\Boost\1.77.0\x86_64\lib64-msvc-14.2;C:\Program Files\MongoDB\Server\5.0\bin;C:\aliyun-cli;C:\vcpkg;C:\Program Files (x86)\NSIS\;C:\tools\zstd;C:\Program Files\Mercurial\;C:\hostedtoolcache\windows\stack\2.9.3\x64;C:\cabal\bin;C:\\ghcup\bin;C:\tools\ghc-9.4.4\bin;C:\Program Files\dotnet;C:\mysql\bin;C:\Program Files\R\R-4.2.2\bin\x64;C:\SeleniumWebDrivers\GeckoDriver;C:\Program Files (x86)\sbt\bin;C:\Program Files (x86)\GitHub CLI;C:\Program Files\Git\bin;C:\Program Files (x86)\pipx_bin;C:\npm\prefix;C:\hostedtoolcache\windows\go\1.17.13\x64\bin;C:\hostedtoolcache\windows\Python\3.9.13\x64\Scripts;C:\hostedtoolcache\windows\Python\3.9.13\x64;C:\hostedtoolcache\windows\Ruby\3.0.5\x64\bin;C:\tools\kotlinc\bin;C:\hostedtoolcache\windows\Java_Temurin-Hotspot_jdk\8.0.352-8\x64\bin;C:\Program Files\ImageMagick-7.1.0-Q16-HDRI;C:\Program Files (x86)\Microsoft SDKs\Azure\CLI2\wbin;C:\ProgramData\kind;C:\Program Files\Microsoft\jdk-11.0.12.7-hotspot\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\dotnet\;C:\ProgramData\Chocolatey\bin;C:\Program Files\PowerShell\7\;C:\Program Files\Microsoft\Web Platform Installer\;C:\Program Files\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\;C:\Program Files\Microsoft SQL Server\150\Tools\Binn\;C:\Program Files\OpenSSL\bin;C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;C:\Strawberry\perl\bin;C:\ProgramData\chocolatey\lib\pulumi\tools\Pulumi\bin;C:\Program Files\TortoiseSVN\bin;C:\Program Files\CMake\bin;C:\ProgramData\chocolatey\lib\maven\apache-maven-3.8.7\bin;C:\Program Files\Microsoft Service Fabric\bin\Fabric\Fabric.Code;C:\Program Files\Microsoft SDKs\Service Fabric\Tools\ServiceFabricLocalClusterManager;C:\Program Files\nodejs\;C:\Program Files\Git\cmd;C:\Program Files\Git\mingw64\bin;C:\Program Files\Git\usr\bin;C:\Program Files\GitHub CLI\;c:\tools\php;C:\Program Files (x86)\sbt\bin;C:\SeleniumWebDrivers\ChromeDriver\;C:\SeleniumWebDrivers\EdgeDriver\;C:\Program Files\Amazon\AWSCLIV2\;C:\Program Files\Amazon\SessionManagerPlugin\bin\;C:\Program Files\Amazon\AWSSAMCLI\bin\;C:\Program Files\Microsoft SQL Server\130\Tools\Binn\;C:\Program Files\LLVM\bin;C:\Users\runneradmin\.dotnet\tools;C:\Users\runneradmin\.cargo\bin;C:\Users\runneradmin\AppData\Local\Microsoft\WindowsApps
    PKG_CONFIG_PATH: C:\devel\install\lib\pkgconfig;
    GIT_DEPENDENCIES: eigenteam/eigen-git-mirror#3.3.7

Preset CMake variables:

  CMAKE_CXX_EXTENSIONS="OFF"
  CMAKE_CXX_FLAGS="/utf-8 /w14165 /w44242 /w44254 /w44263 /w34265 /w34287 /w44296 /w44365 /w44388 /w44464 /w14545 /w14546 /w14547 /w14549 /w14555 /w34619 /w34640 /w24826 /w14905 /w14906 /w14928 /w45038 /W4 /permissive- /volatile:iso /Zc:preprocessor /Zc:__cplusplus /Zc:externConstexpr /Zc:throwingNew /EHsc"
  CMAKE_CXX_STANDARD="17"
  CMAKE_CXX_STANDARD_REQUIRED="ON"
  werkzeugkiste_DEVELOPER_MODE="ON"

-- The CXX compiler identification is MSVC 19.34.31937.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.34.31933/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Performing Test COMPILER_HAS_DEPRECATED_ATTR
-- Performing Test COMPILER_HAS_DEPRECATED_ATTR - Failed
-- Performing Test COMPILER_HAS_DEPRECATED
-- Performing Test COMPILER_HAS_DEPRECATED - Success
-- Configuring done
-- Generating done
-- Build files have been written to: D:/a/werkzeugkiste/werkzeugkiste/build


Run cmake --build build --config Release -j 2
  cmake --build build --config Release -j 2
  shell: C:\Program Files\PowerShell\7\pwsh.EXE -command ". '{0}'"
  env:
    CMAKE_BUILD_PARALLEL_LEVEL: 2
    BOOST_ROOT: C:\hostedtoolcache\windows\Boost\1.77.0\x86_64
    PATH: C:\devel\install\bin;C:\ProgramData\chocolatey\lib\mingw\tools\install\mingw64\bin;C:\hostedtoolcache\windows\Boost\1.77.0\x86_64\lib64-msvc-14.2;C:\Program Files\MongoDB\Server\5.0\bin;C:\aliyun-cli;C:\vcpkg;C:\Program Files (x86)\NSIS\;C:\tools\zstd;C:\Program Files\Mercurial\;C:\hostedtoolcache\windows\stack\2.9.3\x64;C:\cabal\bin;C:\\ghcup\bin;C:\tools\ghc-9.4.4\bin;C:\Program Files\dotnet;C:\mysql\bin;C:\Program Files\R\R-4.2.2\bin\x64;C:\SeleniumWebDrivers\GeckoDriver;C:\Program Files (x86)\sbt\bin;C:\Program Files (x86)\GitHub CLI;C:\Program Files\Git\bin;C:\Program Files (x86)\pipx_bin;C:\npm\prefix;C:\hostedtoolcache\windows\go\1.17.13\x64\bin;C:\hostedtoolcache\windows\Python\3.9.13\x64\Scripts;C:\hostedtoolcache\windows\Python\3.9.13\x64;C:\hostedtoolcache\windows\Ruby\3.0.5\x64\bin;C:\tools\kotlinc\bin;C:\hostedtoolcache\windows\Java_Temurin-Hotspot_jdk\8.0.352-8\x64\bin;C:\Program Files\ImageMagick-7.1.0-Q16-HDRI;C:\Program Files (x86)\Microsoft SDKs\Azure\CLI2\wbin;C:\ProgramData\kind;C:\Program Files\Microsoft\jdk-11.0.12.7-hotspot\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\dotnet\;C:\ProgramData\Chocolatey\bin;C:\Program Files\PowerShell\7\;C:\Program Files\Microsoft\Web Platform Installer\;C:\Program Files\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\;C:\Program Files\Microsoft SQL Server\150\Tools\Binn\;C:\Program Files\OpenSSL\bin;C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;C:\Strawberry\perl\bin;C:\ProgramData\chocolatey\lib\pulumi\tools\Pulumi\bin;C:\Program Files\TortoiseSVN\bin;C:\Program Files\CMake\bin;C:\ProgramData\chocolatey\lib\maven\apache-maven-3.8.7\bin;C:\Program Files\Microsoft Service Fabric\bin\Fabric\Fabric.Code;C:\Program Files\Microsoft SDKs\Service Fabric\Tools\ServiceFabricLocalClusterManager;C:\Program Files\nodejs\;C:\Program Files\Git\cmd;C:\Program Files\Git\mingw64\bin;C:\Program Files\Git\usr\bin;C:\Program Files\GitHub CLI\;c:\tools\php;C:\Program Files (x86)\sbt\bin;C:\SeleniumWebDrivers\ChromeDriver\;C:\SeleniumWebDrivers\EdgeDriver\;C:\Program Files\Amazon\AWSCLIV2\;C:\Program Files\Amazon\SessionManagerPlugin\bin\;C:\Program Files\Amazon\AWSSAMCLI\bin\;C:\Program Files\Microsoft SQL Server\130\Tools\Binn\;C:\Program Files\LLVM\bin;C:\Users\runneradmin\.dotnet\tools;C:\Users\runneradmin\.cargo\bin;C:\Users\runneradmin\AppData\Local\Microsoft\WindowsApps
    PKG_CONFIG_PATH: C:\devel\install\lib\pkgconfig;
    GIT_DEPENDENCIES: eigenteam/eigen-git-mirror#3.3.7

MSBuild version 17.4.1+9a89d02ff for .NET Framework
  Checking Build System
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/CMakeLists.txt
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/examples/CMakeLists.txt
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/CMakeLists.txt
  container_example.cpp
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/CMakeLists.txt
  primitives.cpp
  strings.cpp
C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.34.31933\include\algorithm(3067,95): warning C4242: '=': conversion from 'int' to 'char', possible loss of data [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-strings.vcxproj]
D:\a\werkzeugkiste\werkzeugkiste\src\strings\strings.cpp(83,58): message : see reference to function template instantiation '_OutIt std::transform<std::_String_iterator<std::_String_val<std::_Simple_types<_Elem>>>,std::_String_iterator<std::_String_val<std::_Simple_types<_Elem>>>,int(__cdecl *)(int)>(const _InIt,const _InIt,_OutIt,_Fn)' being compiled [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-strings.vcxproj]
          with
          [
              _OutIt=std::_String_iterator<std::_String_val<std::_Simple_types<char>>>,
              _Elem=char,
              _InIt=std::_String_iterator<std::_String_val<std::_Simple_types<char>>>,
              _Fn=int (__cdecl *)(int)
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/primitives.h(75,27): error C3861: 'M_PI': identifier not found [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/primitives.h(126,2): message : see reference to class template instantiation 'werkzeugkiste::geometry::Circle_<T>' being compiled [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/primitives.h(75,27): error C2065: 'M_PI': undeclared identifier [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
  vector.cpp
LINK : fatal error LNK1181: cannot open input file '..\Release\werkzeugkiste-container.lib' [D:\a\werkzeugkiste\werkzeugkiste\build\examples\container.vcxproj]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(399,17): error C2664: 'bool werkzeugkiste::geometry::Vec<T,4>::IsClose<double>(const werkzeugkiste::geometry::Vec<double,4> &,Tp,Tp) const': cannot convert argument 1 from 'const werkzeugkiste::geometry::Vec<T,4>' to 'const werkzeugkiste::geometry::Vec<double,4> &' [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float,
              Tp=double
          ]
          and
          [
              T=float
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(399,26): message : Reason: cannot convert from 'const werkzeugkiste::geometry::Vec<T,4>' to 'const werkzeugkiste::geometry::Vec<double,4>' [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(399,26): message : No user-defined-conversion operator available that can perform this conversion, or the operator cannot be called [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(354,8): message : see declaration of 'werkzeugkiste::geometry::Vec<T,4>::IsClose' [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(399,17): message : while trying to match the argument list '(const werkzeugkiste::geometry::Vec<T,4>, double, double)' [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(390,1): message : while compiling class template member function 'bool werkzeugkiste::geometry::operator ==(const werkzeugkiste::geometry::Vec<T,4> &,const werkzeugkiste::geometry::Vec<T,4> &)' [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(405,5): message : see reference to function template instantiation 'bool werkzeugkiste::geometry::operator ==(const werkzeugkiste::geometry::Vec<T,4> &,const werkzeugkiste::geometry::Vec<T,4> &)' being compiled [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float
          ]
D:\a\werkzeugkiste\werkzeugkiste\include\werkzeugkiste/geometry/vector.h(879,57): message : see reference to class template instantiation 'werkzeugkiste::geometry::Vec<T,4>' being compiled [D:\a\werkzeugkiste\werkzeugkiste\build\werkzeugkiste-geometry.vcxproj]
          with
          [
              T=float
          ]
  Generating Code...
  werkzeugkiste-strings.vcxproj -> D:\a\werkzeugkiste\werkzeugkiste\build\Release\werkzeugkiste-strings.lib
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/CMakeLists.txt
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/examples/CMakeLists.txt
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/CMakeLists.txt
  fileio.cpp
  stopwatch.cpp
  filesys.cpp
  tictoc.cpp
  Generating Code...
  strings_example.cpp
  Generating Code...
  werkzeugkiste-files.vcxproj -> D:\a\werkzeugkiste\werkzeugkiste\build\Release\werkzeugkiste-files.lib
  werkzeugkiste-timing.vcxproj -> D:\a\werkzeugkiste\werkzeugkiste\build\Release\werkzeugkiste-timing.lib
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/examples/CMakeLists.txt
  timing_example.cpp
  strings.vcxproj -> D:\a\werkzeugkiste\werkzeugkiste\build\examples\Release\strings.exe
  Building Custom Rule D:/a/werkzeugkiste/werkzeugkiste/examples/CMakeLists.txt
  files_example.cpp
  timing.vcxproj -> D:\a\werkzeugkiste\werkzeugkiste\build\examples\Release\timing.exe
  files.vcxproj -> D:\a\werkzeugkiste\werkzeugkiste\build\examples\Release\files.exe
Error: Process completed with exit code 1.
```

Additionally, Eigen setup takes 8+ minutes, need to switch to vcpkg or another package distribution
```bash
Run jrl-umi3218/github-actions/install-dependencies@master
  with:
    ubuntu: apt: libeigen3-dev

    macos: brew: eigen

    windows: github:
  - path: eigenteam/eigen-git-mirror
    ref: 3.3.7

    build-type: RelWithDebInfo
    compiler: gcc
C:\Windows\System32\WindowsPowerShell\v1.0\\powershell.exe D:\a\_actions\jrl-umi3218\github-actions\master\utils\get-boost.ps1
Download succeeded after 1 tries


Algorithm : SHA256
Hash      : 8CF6BB866FF83922B19A7418E94BC1FCE87CAE0DEA6077624E44DE3C6E27505A
Path      : C:\Users\RUNNER~1\AppData\Local\Temp\boost.exe



Install Windows specific GitHub dependencies
Install GitHub dependencies
Building eigenteam/eigen-git-mirror
--> Cloning eigenteam/eigen-git-mirror
Modified PATH variable

--> Configure eigenteam/eigen-git-mirror
--> Building eigenteam/eigen-git-mirror
--> Install eigenteam/eigen-git-mirror
```
</details>
