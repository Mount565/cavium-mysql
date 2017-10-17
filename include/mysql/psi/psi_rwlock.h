/* Copyright (c) 2008, 2017, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#ifndef MYSQL_PSI_RWLOCK_H
#define MYSQL_PSI_RWLOCK_H

/**
  @file include/mysql/psi/psi_rwlock.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_rwlock Rwlock Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "psi_base.h"
#include "mysql/components/services/psi_rwlock_bits.h"

/*
  MAINTAINER:
  The following pattern:
    typedef struct XYZ XYZ;
  is not needed in C++, but required for C.
*/

C_MODE_START

/** Entry point for the performance schema interface. */
struct PSI_rwlock_bootstrap
{
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_RWLOCK_VERSION_1
    @sa PSI_RWLOCK_VERSION_2
    @sa PSI_CURRENT_RWLOCK_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_rwlock_bootstrap PSI_rwlock_bootstrap;

#ifdef HAVE_PSI_RWLOCK_INTERFACE

/**
  Performance Schema Rwlock Interface, version 1.
  @since PSI_RWLOCK_VERSION_1
*/
struct PSI_rwlock_service_v1
{
  /** @sa register_rwlock_v1_t. */
  register_rwlock_v1_t register_rwlock;
  /** @sa init_rwlock_v1_t. */
  init_rwlock_v1_t init_rwlock;
  /** @sa destroy_rwlock_v1_t. */
  destroy_rwlock_v1_t destroy_rwlock;
  /** @sa start_rwlock_rdwait_v1_t. */
  start_rwlock_rdwait_v1_t start_rwlock_rdwait;
  /** @sa end_rwlock_rdwait_v1_t. */
  end_rwlock_rdwait_v1_t end_rwlock_rdwait;
  /** @sa start_rwlock_wrwait_v1_t. */
  start_rwlock_wrwait_v1_t start_rwlock_wrwait;
  /** @sa end_rwlock_wrwait_v1_t. */
  end_rwlock_wrwait_v1_t end_rwlock_wrwait;
  /** @sa unlock_rwlock_v1_t. */
  unlock_rwlock_v1_t unlock_rwlock;
};

typedef struct PSI_rwlock_service_v1 PSI_rwlock_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_rwlock_service_t *psi_rwlock_service;

#endif /* HAVE_PSI_RWLOCK_INTERFACE */

/** @} (end of group psi_abi_rwlock) */

C_MODE_END

#endif /* MYSQL_PSI_RWLOCK_H */
