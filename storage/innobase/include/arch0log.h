/*****************************************************************************

Copyright (c) 2017, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************//**
@file include/arch0log.h
Innodb interface for log archive

*******************************************************/

#ifndef ARCH_LOG_INCLUDE
#define ARCH_LOG_INCLUDE

#include "arch0arch.h"

/** File Node Iterator callback
@param[in]	file_name	NULL terminated file name
@param[in]	file_size	size of file in bytes
@param[in]	read_offset	offset to start reading from
@param[in]	ctx		context passed by caller
@return error code */
using Log_Arch_Cbk = dberr_t (
	char*		file_name,
	ib_uint64_t	file_size,
	ib_uint64_t	read_offset,
	void*		ctx);

/** Redo Log archiver client context */
class Log_Arch_Client_Ctx
{
public:
	/** Constructor: Initialize elements */
	Log_Arch_Client_Ctx()
	:
	m_state(ARCH_CLIENT_STATE_INIT),
	m_group(nullptr),
	m_begin_lsn(LSN_MAX),
	m_end_lsn(LSN_MAX)
	{}

	/** Get redo file, header and trailer size
	@param[out]	file_sz		redo file size
	@param[out]	header_sz	redo header size
	@param[out]	trailer_sz	redo trailer size */
	void get_header_size(
		ib_uint64_t&	file_sz,
		uint&		header_sz,
		uint&		trailer_sz);

	/** Start redo log archiving
	@param[out]	header	redo header. Caller must allocate buffer.
	@param[in]	len	buffer length
	@return error code */
	dberr_t start(byte* header, uint len);

	/** Stop redo log archiving
	@param[out]	trailer	redo trailer. Caller must allocate buffer.
	@param[in]	len	buffer length
	@param[out]	offset	trailer block offset
	@return error code */
	dberr_t stop(byte* trailer, uint len, ib_uint64_t& offset);

	/** Get archived data file details
	@param[in]	cbk_func	callback called for each file
	@param[in]	ctx		callback function context
	@return error code */
	dberr_t get_files(Log_Arch_Cbk* cbk_func, void* ctx);

	/** Release archived data so that system can purge it */
	void release();

private:
	/** Archiver client state */
	Arch_Client_State	m_state;

	/** Archive group the client is attached to */
	Arch_Group*		m_group;

	/** Start LSN for archived data */
	lsn_t			m_begin_lsn;

	/** Stop LSN for archived data */
	lsn_t			m_end_lsn;
};

#endif /* ARCH_LOG_INCLUDE */
