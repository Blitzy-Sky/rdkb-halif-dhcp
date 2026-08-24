/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file dhcpv4c_api.h
 * @brief RDK-B DHCPv4 client status HAL, expressed in the RDK compatibility types.
 *
 * This header declares 24 read-only accessors that report the state of the
 * DHCPv4 clients running on the E-Router and the ECM, plus 3 more for the eMTA
 * when `NO_MTA_FEATURE_SUPPORT` is not defined. Each accessor reports one value
 * a DHCPv4 client has already learned: a lease timer, a configuration attempt
 * count, an interface name, a client state, an address or a DNS server list.
 *
 * This repository ships two DHCPv4 client families that cover the same
 * E-Router, ECM and eMTA surface and differ only in the types they are
 * expressed in:
 *
 * - this header declares the `dhcpv4c_*` family in the RDK compatibility
 *   macros `INT`, `UINT` and `CHAR`, and reports address lists in
 *   `dhcpv4c_ip_list_t`;
 * - `dhcp4cApi.h` declares the equivalent `dhcp4c_*` family in plain C types
 *   (`int`, `unsigned int`, `char`), and reports address lists in
 *   `ipv4AddrList_t`.
 *
 * A caller therefore chooses between the two on type compatibility with its own
 * build, not on capability.
 *
 * The interface is read-only throughout. It declares no initialization,
 * teardown, start, stop or renew entry point, no setter, no callback
 * registration and no context handle, so there is nothing for a caller to open
 * or close before reading a value. Consistent with that, the Asynchronous
 * Notification Model topic of this repository's HAL specification states that
 * this interface raises no asynchronous notifications, and its Persistence
 * Model topic states that it is not required to persist any setting.
 *
 * @note The type names below are `#ifndef`-guarded fallbacks, not definitions
 *       this header insists on.
 * @warning Because those guards yield to whatever the including environment has
 *          already defined, a platform that defines `INT`, `UINT` or `CHAR`
 *          differently silently changes the signature of every declaration in
 *          this header. A caller that mixes this header with its own
 *          compatibility layer must confirm the two agree.
 *
 * @see dhcpv4c_ip_list_t
 */

#ifndef __DHCPV4_CLIENT_API_H__
#define __DHCPV4_CLIENT_API_H__


#ifndef ULONG
#define ULONG unsigned long   /*!< Fallback alias for `unsigned long`. No declaration in this header or in `dhcp4cApi.h` uses it. */
#endif

#ifndef BOOL
#define BOOL  unsigned char   /*!< Fallback alias for the byte-wide boolean used elsewhere in RDK-B. No declaration in this header or in `dhcp4cApi.h` uses it. */
#endif

#ifndef CHAR
#define CHAR  char            /*!< Fallback alias for `char`. The two interface-name accessors take `CHAR *`, so their buffers are sized in bytes. */
#endif

#ifndef UCHAR
#define UCHAR unsigned char   /*!< Fallback alias for `unsigned char`. No declaration in this header or in `dhcp4cApi.h` uses it. */
#endif

#ifndef INT
#define INT   int             /*!< Fallback alias for `int`. Every accessor here returns `INT`, and the state and attempt-count accessors also write through an `INT *`. */
#endif

#ifndef UINT
#define UINT  unsigned int    /*!< Fallback alias for `unsigned int`. Carries lease timers in seconds and packed 32-bit IPv4 addresses. */
#endif

#ifndef TRUE
#define TRUE     1            /*!< Boolean true, for use with `BOOL`. No declaration in this header or in `dhcp4cApi.h` uses it. */
#endif

#ifndef FALSE
#define FALSE    0            /*!< Boolean false, for use with `BOOL`. No declaration in this header or in `dhcp4cApi.h` uses it. */
#endif

#ifndef ENABLE
#define ENABLE   1            /*!< Enabled state, for use with `BOOL`. No declaration in this header or in `dhcp4cApi.h` uses it. */
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS     0  /*!< Success: the requested value was retrieved and written to the caller's location. Returned by every accessor in this header, and cited by `dhcp4cApi.h` for its own. */
#endif

#ifndef STATUS_FAILURE
#define STATUS_FAILURE     -1 /*!< Failure: the requested value was not retrieved, and the caller's output location is left in an unspecified state. The only failure code this interface defines; it does not distinguish a rejected argument from a retrieval error. */
#endif

/**
 * @addtogroup DHCPV4C_HAL_TYPES
 * @{
 */
#define DHCPV4_MAX_IPV4_ADDRS   4  /*!< Capacity of `dhcpv4c_ip_list_t::addrs`. A DNS server list never reports more entries than this, so a caller needs no bounds check beyond the reported count. */

