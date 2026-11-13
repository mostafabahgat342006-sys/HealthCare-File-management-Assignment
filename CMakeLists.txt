cmake_minimum_required(VERSION 4.3)
project(HealthCare_File_management_Assignment)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(HealthCare_File_management_Assignment
        Appointment.h
        CRUD.cpp
        CRUD.h
        Doctor.h
        FileManager.cpp
        FileManager.h
        Integration.cpp
        Integration.h
        main.cpp
        PrimaryIndex.cpp
        PrimaryIndex.h
        QueryEngine.cpp
        QueryEngine.h
        SecondaryIndex.cpp
        SecondaryIndex.h
)