# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /autofs/nccs-svm1_sw/summit/spack-envs/base/opt/linux-rhel8-ppc64le/gcc-9.3.0/cmake-3.18.4-a6tr5stdc7okxeyuurraptwpc77ns5lx/bin/cmake

# The command to remove a file.
RM = /autofs/nccs-svm1_sw/summit/spack-envs/base/opt/linux-rhel8-ppc64le/gcc-9.3.0/cmake-3.18.4-a6tr5stdc7okxeyuurraptwpc77ns5lx/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build

# Include any dependencies generated for this target.
include test/CMakeFiles/test_refactor.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test_refactor.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test_refactor.dir/flags.make

test/CMakeFiles/test_refactor.dir/test_refactor.cpp.o: test/CMakeFiles/test_refactor.dir/flags.make
test/CMakeFiles/test_refactor.dir/test_refactor.cpp.o: ../test/test_refactor.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/test_refactor.dir/test_refactor.cpp.o"
	cd /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test && /sw/summit/gcc/9.3.0-2/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_refactor.dir/test_refactor.cpp.o -c /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/test/test_refactor.cpp

test/CMakeFiles/test_refactor.dir/test_refactor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_refactor.dir/test_refactor.cpp.i"
	cd /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test && /sw/summit/gcc/9.3.0-2/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/test/test_refactor.cpp > CMakeFiles/test_refactor.dir/test_refactor.cpp.i

test/CMakeFiles/test_refactor.dir/test_refactor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_refactor.dir/test_refactor.cpp.s"
	cd /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test && /sw/summit/gcc/9.3.0-2/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/test/test_refactor.cpp -o CMakeFiles/test_refactor.dir/test_refactor.cpp.s

# Object files for target test_refactor
test_refactor_OBJECTS = \
"CMakeFiles/test_refactor.dir/test_refactor.cpp.o"

# External object files for target test_refactor
test_refactor_EXTERNAL_OBJECTS =

test/test_refactor: test/CMakeFiles/test_refactor.dir/test_refactor.cpp.o
test/test_refactor: test/CMakeFiles/test_refactor.dir/build.make
test/test_refactor: ../external/SZ3/build/src/libSZ3.so
test/test_refactor: /sw/summit/spack-envs/base/opt/linux-rhel8-ppc64le/gcc-9.3.0/zstd-1.5.0-nyznyntrichkgdzue3lceevqqgvyzcau/lib/libzstd.so
test/test_refactor: test/CMakeFiles/test_refactor.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test_refactor"
	cd /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_refactor.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test_refactor.dir/build: test/test_refactor

.PHONY : test/CMakeFiles/test_refactor.dir/build

test/CMakeFiles/test_refactor.dir/clean:
	cd /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test && $(CMAKE_COMMAND) -P CMakeFiles/test_refactor.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test_refactor.dir/clean

test/CMakeFiles/test_refactor.dir/depend:
	cd /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/test /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test /gpfs/alpine/proj-shared/csc143/jwang/DNN_Mgard/ExternalDependencies/Multiprecision-data-refactoring/build/test/CMakeFiles/test_refactor.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test_refactor.dir/depend