/**********************************************************************
                STRUCTURE DEFINITIONS
**********************************************************************/
/**
 * @brief A list of IPv4 addresses reported by this interface.
 *
 * The caller allocates the structure and the accessor fills it in; the accessor
 * does not retain the pointer, so the caller may allocate it on its own stack.
 * `number` bounds the meaningful part of `addrs`: a caller reads `addrs[0]`
 * through `addrs[number - 1]` and must not read further, even though the array
 * is always `DHCPV4_MAX_IPV4_ADDRS` entries wide.
 *
 * @see dhcpv4c_get_ert_dns_svrs
 * @see dhcpv4c_get_ecm_dns_svrs
 */
typedef struct {
    INT  number;  /*!< Count of entries the accessor filled in `addrs`, from 0 to `DHCPV4_MAX_IPV4_ADDRS`. Bounds how much of `addrs` the caller may read. */
    UINT addrs[DHCPV4_MAX_IPV4_ADDRS];  /*!< The addresses, each one packed into a single 32-bit `UINT` rather than held as a dotted-quad string. This interface does not specify their byte order. */
} dhcpv4c_ip_list_t;
/** @} */  //END OF GROUP DHCPV4C_HAL_TYPES

/**
 * @addtogroup DHCPV4C_HAL_APIS
 * @{
 *
 * Every accessor in this group follows one contract, stated here once and
 * summarised on each declaration.
 *
 * **Argument convention.** Each accessor takes exactly one caller-supplied
 * output location and writes the requested value through it. The caller owns
 * that storage in every case, whether it is a scalar, a `CHAR` buffer or a
 * `dhcpv4c_ip_list_t`; no accessor allocates memory on the caller's behalf and
 * none retains the pointer once it has returned, so a caller may pass the
 * address of a local variable.
 *
 * **Return convention.** Each accessor returns `STATUS_SUCCESS` or
 * `STATUS_FAILURE` and nothing else. This interface deliberately defines no
 * granular error enumeration, so a single failure code covers a rejected
 * argument, a value the DHCPv4 client has not yet learned, and an internal
 * retrieval error alike; a caller cannot tell them apart from the return value
 * and must distinguish them by inspecting its own argument and by retrying.
 * The Internal Error Handling topic of this repository's HAL specification
 * requires every failure, internal errors such as an out-of-memory condition
 * included, to be reported synchronously through this return value.
 *
 * **Blocking.** The Blocking calls topic of that specification requires these
 * accessors to operate synchronously and not to block or suspend the caller's
 * main thread, while its Description topic states that the interface is
 * expected to block while the underlying hardware is not yet ready. Both hold,
 * so a caller on a latency-sensitive path must treat any of these calls as
 * potentially slow around start-up and prompt thereafter.
 *
 * **Thread safety.** The Threading Model topic states that this interface is
 * not required to be thread safe and that the calling module must ensure its
 * invocations are serialised. The Process Model topic adds that these
 * accessors are expected to be called from several processes, which the vendor
 * implementation must protect against; that obligation is the implementer's and
 * does not relieve a caller of serialising its own threads.
 *
 * **Not present.** This group declares no lifecycle, no setter, no callback
 * registration and no DHCPv6 or IPv6 accessor. A caller needing any of those
 * is looking at the wrong interface rather than at a gap to be worked around.
 */

/**********************************************************************************
 *
 *  DHCPV4-Client Subsystem level function prototypes
 *
**********************************************************************************/

/**
 * @brief Reports the lease time the E-Router's DHCPv4 server offered.
 *
 * The offered lease time is the whole duration the server granted, fixed when
 * the lease was taken; it does not count down. For the part still to run, use
 * dhcpv4c_get_ert_remain_lease_time().
 *
 * @param[out] pValue - Caller-owned location, set to the offered lease time in
 *                      seconds. `dhcp4cApi.h` bounds the equivalent value at
 *                      604800, seven days, and the same bound applies here.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the offered lease time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The lease time was retrieved and written to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client has not
 *                          been offered a lease yet, or the vendor
 *                          implementation could not read it. A caller re-checks
 *                          the pointer it passed, and otherwise re-reads later
 *                          rather than substituting zero for an unknown lease.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_remain_lease_time
 */
INT dhcpv4c_get_ert_lease_time(UINT *pValue);

/**
 * @brief Reports how much of the E-Router's current DHCP lease is left to run.
 *
 * This value decreases as the lease ages, so two calls a few seconds apart
 * legitimately return different numbers. It reaches zero when the lease expires.
 *
 * @param[out] pValue - Caller-owned location, set to the remaining lease time in
 *                      seconds, from 0 up to the offered lease time reported by
 *                      dhcpv4c_get_ert_lease_time().
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining lease time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining lease time was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client holds no
 *                          lease, or the vendor implementation could not read
 *                          it. A caller re-checks the pointer it passed, and
 *                          otherwise re-reads later; a failure does not mean the
 *                          lease has expired.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_lease_time
 */
