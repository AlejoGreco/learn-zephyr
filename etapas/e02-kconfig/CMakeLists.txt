cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(e02_kconfig)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE src/main.c src/saludos.c)
target_sources_ifdef(CONFIG_FEATURE_CONDITIONAL app PRIVATE src/feat_conditional.c)
