# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

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
CMAKE_COMMAND = /home/c402/.local/lib/python3.8/site-packages/cmake/data/bin/cmake

# The command to remove a file.
RM = /home/c402/.local/lib/python3.8/site-packages/cmake/data/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/c402/FinalVersion/Search/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/c402/FinalVersion/Search/src/build

# Include any dependencies generated for this target.
include CMakeFiles/GadgetSearcher.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/GadgetSearcher.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/GadgetSearcher.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/GadgetSearcher.dir/flags.make

CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o: CMakeFiles/GadgetSearcher.dir/flags.make
CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o: /home/c402/FinalVersion/Search/src/Interpreter.cpp
CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o: CMakeFiles/GadgetSearcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o -MF CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o.d -o CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o -c /home/c402/FinalVersion/Search/src/Interpreter.cpp

CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c402/FinalVersion/Search/src/Interpreter.cpp > CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.i

CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c402/FinalVersion/Search/src/Interpreter.cpp -o CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.s

CMakeFiles/GadgetSearcher.dir/Utils.cpp.o: CMakeFiles/GadgetSearcher.dir/flags.make
CMakeFiles/GadgetSearcher.dir/Utils.cpp.o: /home/c402/FinalVersion/Search/src/Utils.cpp
CMakeFiles/GadgetSearcher.dir/Utils.cpp.o: CMakeFiles/GadgetSearcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/GadgetSearcher.dir/Utils.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/GadgetSearcher.dir/Utils.cpp.o -MF CMakeFiles/GadgetSearcher.dir/Utils.cpp.o.d -o CMakeFiles/GadgetSearcher.dir/Utils.cpp.o -c /home/c402/FinalVersion/Search/src/Utils.cpp

CMakeFiles/GadgetSearcher.dir/Utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GadgetSearcher.dir/Utils.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c402/FinalVersion/Search/src/Utils.cpp > CMakeFiles/GadgetSearcher.dir/Utils.cpp.i

CMakeFiles/GadgetSearcher.dir/Utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GadgetSearcher.dir/Utils.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c402/FinalVersion/Search/src/Utils.cpp -o CMakeFiles/GadgetSearcher.dir/Utils.cpp.s

CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o: CMakeFiles/GadgetSearcher.dir/flags.make
CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o: /home/c402/FinalVersion/Search/src/LocationInfo.cpp
CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o: CMakeFiles/GadgetSearcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o -MF CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o.d -o CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o -c /home/c402/FinalVersion/Search/src/LocationInfo.cpp

CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c402/FinalVersion/Search/src/LocationInfo.cpp > CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.i

CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c402/FinalVersion/Search/src/LocationInfo.cpp -o CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.s

CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o: CMakeFiles/GadgetSearcher.dir/flags.make
CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o: /home/c402/FinalVersion/Search/src/GadgetInfo.cpp
CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o: CMakeFiles/GadgetSearcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o -MF CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o.d -o CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o -c /home/c402/FinalVersion/Search/src/GadgetInfo.cpp

CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c402/FinalVersion/Search/src/GadgetInfo.cpp > CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.i

CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c402/FinalVersion/Search/src/GadgetInfo.cpp -o CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.s

CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o: CMakeFiles/GadgetSearcher.dir/flags.make
CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o: /home/c402/FinalVersion/Search/src/BackTrace.cpp
CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o: CMakeFiles/GadgetSearcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o -MF CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o.d -o CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o -c /home/c402/FinalVersion/Search/src/BackTrace.cpp

CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c402/FinalVersion/Search/src/BackTrace.cpp > CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.i

CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c402/FinalVersion/Search/src/BackTrace.cpp -o CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.s

CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o: CMakeFiles/GadgetSearcher.dir/flags.make
CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o: /home/c402/FinalVersion/Search/src/GadgetSearcher.cpp
CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o: CMakeFiles/GadgetSearcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o -MF CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o.d -o CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o -c /home/c402/FinalVersion/Search/src/GadgetSearcher.cpp

CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c402/FinalVersion/Search/src/GadgetSearcher.cpp > CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.i

CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c402/FinalVersion/Search/src/GadgetSearcher.cpp -o CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.s

# Object files for target GadgetSearcher
GadgetSearcher_OBJECTS = \
"CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o" \
"CMakeFiles/GadgetSearcher.dir/Utils.cpp.o" \
"CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o" \
"CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o" \
"CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o" \
"CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o"

# External object files for target GadgetSearcher
GadgetSearcher_EXTERNAL_OBJECTS =

libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/Interpreter.cpp.o
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/Utils.cpp.o
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/LocationInfo.cpp.o
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/GadgetInfo.cpp.o
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/BackTrace.cpp.o
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/GadgetSearcher.cpp.o
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/build.make
libGadgetSearcher.so: CMakeFiles/GadgetSearcher.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/c402/FinalVersion/Search/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX shared library libGadgetSearcher.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/GadgetSearcher.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/GadgetSearcher.dir/build: libGadgetSearcher.so
.PHONY : CMakeFiles/GadgetSearcher.dir/build

CMakeFiles/GadgetSearcher.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/GadgetSearcher.dir/cmake_clean.cmake
.PHONY : CMakeFiles/GadgetSearcher.dir/clean

CMakeFiles/GadgetSearcher.dir/depend:
	cd /home/c402/FinalVersion/Search/src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/c402/FinalVersion/Search/src /home/c402/FinalVersion/Search/src /home/c402/FinalVersion/Search/src/build /home/c402/FinalVersion/Search/src/build /home/c402/FinalVersion/Search/src/build/CMakeFiles/GadgetSearcher.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/GadgetSearcher.dir/depend

