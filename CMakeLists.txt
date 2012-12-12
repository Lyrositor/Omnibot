#
#   Copyright (C) 2012 Lyrositor
#
#   This file is part of Omnibot.
#
#   Omnibot is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Omnibot is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Omnibot.  If not, see <http://www.gnu.org/licenses/>.
#

project(Omnibot)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(HSPlasmaNet REQUIRED)
find_package(Readline REQUIRED)

set(Omnibot_SOURCES
    AuthClient.cpp
    ConsoleParser.cpp
    GameClient.cpp
    Omnibot.cpp
    main.cpp
    Python.cpp
    Utils.cpp
    Vault.cpp
    )

set(Omnibot_HEADERS
    AuthClient.h
    ConsoleParser.h
    GameClient.h
    Python.h
    Omnibot.h
    Utils.h
    Vault.h
    )

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++0x -pthread ${CMAKE_CXXFLAGS}")
endif()

include_directories("${CMAKE_SOURCE_DIR}"
                    "${HSPlasmaNet_INCLUDE_DIRS}"
                    "${READLINE_INCLUDE_DIR}"
                    )

source_group("Omnibot Sources" FILES ${Omnibot_SOURCES})
source_group("Omnibot Headers" FILES ${Omnibot_HEADERS})

add_executable(Omnibot
               ${all_SOURCES}
               ${Omnibot_SOURCES}
               ${Omnibot_HEADERS}
               )

target_link_libraries(Omnibot ${HSPlasmaNet_LIBRARIES} ${READLINE_LIBRARY})