INT dhcpv4c_get_ert_remain_lease_time(UINT *pValue);

/**
 * @brief Reports how long until the E-Router's DHCPv4 client begins renewing.
 *
 * Renewal is the client's first attempt to extend the lease with the server that
 * granted it. This value counts down to that moment and is therefore no greater
 * than the remaining lease time.
 *
 * @param[out] pValue - Caller-owned location, set to the time remaining before
 *                      renewal starts, in seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining renewal time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining renewal time was retrieved and written
 *                          to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client holds no
 *                          lease to renew, or the vendor implementation could
 *                          not read it. A caller re-checks the pointer it
 *                          passed, and otherwise re-reads later.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_remain_rebind_time
 */
INT dhcpv4c_get_ert_remain_renew_time(UINT *pValue);

/**
 * @brief Reports how long until the E-Router's DHCPv4 client begins rebinding.
 *
 * Rebinding is what the client falls back to when renewal with the original
 * server has not succeeded: it solicits any reachable server instead. This value
 * counts down to that moment and is therefore no less than the remaining
 * renewal time.
 *
 * @param[out] pValue - Caller-owned location, set to the time remaining before
 *                      rebinding starts, in seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining rebind time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining rebind time was retrieved and written
 *                          to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client holds no
 *                          lease to rebind, or the vendor implementation could
 *                          not read it. A caller re-checks the pointer it
 *                          passed, and otherwise re-reads later.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_remain_renew_time
 */
INT dhcpv4c_get_ert_remain_rebind_time(UINT *pValue);

/**
 * @brief Reports how many times the E-Router's DHCPv4 client has tried to
 *        configure the interface.
 *
 * The count is a diagnostic: a value that keeps climbing while
 * dhcpv4c_get_ert_fsm_state() stays away from the bound state indicates the
 * client is not reaching a server. This interface does not specify when, or
 * whether, the vendor implementation resets the count.
 *
 * @param[out] pValue - Caller-owned location, set to the number of configuration
 *                      attempts. The count is not negative; no upper bound is
 *                      specified by this interface.
 * @pre `pValue` addresses writable storage for one `INT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the attempt count; on failure its content is
 *       not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The attempt count was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, or the vendor implementation
 *                          does not maintain the E-Router client's count or
 *                          could not read it. A caller re-checks the pointer it
 *                          passed; a persistent failure here means the
 *                          diagnostic is unavailable on this platform, not that
 *                          no attempt was made.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_fsm_state
 */
INT dhcpv4c_get_ert_config_attempts(INT *pValue);

/**
 * @brief Writes the name of the E-Router network interface into a caller-supplied
 *        buffer.
 *
 * The name is vendor-specific and platform-specific: it is whatever the
 * underlying system calls the E-Router interface, so a caller passes it on to
 * the system rather than matching it against a fixed string.
 *
 * @param[out] pName - Caller-owned, caller-allocated buffer of **at least 64
 *                     bytes**, which the accessor fills with the interface name
 *                     as a zero-terminated string, as the Memory Model topic of
 *                     this repository's HAL specification requires of every
 *                     string it produces. The accessor writes no more than the
 *                     buffer holds and does not retain the pointer once it has
 *                     returned, so the buffer may be a local array; the caller
 *                     frees or discards it on its own terms.
 * @pre `pName` addresses at least 64 writable bytes; this interface has no
 *      initialization call to make first. A shorter buffer, or one whose size
 *      the accessor cannot know, is the caller's error to avoid: nothing in this
 *      interface can detect it.
 * @post On success `pName` holds a zero-terminated interface name; on failure its
 *       content is not specified by this interface, so a caller must not treat a
 *       failed call's buffer as a valid string.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The interface name was written to `pName`.
 * @retval STATUS_FAILURE - `pName` was rejected, or the vendor implementation
 *                          could not determine the E-Router interface's name. A
 *                          caller re-checks the pointer and the buffer size it
 *                          passed, and otherwise treats the name as unavailable
 *                          rather than reading the buffer.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @warning The buffer size is a contract this interface cannot enforce, because
 *          the accessor receives a bare `CHAR *` and no length. A buffer smaller
 *          than 64 bytes risks an overrun.
 */
INT dhcpv4c_get_ert_ifname(CHAR *pName);

