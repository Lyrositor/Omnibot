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

find_path(READLINE_INCLUDE_DIR readline.h
          /usr/include/readline
          /usr/local/include/readline
)

find_library(READLINE_LIBRARY
             NAMES readline libreadline
             PATH /usr/lib      /usr/local/lib
                  /usr/lib64    /usr/local/lib64
)

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    set(READLINE_FOUND TRUE)
endif()


if(READLINE_FOUND)
    if(NOT Readline_FIND_QUIETLY)
        message(STATUS "Found GNU Readline: ${READLINE_LIBRARY}")
    endif()
else()
   if(Readline_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find GNU Readline")
   endif()
endif()
