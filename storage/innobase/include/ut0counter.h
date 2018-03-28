/*****************************************************************************

Copyright (c) 2012, 2017, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**************************************************//**
@file include/ut0counter.h

Counter utility class

Created 2012/04/12 by Sunny Bains
*******************************************************/

#ifndef ut0counter_h
#define ut0counter_h

#include <my_rdtsc.h>
#include "univ.i"
#include "os0thread.h"
#include "ut0rnd.h"
#include "os0atomic.h"

/** Default number of slots to use in ib_counter_t */
#define IB_N_SLOTS		64

/** Get the offset into the counter array. */
template <typename Type, int N>
struct generic_indexer_t {
	/** Default constructor/destructor should be OK. */

        /** @return offset within m_counter */
        static size_t offset(size_t index) UNIV_NOTHROW
	{
                return(((index % N) + 1) *
                      (INNOBASE_CACHE_LINE_SIZE / sizeof(Type)));
        }
};

#ifdef HAVE_SCHED_GETCPU
#include <sched.h>
/** Use the cpu id to index into the counter array. If it fails then
use the thread id. */
template <typename Type=ulint, int N=1>
struct get_sched_indexer_t : public generic_indexer_t<Type, N> {
	/** Default constructor/destructor should be OK. */

	enum { fast = 1 };

	/* @return result from sched_getcpu(), the thread id if it fails. */
	static size_t get_rnd_index() UNIV_NOTHROW {

		int	cpu = sched_getcpu();

		if (cpu == -1) {
			cpu = (int) os_thread_get_curr_id();
		}

		return(size_t(cpu));
	}
};
#elif defined(_WIN32)
template <typename Type=ulint, int N=1>
struct get_sched_indexer_t : public generic_indexer_t<Type, N> {
 	/** Default constructor/destructor should be OK. */

	enum { fast = 1 };

	/* @return result from GetCurrentProcessorNumber (). */
	static size_t get_rnd_index() UNIV_NOTHROW {

		/* According to the Windows documentation, it returns the
		processor number within the Processor group if the host
		has more than 64 logical CPUs. We ignore that here. If the
		processor number from a different group maps to the same
		slot that is acceptable. We want to avoid making another
		system call to determine the processor group. If this becomes
		an issue, the fix is to multiply the group with the processor
		number and use that, also note that the number of slots should
		be increased to avoid overlap. */
		return(size_t(GetCurrentProcessorNumber()));
	}
};
#endif /* HAVE_SCHED_GETCPU */

#ifndef UNIV_INNOCHECKSUM

/** Use the random number to index into the counter array. */
template <typename Type=ulint, int N=1>
struct get_rand_indexer_t : public generic_indexer_t<Type, N> {
	/** Default constructor/destructor should be OK. */

	enum { fast = 1 };

	/* @return result from ut_rnd_gen_ulint(). */
	static size_t get_rnd_index() UNIV_NOTHROW {

		ulint	idx = ut_rnd_gen_ulint();

		return(static_cast<size_t>(idx));
	}
};

#endif /* !UNIV_INNOCHECKSUM */

/** Use the result of my_timer_cycles(), which mainly uses RDTSC for cycles,
to index into the counter array. See the comments for my_timer_cycles() */
template <typename Type=ulint, int N=1>
struct counter_indexer_t : public generic_indexer_t<Type, N> {

	/** Default constructor/destructor should be OK. */

	enum { fast = 1 };

	/** @return result from RDTSC or similar functions. */
	static size_t get_rnd_index() UNIV_NOTHROW
	{
		size_t	c = static_cast<size_t>(my_timer_cycles());

		if (c != 0) {
			return(c);
		} else {
			/* We may go here if my_timer_cycles() returns 0,
			so we have to have the plan B for the counter. */
#if !defined(_WIN32)
			return(size_t(os_thread_get_curr_id()));
#else
			LARGE_INTEGER cnt;
			QueryPerformanceCounter(&cnt);

			return(static_cast<size_t>(cnt.QuadPart));
#endif /* !_WIN32 */
		}
	}
};

/** For counters where N=1 */
template <typename Type=ulint, int N=1>
struct single_indexer_t {
	/** Default constructor/destructor should are OK. */

	enum { fast = 0 };

        /** @return offset within m_counter */
        static size_t offset(size_t index) UNIV_NOTHROW
	{
		ut_ad(N == 1);
                return((INNOBASE_CACHE_LINE_SIZE / sizeof(Type)));
        }

	/** @return 1 */
	static size_t get_rnd_index() UNIV_NOTHROW
	{
		ut_ad(N == 1);
		return(1);
	}
};