/**
 * @brief Reports the state of the E-Router's DHCPv4 client state machine.
 *
 * The state says where the client is in acquiring or holding a lease, which is
 * what distinguishes "no address yet" from "address held" when another accessor
 * in this group fails.
 *
 * @param[out] pValue - Caller-owned location, set to the client's state as a
 *                      plain `INT`. This interface defines no enumeration for
 *                      these values, so a caller compares the integer against
 *                      the list below.
 *
 * @note Possible values for `pValue`:
 *  - 1: RELEASED or INIT_REBOOT
 *  - 2: INIT_SELECTING
 *  - 3: REQUESTING
 *  - 4: REBINDING
 *  - 5: BOUND
 *  - 6: RENEWING or RENEW_REQUESTED
 *
 * @pre `pValue` addresses writable storage for one `INT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds one of the values listed above; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The client state was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, or the vendor implementation
 *                          could not read the E-Router client's state. A caller
 *                          re-checks the pointer it passed, and otherwise
 *                          re-reads later; it must not read a failure as any
 *                          particular state.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @note This interface specifies the values above but not the transitions
 *       between them, so a caller must not infer a legal ordering from this
 *       list.
 */
INT dhcpv4c_get_ert_fsm_state(INT *pValue);

/**
 * @brief Reports the IPv4 address currently assigned to the E-Router interface.
 *
 * The address is the one the DHCPv4 client holds under its current lease, so it
 * is meaningful only while a lease is held.
 *
 * @param[out] pValue - Caller-owned location, set to the IPv4 address packed
 *                      into a single 32-bit `UINT`. This interface does not
 *                      specify the byte order of the returned address, so a
 *                      caller must confirm it with the vendor implementation
 *                      rather than assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the assigned address; on failure its content
 *       is not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The address was retrieved and written to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client holds no
 *                          lease and therefore no address, or the vendor
 *                          implementation could not read it. A caller re-checks
 *                          the pointer it passed, and otherwise treats the
 *                          address as not yet assigned rather than reading
 *                          `*pValue` as 0.0.0.0.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_mask
 * @see dhcpv4c_get_ert_gw
 */
INT dhcpv4c_get_ert_ip_addr(UINT *pValue);

/**
 * @brief Reports the subnet mask of the E-Router interface.
 *
 * The mask accompanies the address reported by dhcpv4c_get_ert_ip_addr() and
 * together with it defines the directly reachable subnet.
 *
 * @param[out] pValue - Caller-owned location, set to the subnet mask packed into
 *                      a single 32-bit `UINT` in the same form as an address,
 *                      not as a prefix length. This interface does not specify
 *                      the byte order of the returned value, so a caller must
 *                      confirm it with the vendor implementation rather than
 *                      assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the subnet mask; on failure its content is
 *       not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The subnet mask was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client holds no
 *                          lease and therefore no mask, or the vendor
 *                          implementation could not read it. A caller re-checks
 *                          the pointer it passed, and otherwise treats the mask
 *                          as unavailable; it must not fall back to a classful
 *                          default.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_ip_addr
 */
INT dhcpv4c_get_ert_mask(UINT *pValue);

/**
 * @brief Reports the IPv4 address of the E-Router's default gateway.
 *
 * This is the router the DHCPv4 server offered with the lease, to which the
 * E-Router sends traffic outside its own subnet.
 *
 * @param[out] pValue - Caller-owned location, set to the gateway address packed
 *                      into a single 32-bit `UINT`. This interface does not
 *                      specify the byte order of the returned address, so a
 *                      caller must confirm it with the vendor implementation
 *                      rather than assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the gateway address; on failure its content
 *       is not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The gateway address was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the lease carried no router
 *                          option, or the vendor implementation could not read
 *                          it. A caller re-checks the pointer it passed, and
 *                          otherwise treats the gateway as unknown rather than
 *                          assuming one exists on the subnet.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_ip_addr
 */
INT dhcpv4c_get_ert_gw(UINT *pValue);

/**
 * @brief Reports the DNS servers offered to the E-Router with its current lease.
 *
 * The list is what the DHCPv4 server supplied, in the order it supplied them, so
 * a resolver should try them in that order.
 *
 * @param[out] pList - Caller-owned, caller-allocated `dhcpv4c_ip_list_t`. The
 *                     accessor sets `pList->number` to the count of servers it
 *                     wrote and fills that many entries of `pList->addrs`, never
 *                     more than `DHCPV4_MAX_IPV4_ADDRS`; the caller must read no
 *                     further than `pList->number` entries. The accessor does
 *                     not retain the pointer once it has returned, so the
 *                     structure may be a local variable.
 * @pre `pList` addresses writable storage for one `dhcpv4c_ip_list_t`; this
 *      interface has no initialization call to make first. The caller need not
 *      pre-initialize the structure.
 * @post On success `pList->number` and the first `pList->number` entries of
 *       `pList->addrs` are set; on failure the structure's content is not
 *       specified by this interface, so a caller must not read `number` after a
 *       failed call.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The DNS server list was retrieved and written to
 *                          `*pList`. A count of 0 is a valid success: the lease
 *                          offered no DNS server.
 * @retval STATUS_FAILURE - `pList` was rejected, the E-Router client holds no
 *                          lease, or the vendor implementation could not read
 *                          the list. A caller re-checks the pointer it passed,
 *                          and otherwise re-reads later rather than treating the
 *                          list as empty.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_ip_list_t
 */
