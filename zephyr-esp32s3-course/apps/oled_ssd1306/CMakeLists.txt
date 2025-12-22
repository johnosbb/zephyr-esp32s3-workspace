cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(oled_ssd1306 LANGUAGES C)

target_sources(app PRIVATE src/main.c)