#if defined(__aarch64__) && defined(UNIV_LINUX) && !defined(UNIV_INNOCHECKSUM)
# define default_indexer_t	get_rand_indexer_t
#elif defined(HAVE_SCHED_GETCPU) || defined(_WIN32)
# define default_indexer_t	get_sched_indexer_t
#else
# define default_indexer_t	counter_indexer_t
#endif


template <typename T>
UNIV_INLINE void add_noreturn(T &val, T n) {
	val += n;
}

template <typename T>
UNIV_INLINE void sub_noreturn(T &val, T n) {
	val -= n;
}

/* Template specializations for native word size */
template <>
inline void add_noreturn<ulint>(ulint &val, ulint n) {
	os_nonatomic_increment_ulint_nr(&val, n);
}

template <>
inline void sub_noreturn<ulint>(ulint &val, ulint n) {
	os_nonatomic_decrement_lint_nr(&val, n);
}

template <>
inline void add_noreturn<lint>(lint &val, lint n) {
	os_nonatomic_increment_lint_nr(&val, n);
}

template <>
inline void sub_noreturn<lint>(lint &val, lint n) {
	os_nonatomic_decrement_lint_nr(&val, n);
}

/** Class for using fuzzy counters. The counter is not protected by any
mutex and the results are not guaranteed to be 100% accurate but close
enough. Creates an array of counters and separates each element by the
INNOBASE_CACHE_LINE_SIZE bytes */
template <
	typename Type,
	int N = IB_N_SLOTS,
	template<typename, int> class Indexer = default_indexer_t>
class ib_counter_t {
public:
	ib_counter_t() { memset(m_counter, 0x0, sizeof(m_counter)); }

	~ib_counter_t()
	{
		ut_ad(validate());
	}

	static bool is_fast() { return(Indexer<Type, N>::fast); }

	bool validate() UNIV_NOTHROW {
#ifdef UNIV_DEBUG
		size_t	n = (INNOBASE_CACHE_LINE_SIZE / sizeof(Type));

		/* Check that we aren't writing outside our defined bounds. */
		for (size_t i = 0; i < UT_ARR_SIZE(m_counter); i += n) {
			for (size_t j = 1; j < n - 1; ++j) {
				ut_ad(m_counter[i + j] == 0);
			}
		}
#endif /* UNIV_DEBUG */
		return(true);
	}

	/** If you can't use a good index id. Increment by 1. */
	void inc() UNIV_NOTHROW { add(1); }

	/** If you can't use a good index id.
	@param n is the amount to increment */
	void add(Type n) UNIV_NOTHROW {
		size_t	i = m_policy.offset(m_policy.get_rnd_index());

		ut_ad(i < UT_ARR_SIZE(m_counter));

		add_noreturn(m_counter[i], n);
	}

	/** Use this if you can use a unique identifier, saves a
	call to get_rnd_index().
	@param	index	index into a slot
	@param	n	amount to increment */
	void add(size_t index, Type n) UNIV_NOTHROW {
		size_t	i = m_policy.offset(index);

		ut_ad(i < UT_ARR_SIZE(m_counter));

		add_noreturn(m_counter[i], n);
	}

	/** If you can't use a good index id. Decrement by 1. */
	void dec() UNIV_NOTHROW { sub(1); }

	/** If you can't use a good index id.
	@param n the amount to decrement */
	void sub(Type n) UNIV_NOTHROW {
		size_t	i = m_policy.offset(m_policy.get_rnd_index());

		ut_ad(i < UT_ARR_SIZE(m_counter));

		sub_noreturn(m_counter[i], n);
	}

	/** Use this if you can use a unique identifier, saves a
	call to get_rnd_index().
	@param	index	index into a slot
	@param	n	amount to decrement */
	void sub(size_t index, Type n) UNIV_NOTHROW {
		size_t	i = m_policy.offset(index);

		ut_ad(i < UT_ARR_SIZE(m_counter));

		sub_noreturn(m_counter[i], n);
	}

	/* @return total value - not 100% accurate, since it is not atomic. */
	operator Type() const UNIV_NOTHROW {
		Type	total = 0;

		for (size_t i = 0; i < N; ++i) {
			total += m_counter[m_policy.offset(i)];
		}

		return(total);
	}

private:
	/** Indexer into the array */
	Indexer<Type, N>m_policy;

        /** Slot 0 is unused. */
	Type		m_counter[(N + 1) *
                                (INNOBASE_CACHE_LINE_SIZE / sizeof(Type))];
};

#endif /* ut0counter_h */