INT dhcpv4c_get_ert_dns_svrs(dhcpv4c_ip_list_t *pList);

/**
 * @brief Reports the IPv4 address of the DHCPv4 server serving the E-Router.
 *
 * This is the server that granted the current lease and the one the client will
 * approach first when it renews.
 *
 * @param[out] pValue - Caller-owned location, set to the server's address packed
 *                      into a single 32-bit `UINT`. This interface does not
 *                      specify the byte order of the returned address, so a
 *                      caller must confirm it with the vendor implementation
 *                      rather than assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the server address; on failure its content is
 *       not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The server address was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the E-Router client has not
 *                          yet reached a server, or the vendor implementation
 *                          could not read the address. A caller re-checks the
 *                          pointer it passed, and otherwise reads
 *                          dhcpv4c_get_ert_fsm_state() to tell "no server yet"
 *                          from a retrieval fault.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ert_fsm_state
 */
INT dhcpv4c_get_ert_dhcp_svr(UINT *pValue);

/**
 * @brief Reports the lease time the ECM's DHCPv4 server offered.
 *
 * The Embedded Cable Modem takes its own lease, separately from the E-Router, so
 * this value is unrelated to the one dhcpv4c_get_ert_lease_time() reports. It is
 * the whole duration the server granted, fixed when the lease was taken, and
 * does not count down.
 *
 * @param[out] pValue - Caller-owned location, set to the offered lease time in
 *                      seconds. `dhcp4cApi.h` bounds the equivalent value at
 *                      604800, seven days, and the same bound applies here.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the offered lease time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The lease time was retrieved and written to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client has not been
 *                          offered a lease yet, or the vendor implementation
 *                          could not read it. A caller re-checks the pointer it
 *                          passed, and otherwise re-reads later rather than
 *                          substituting zero for an unknown lease.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_remain_lease_time
 */
INT dhcpv4c_get_ecm_lease_time(UINT *pValue);

/**
 * @brief Reports how much of the ECM's current DHCP lease is left to run.
 *
 * This value decreases as the lease ages, so two calls a few seconds apart
 * legitimately return different numbers. It reaches zero when the lease expires.
 *
 * @param[out] pValue - Caller-owned location, set to the remaining lease time in
 *                      seconds, from 0 up to the offered lease time reported by
 *                      dhcpv4c_get_ecm_lease_time().
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining lease time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining lease time was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client holds no lease,
 *                          or the vendor implementation could not read it. A
 *                          caller re-checks the pointer it passed, and otherwise
 *                          re-reads later; a failure does not mean the lease has
 *                          expired.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_lease_time
 */
INT dhcpv4c_get_ecm_remain_lease_time(UINT *pValue);

/**
 * @brief Reports how long until the ECM's DHCPv4 client begins renewing.
 *
 * Renewal is the client's first attempt to extend the lease with the server that
 * granted it. This value counts down to that moment and is therefore no greater
 * than the remaining lease time.
 *
 * @param[out] pValue - Caller-owned location, set to the time remaining before
 *                      renewal starts, in seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining renewal time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining renewal time was retrieved and written
 *                          to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client holds no lease
 *                          to renew, or the vendor implementation could not read
 *                          it. A caller re-checks the pointer it passed, and
 *                          otherwise re-reads later.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_remain_rebind_time
 */
INT dhcpv4c_get_ecm_remain_renew_time(UINT *pValue);

/**
 * @brief Reports how long until the ECM's DHCPv4 client begins rebinding.
 *
 * Rebinding is what the client falls back to when renewal with the original
 * server has not succeeded: it solicits any reachable server instead. This value
 * counts down to that moment and is therefore no less than the remaining
 * renewal time.
 *
 * @param[out] pValue - Caller-owned location, set to the time remaining before
 *                      rebinding starts, in seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining rebind time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining rebind time was retrieved and written
 *                          to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client holds no lease
 *                          to rebind, or the vendor implementation could not
 *                          read it. A caller re-checks the pointer it passed,
 *                          and otherwise re-reads later.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_remain_renew_time
 */
