# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

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
CMAKE_SOURCE_DIR = /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build

# Utility rule file for curl-generate-wcurl.1.

# Include any custom commands dependencies for this target.
include external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/compiler_depend.make

# Include the progress variables for this target.
include external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/progress.make

external/curl/docs/CMakeFiles/curl-generate-wcurl.1: external/curl/docs/wcurl.1

external/curl/docs/wcurl.1: /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/external/curl/scripts/cd2nroff
external/curl/docs/wcurl.1: /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/external/curl/docs/wcurl.md
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating wcurl.1"
	cd /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/external/curl/docs && /usr/bin/perl /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/external/curl/scripts/cd2nroff wcurl.md > /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build/external/curl/docs/wcurl.1

external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/codegen:
.PHONY : external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/codegen

curl-generate-wcurl.1: external/curl/docs/CMakeFiles/curl-generate-wcurl.1
curl-generate-wcurl.1: external/curl/docs/wcurl.1
curl-generate-wcurl.1: external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/build.make
.PHONY : curl-generate-wcurl.1

# Rule to build all files generated by this target.
external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/build: curl-generate-wcurl.1
.PHONY : external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/build

external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/clean:
	cd /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build/external/curl/docs && $(CMAKE_COMMAND) -P CMakeFiles/curl-generate-wcurl.1.dir/cmake_clean.cmake
.PHONY : external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/clean

external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/depend:
	cd /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/external/curl/docs /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build/external/curl/docs /home/vxsperal/Downloads/song-lyrics-sentiment-analysis/scraper_cpp/build/external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : external/curl/docs/CMakeFiles/curl-generate-wcurl.1.dir/depend

