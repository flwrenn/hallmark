cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hallmark)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE
  src/main.c
  src/rapid_trigger.c
  src/home_row_mod.c
  src/socd.c
  src/keymap_config.c
)