INT dhcpv4c_get_ecm_remain_rebind_time(UINT *pValue);

/**
 * @brief Reports how many times the ECM's DHCPv4 client has tried to configure
 *        the interface.
 *
 * The count is a diagnostic: a value that keeps climbing while
 * dhcpv4c_get_ecm_fsm_state() stays away from the bound state indicates the
 * client is not reaching a server. This interface does not specify when, or
 * whether, the vendor implementation resets the count.
 *
 * @param[out] pValue - Caller-owned location, set to the number of configuration
 *                      attempts. The count is not negative; no upper bound is
 *                      specified by this interface.
 * @pre `pValue` addresses writable storage for one `INT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the attempt count; on failure its content is
 *       not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The attempt count was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, or the vendor implementation
 *                          does not maintain the ECM client's count or could
 *                          not read it. A caller re-checks the pointer it
 *                          passed; a persistent failure here means the
 *                          diagnostic is unavailable on this platform, not that
 *                          no attempt was made.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_fsm_state
 */
INT dhcpv4c_get_ecm_config_attempts(INT *pValue);

/**
 * @brief Writes the name of the ECM network interface into a caller-supplied
 *        buffer.
 *
 * The name is vendor-specific and platform-specific: it is whatever the
 * underlying system calls the Embedded Cable Modem interface, so a caller passes
 * it on to the system rather than matching it against a fixed string.
 *
 * @param[out] pName - Caller-owned, caller-allocated buffer of **at least 64
 *                     bytes**, which the accessor fills with the interface name
 *                     as a zero-terminated string, as the Memory Model topic of
 *                     this repository's HAL specification requires of every
 *                     string it produces. The accessor writes no more than the
 *                     buffer holds and does not retain the pointer once it has
 *                     returned, so the buffer may be a local array; the caller
 *                     frees or discards it on its own terms.
 * @pre `pName` addresses at least 64 writable bytes; this interface has no
 *      initialization call to make first. A shorter buffer, or one whose size
 *      the accessor cannot know, is the caller's error to avoid: nothing in this
 *      interface can detect it.
 * @post On success `pName` holds a zero-terminated interface name; on failure its
 *       content is not specified by this interface, so a caller must not treat a
 *       failed call's buffer as a valid string.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The interface name was written to `pName`.
 * @retval STATUS_FAILURE - `pName` was rejected, or the vendor implementation
 *                          could not determine the ECM interface's name. A
 *                          caller re-checks the pointer and the buffer size it
 *                          passed, and otherwise treats the name as unavailable
 *                          rather than reading the buffer.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @warning The buffer size is a contract this interface cannot enforce, because
 *          the accessor receives a bare `CHAR *` and no length. A buffer smaller
 *          than 64 bytes risks an overrun.
 */
INT dhcpv4c_get_ecm_ifname(CHAR *pName);

/**
 * @brief Reports the state of the ECM's DHCPv4 client state machine.
 *
 * The state says where the Embedded Cable Modem's client is in acquiring or
 * holding a lease, which is what distinguishes "no address yet" from "address
 * held" when another accessor in this group fails.
 *
 * @param[out] pValue - Caller-owned location, set to the client's state as a
 *                      plain `INT`. This interface defines no enumeration for
 *                      these values, so a caller compares the integer against
 *                      the list below.
 *
 * @note Possible values for `pValue`:
 *  - 1: RELEASED or INIT_REBOOT
 *  - 2: INIT_SELECTING
 *  - 3: REQUESTING
 *  - 4: REBINDING
 *  - 5: BOUND
 *  - 6: RENEWING or RENEW_REQUESTED
 *
 * @pre `pValue` addresses writable storage for one `INT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds one of the values listed above; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The client state was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, or the vendor implementation
 *                          could not read the ECM client's state. A caller
 *                          re-checks the pointer it passed, and otherwise
 *                          re-reads later; it must not read a failure as any
 *                          particular state.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @note This interface specifies the values above but not the transitions
 *       between them, so a caller must not infer a legal ordering from this
 *       list.
 */
INT dhcpv4c_get_ecm_fsm_state(INT *pValue);

