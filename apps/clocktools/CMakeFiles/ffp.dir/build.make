# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ww/pppbox

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ww/pppbox

# Include any dependencies generated for this target.
include apps/clocktools/CMakeFiles/ffp.dir/depend.make

# Include the progress variables for this target.
include apps/clocktools/CMakeFiles/ffp.dir/progress.make

# Include the compile flags for this target's objects.
include apps/clocktools/CMakeFiles/ffp.dir/flags.make

apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o: apps/clocktools/CMakeFiles/ffp.dir/flags.make
apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o: apps/clocktools/ffp.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ww/pppbox/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o"
	cd /home/ww/pppbox/apps/clocktools && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ffp.dir/ffp.cpp.o -c /home/ww/pppbox/apps/clocktools/ffp.cpp

apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ffp.dir/ffp.cpp.i"
	cd /home/ww/pppbox/apps/clocktools && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ww/pppbox/apps/clocktools/ffp.cpp > CMakeFiles/ffp.dir/ffp.cpp.i

apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ffp.dir/ffp.cpp.s"
	cd /home/ww/pppbox/apps/clocktools && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ww/pppbox/apps/clocktools/ffp.cpp -o CMakeFiles/ffp.dir/ffp.cpp.s

apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.requires:
.PHONY : apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.requires

apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.provides: apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.requires
	$(MAKE) -f apps/clocktools/CMakeFiles/ffp.dir/build.make apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.provides.build
.PHONY : apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.provides

apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.provides.build: apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o

# Object files for target ffp
ffp_OBJECTS = \
"CMakeFiles/ffp.dir/ffp.cpp.o"

# External object files for target ffp
ffp_EXTERNAL_OBJECTS =

apps/clocktools/ffp: apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o
apps/clocktools/ffp: apps/clocktools/CMakeFiles/ffp.dir/build.make
apps/clocktools/ffp: libpppbox.so
apps/clocktools/ffp: apps/clocktools/CMakeFiles/ffp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ffp"
	cd /home/ww/pppbox/apps/clocktools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ffp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
apps/clocktools/CMakeFiles/ffp.dir/build: apps/clocktools/ffp
.PHONY : apps/clocktools/CMakeFiles/ffp.dir/build

apps/clocktools/CMakeFiles/ffp.dir/requires: apps/clocktools/CMakeFiles/ffp.dir/ffp.cpp.o.requires
.PHONY : apps/clocktools/CMakeFiles/ffp.dir/requires

apps/clocktools/CMakeFiles/ffp.dir/clean:
	cd /home/ww/pppbox/apps/clocktools && $(CMAKE_COMMAND) -P CMakeFiles/ffp.dir/cmake_clean.cmake
.PHONY : apps/clocktools/CMakeFiles/ffp.dir/clean

apps/clocktools/CMakeFiles/ffp.dir/depend:
	cd /home/ww/pppbox && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ww/pppbox /home/ww/pppbox/apps/clocktools /home/ww/pppbox /home/ww/pppbox/apps/clocktools /home/ww/pppbox/apps/clocktools/CMakeFiles/ffp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/clocktools/CMakeFiles/ffp.dir/depend
