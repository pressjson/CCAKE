# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /CCAKE

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /CCAKE/build

# Include any dependencies generated for this target.
include CMakeFiles/ccake.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ccake.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ccake.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ccake.dir/flags.make

CMakeFiles/ccake.dir/src/ccake.cpp.o: CMakeFiles/ccake.dir/flags.make
CMakeFiles/ccake.dir/src/ccake.cpp.o: ../src/ccake.cpp
CMakeFiles/ccake.dir/src/ccake.cpp.o: CMakeFiles/ccake.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/CCAKE/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ccake.dir/src/ccake.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ccake.dir/src/ccake.cpp.o -MF CMakeFiles/ccake.dir/src/ccake.cpp.o.d -o CMakeFiles/ccake.dir/src/ccake.cpp.o -c /CCAKE/src/ccake.cpp

CMakeFiles/ccake.dir/src/ccake.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ccake.dir/src/ccake.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /CCAKE/src/ccake.cpp > CMakeFiles/ccake.dir/src/ccake.cpp.i

CMakeFiles/ccake.dir/src/ccake.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ccake.dir/src/ccake.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /CCAKE/src/ccake.cpp -o CMakeFiles/ccake.dir/src/ccake.cpp.s

# Object files for target ccake
ccake_OBJECTS = \
"CMakeFiles/ccake.dir/src/ccake.cpp.o"

# External object files for target ccake
ccake_EXTERNAL_OBJECTS = \
"/CCAKE/build/src/CMakeFiles/src.dir/BSQHydro.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/eos.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/eos_interpolator.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/eos_extension.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/eos_table.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/evolver.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/formatted_output.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/kernel.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/matrix.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/particle.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/rootfinder.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/eom_default.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/sph_workstation.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/system_state.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/transport_coefficients.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/vector.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/utilities.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/input.cpp.o" \
"/CCAKE/build/src/CMakeFiles/src.dir/output.cpp.o"

ccake: CMakeFiles/ccake.dir/src/ccake.cpp.o
ccake: src/CMakeFiles/src.dir/BSQHydro.cpp.o
ccake: src/CMakeFiles/src.dir/eos.cpp.o
ccake: src/CMakeFiles/src.dir/eos_interpolator.cpp.o
ccake: src/CMakeFiles/src.dir/eos_extension.cpp.o
ccake: src/CMakeFiles/src.dir/eos_table.cpp.o
ccake: src/CMakeFiles/src.dir/evolver.cpp.o
ccake: src/CMakeFiles/src.dir/formatted_output.cpp.o
ccake: src/CMakeFiles/src.dir/kernel.cpp.o
ccake: src/CMakeFiles/src.dir/matrix.cpp.o
ccake: src/CMakeFiles/src.dir/particle.cpp.o
ccake: src/CMakeFiles/src.dir/rootfinder.cpp.o
ccake: src/CMakeFiles/src.dir/eom_default.cpp.o
ccake: src/CMakeFiles/src.dir/sph_workstation.cpp.o
ccake: src/CMakeFiles/src.dir/system_state.cpp.o
ccake: src/CMakeFiles/src.dir/transport_coefficients.cpp.o
ccake: src/CMakeFiles/src.dir/vector.cpp.o
ccake: src/CMakeFiles/src.dir/utilities.cpp.o
ccake: src/CMakeFiles/src.dir/input.cpp.o
ccake: src/CMakeFiles/src.dir/output.cpp.o
ccake: CMakeFiles/ccake.dir/build.make
ccake: /usr/lib/x86_64-linux-gnu/libgsl.so
ccake: /usr/lib/x86_64-linux-gnu/libgslcblas.so
ccake: /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5_cpp.so
ccake: /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so
ccake: /usr/lib/x86_64-linux-gnu/libcrypto.so
ccake: /usr/lib/x86_64-linux-gnu/libcurl.so
ccake: /usr/lib/x86_64-linux-gnu/libsz.so
ccake: /usr/lib/x86_64-linux-gnu/libz.so
ccake: /usr/lib/x86_64-linux-gnu/libdl.a
ccake: /usr/lib/x86_64-linux-gnu/libm.so
ccake: /usr/local/lib/libkokkoscontainers.a
ccake: /usr/local/lib/libkokkoscore.a
ccake: /usr/lib/gcc/x86_64-linux-gnu/11/libgomp.so
ccake: /usr/lib/x86_64-linux-gnu/libpthread.a
ccake: /usr/local/lib/libkokkossimd.a
ccake: /usr/lib/x86_64-linux-gnu/libyaml-cpp.so.0.7.0
ccake: CMakeFiles/ccake.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/CCAKE/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ccake"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ccake.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ccake.dir/build: ccake
.PHONY : CMakeFiles/ccake.dir/build

CMakeFiles/ccake.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ccake.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ccake.dir/clean

CMakeFiles/ccake.dir/depend:
	cd /CCAKE/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /CCAKE /CCAKE /CCAKE/build /CCAKE/build /CCAKE/build/CMakeFiles/ccake.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ccake.dir/depend