/**
 * @brief Reports the IPv4 address currently assigned to the ECM interface.
 *
 * The address is the one the Embedded Cable Modem's DHCPv4 client holds under its
 * current lease, so it is meaningful only while a lease is held.
 *
 * @param[out] pValue - Caller-owned location, set to the IPv4 address packed
 *                      into a single 32-bit `UINT`. This interface does not
 *                      specify the byte order of the returned address, so a
 *                      caller must confirm it with the vendor implementation
 *                      rather than assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the assigned address; on failure its content
 *       is not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The address was retrieved and written to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client holds no lease
 *                          and therefore no address, or the vendor
 *                          implementation could not read it. A caller re-checks
 *                          the pointer it passed, and otherwise treats the
 *                          address as not yet assigned rather than reading
 *                          `*pValue` as 0.0.0.0.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_mask
 * @see dhcpv4c_get_ecm_gw
 */
INT dhcpv4c_get_ecm_ip_addr(UINT *pValue);

/**
 * @brief Reports the subnet mask of the ECM interface.
 *
 * The mask accompanies the address reported by dhcpv4c_get_ecm_ip_addr() and
 * together with it defines the directly reachable subnet. For the gateway
 * outside that subnet, use dhcpv4c_get_ecm_gw().
 *
 * @param[out] pValue - Caller-owned location, set to the subnet mask packed into
 *                      a single 32-bit `UINT` in the same form as an address,
 *                      not as a prefix length. This interface does not specify
 *                      the byte order of the returned value, so a caller must
 *                      confirm it with the vendor implementation rather than
 *                      assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the subnet mask; on failure its content is
 *       not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The subnet mask was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client holds no lease
 *                          and therefore no mask, or the vendor implementation
 *                          could not read it. A caller re-checks the pointer it
 *                          passed, and otherwise treats the mask as unavailable;
 *                          it must not fall back to a classful default.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_ip_addr
 */
INT dhcpv4c_get_ecm_mask(UINT *pValue);

/**
 * @brief Reports the IPv4 address of the ECM's default gateway.
 *
 * This is the router the DHCPv4 server offered with the ECM's lease, to which
 * the Embedded Cable Modem sends traffic outside its own subnet. It is a
 * gateway address and not a subnet mask; for the mask, use
 * dhcpv4c_get_ecm_mask().
 *
 * @param[out] pValue - Caller-owned location, set to the gateway address packed
 *                      into a single 32-bit `UINT`. This interface does not
 *                      specify the byte order of the returned address, so a
 *                      caller must confirm it with the vendor implementation
 *                      rather than assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the gateway address; on failure its content
 *       is not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The gateway address was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM's lease carried no
 *                          router option, or the vendor implementation could not
 *                          read it. A caller re-checks the pointer it passed,
 *                          and otherwise treats the gateway as unknown rather
 *                          than assuming one exists on the subnet.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_ip_addr
 * @see dhcpv4c_get_ecm_mask
 */
INT dhcpv4c_get_ecm_gw(UINT *pValue);

/**
 * @brief Reports the DNS servers offered to the ECM with its current lease.
 *
 * The list is what the DHCPv4 server supplied, in the order it supplied them, so
 * a resolver should try them in that order.
 *
 * @param[out] pList - Caller-owned, caller-allocated `dhcpv4c_ip_list_t`. The
 *                     accessor sets `pList->number` to the count of servers it
 *                     wrote and fills that many entries of `pList->addrs`, never
 *                     more than `DHCPV4_MAX_IPV4_ADDRS`; the caller must read no
 *                     further than `pList->number` entries. The accessor does
 *                     not retain the pointer once it has returned, so the
 *                     structure may be a local variable.
 * @pre `pList` addresses writable storage for one `dhcpv4c_ip_list_t`; this
 *      interface has no initialization call to make first. The caller need not
 *      pre-initialize the structure.
 * @post On success `pList->number` and the first `pList->number` entries of
 *       `pList->addrs` are set; on failure the structure's content is not
 *       specified by this interface, so a caller must not read `number` after a
 *       failed call.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The DNS server list was retrieved and written to
 *                          `*pList`. A count of 0 is a valid success: the lease
 *                          offered no DNS server.
 * @retval STATUS_FAILURE - `pList` was rejected, the ECM client holds no lease,
 *                          or the vendor implementation could not read the list.
 *                          A caller re-checks the pointer it passed, and
 *                          otherwise re-reads later rather than treating the
 *                          list as empty.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_ip_list_t
 */
INT dhcpv4c_get_ecm_dns_svrs(dhcpv4c_ip_list_t *pList);

/**
 * @brief Reports the IPv4 address of the DHCPv4 server serving the ECM.
 *
 * This is the server that granted the ECM's current lease and the one its client
 * will approach first when it renews.
 *
 * @param[out] pValue - Caller-owned location, set to the server's address packed
 *                      into a single 32-bit `UINT`. This interface does not
 *                      specify the byte order of the returned address, so a
 *                      caller must confirm it with the vendor implementation
 *                      rather than assuming host or network order.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the server address; on failure its content is
 *       not specified by this interface, so a caller must not assume it is
 *       unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The server address was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the ECM client has not yet
 *                          reached a server, or the vendor implementation could
 *                          not read the address. A caller re-checks the pointer
 *                          it passed, and otherwise reads
 *                          dhcpv4c_get_ecm_fsm_state() to tell "no server yet"
 *                          from a retrieval fault.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @see dhcpv4c_get_ecm_fsm_state
 */
