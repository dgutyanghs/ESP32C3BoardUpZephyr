# 1. Specify the minimum CMake version required by Zephyr
cmake_minimum_required(VERSION 3.20.0)

# 2. Find and pull in the core Zephyr RTOS build system configs
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# 3. Define your project name (can be anything you choose)
project(my_esp32c3_project)

# 4. Link your source code files to the 'app' target
target_sources(app PRIVATE src/main.c)
