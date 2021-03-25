cmake_minimum_required(VERSION 3.17)
project(Task_Scheduling_and_Memory_Management C)

set(CMAKE_C_STANDARD 11)

add_executable(Task_Scheduling_and_Memory_Management tsmm.c tsmm.h tst.cpp queue.c)