INT dhcpv4c_get_ecm_dhcp_svr(UINT *pValue);

#if !defined (NO_MTA_FEATURE_SUPPORT)
/**
 * @brief Reports how much of the eMTA's current DHCP lease is left to run.
 *
 * The Embedded Multimedia Terminal Adapter takes its own lease, separately from
 * the E-Router and the ECM. This value decreases as that lease ages and reaches
 * zero when it expires.
 *
 * Unlike the E-Router and ECM groups, the eMTA group reports only the three
 * lease timers: this interface declares no eMTA accessor for an offered lease
 * time, a configuration attempt count, an interface name, a client state, an
 * address or a DNS server list.
 *
 * @param[out] pValue - Caller-owned location, set to the remaining lease time in
 *                      seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining lease time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining lease time was retrieved and written to
 *                          `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the eMTA client holds no lease,
 *                          or the vendor implementation could not read it. A
 *                          caller re-checks the pointer it passed, and otherwise
 *                          re-reads later; a failure does not mean the lease has
 *                          expired.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @note Declared only when `NO_MTA_FEATURE_SUPPORT` is not defined. On a build
 *       that defines it, this accessor does not exist and a reference to it
 *       fails to compile, so a caller that supports both builds must guard its
 *       own use of it with the same condition.
 * @see dhcpv4c_get_emta_remain_renew_time
 */
INT dhcpv4c_get_emta_remain_lease_time(UINT *pValue);

/**
 * @brief Reports how long until the eMTA's DHCPv4 client begins renewing.
 *
 * Renewal is the client's first attempt to extend the lease with the server that
 * granted it. This value counts down to that moment and is therefore no greater
 * than the eMTA's remaining lease time.
 *
 * @param[out] pValue - Caller-owned location, set to the time remaining before
 *                      renewal starts, in seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining renewal time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining renewal time was retrieved and written
 *                          to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the eMTA client holds no lease
 *                          to renew, or the vendor implementation could not read
 *                          it. A caller re-checks the pointer it passed, and
 *                          otherwise re-reads later.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @note Declared only when `NO_MTA_FEATURE_SUPPORT` is not defined. On a build
 *       that defines it, this accessor does not exist and a reference to it
 *       fails to compile, so a caller that supports both builds must guard its
 *       own use of it with the same condition.
 * @see dhcpv4c_get_emta_remain_rebind_time
 */
INT dhcpv4c_get_emta_remain_renew_time(UINT *pValue);

/**
 * @brief Reports how long until the eMTA's DHCPv4 client begins rebinding.
 *
 * Rebinding is what the client falls back to when renewal with the original
 * server has not succeeded: it solicits any reachable server instead. This value
 * counts down to that moment and is therefore no less than the eMTA's remaining
 * renewal time.
 *
 * @param[out] pValue - Caller-owned location, set to the time remaining before
 *                      rebinding starts, in seconds.
 * @pre `pValue` addresses writable storage for one `UINT`; this interface has no
 *      initialization call to make first.
 * @post On success `*pValue` holds the remaining rebind time; on failure its
 *       content is not specified by this interface, so a caller must not assume
 *       it is unmodified.
 * @returns Status of the operation.
 * @retval STATUS_SUCCESS - The remaining rebind time was retrieved and written
 *                          to `*pValue`.
 * @retval STATUS_FAILURE - `pValue` was rejected, the eMTA client holds no lease
 *                          to rebind, or the vendor implementation could not
 *                          read it. A caller re-checks the pointer it passed,
 *                          and otherwise re-reads later.
 * @note Synchronous and must not block the caller's main thread, but expected to
 *       block while the underlying hardware is not yet ready.
 * @note Not required to be thread safe; the calling module must serialise its
 *       invocations.
 * @note Declared only when `NO_MTA_FEATURE_SUPPORT` is not defined. On a build
 *       that defines it, this accessor does not exist and a reference to it
 *       fails to compile, so a caller that supports both builds must guard its
 *       own use of it with the same condition.
 * @see dhcpv4c_get_emta_remain_renew_time
 */
INT dhcpv4c_get_emta_remain_rebind_time(UINT *pValue);
#endif
/** @} */  //END OF GROUP DHCPV4C_HAL_APIS
#endif
