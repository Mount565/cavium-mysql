#ifndef MY_CACHE_LINE_INCLUDED
#define MY_CACHE_LINE_INCLUDED

/* Copyright (c) 2017, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <my_compiler.h>

/**
  @file include/my_cache_line.h

  Provide defaults for the CPU cache line size, if it has not been detected by
  CMake using getconf
*/

#if defined(CPU_LEVEL1_DCACHE_LINESIZE) && CPU_LEVEL1_DCACHE_LINESIZE != 0
#  define MY_CACHE_LINE_SIZE CPU_LEVEL1_DCACHE_LINESIZE
#elif defined(__powerpc__) || defined(__aarch64__)
#  define MY_CACHE_LINE_SIZE 128
#else
#  define MY_CACHE_LINE_SIZE 64
#endif

#define MY_CACHE_LINE_ALIGNED MY_ALIGNED(MY_CACHE_LINE_SIZE)

#endif /* MY_CACHE_LINE_INCLUDED */
