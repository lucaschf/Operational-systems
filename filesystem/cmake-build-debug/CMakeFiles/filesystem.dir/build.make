# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

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

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.3.2\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.3.2\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/filesystem.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/filesystem.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/filesystem.dir/flags.make

CMakeFiles/filesystem.dir/filesystem.c.obj: CMakeFiles/filesystem.dir/flags.make
CMakeFiles/filesystem.dir/filesystem.c.obj: ../filesystem.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\cmake-build-debug\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/filesystem.dir/filesystem.c.obj"
	C:\PROGRA~2\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\filesystem.dir\filesystem.c.obj   -c "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\filesystem.c"

CMakeFiles/filesystem.dir/filesystem.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/filesystem.dir/filesystem.c.i"
	C:\PROGRA~2\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\filesystem.c" > CMakeFiles\filesystem.dir\filesystem.c.i

CMakeFiles/filesystem.dir/filesystem.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/filesystem.dir/filesystem.c.s"
	C:\PROGRA~2\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\filesystem.c" -o CMakeFiles\filesystem.dir\filesystem.c.s

# Object files for target filesystem
filesystem_OBJECTS = \
"CMakeFiles/filesystem.dir/filesystem.c.obj"

# External object files for target filesystem
filesystem_EXTERNAL_OBJECTS =

filesystem.exe: CMakeFiles/filesystem.dir/filesystem.c.obj
filesystem.exe: CMakeFiles/filesystem.dir/build.make
filesystem.exe: CMakeFiles/filesystem.dir/linklibs.rsp
filesystem.exe: CMakeFiles/filesystem.dir/objects1.rsp
filesystem.exe: CMakeFiles/filesystem.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\cmake-build-debug\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable filesystem.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\filesystem.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/filesystem.dir/build: filesystem.exe

.PHONY : CMakeFiles/filesystem.dir/build

CMakeFiles/filesystem.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\filesystem.dir\cmake_clean.cmake
.PHONY : CMakeFiles/filesystem.dir/clean

CMakeFiles/filesystem.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem" "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem" "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\cmake-build-debug" "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\cmake-build-debug" "F:\TSI\2nd period\Operational systems\Operational-systems\filesystem\cmake-build-debug\CMakeFiles\filesystem.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/filesystem.dir/depend

