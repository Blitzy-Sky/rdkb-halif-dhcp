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
 * @file dhcp4cApi.h
 * @brief Defines the public interface for the RDK-Broadband DHCPv4 Client HAL.
 *
 * This header includes function prototypes, data structures, and constants used to manage
 * DHCPv4 client operations on cable modems.
 */
#ifndef _DHCP4_CLIENT_API_
#define _DHCP4_CLIENT_API_

/**
 * @defgroup DHCPV4C_HAL DHCPV4C HAL
 *
 * DHCPv4C HAL is used for the RDK-B DHCPv4 Client Status abstraction layer.
 *
 * @defgroup DHCPV4C_HAL_TYPES  DHCPV4C HAL Data Types
 * @ingroup  DHCPV4C_HAL
 *
 * @defgroup DHCPV4C_HAL_APIS   DHCPV4C HAL APIs
 * @ingroup  DHCPV4C_HAL
 *
 **/

/**
 * @addtogroup DHCPV4C_HAL_TYPES
 * @{
 */

/**********************************************************************
                ENUMERATION DEFINITIONS
**********************************************************************/
/**
 * @brief Represents the information for the DHCP client.
 *
 * This enumeration contains the different information used in DHCP client such as lease time, 
 * configuration attempts, interface name, FSM state and server information.
 */
/** Represents the information for the DHCPv4 client. */
enum DHCPC_CMD {
    DHCPC_CMD_LEASE_TIME,             /*!< Lease time obtained from the DHCP server. */
    DHCPC_CMD_LEASE_TIME_REMAIN,      /*!< Remaining lease time. */
    DHCPC_CMD_RENEW_TIME_REMAIN,      /*!< Remaining time for lease renewal. */
    DHCPC_CMD_REBIND_TIME_REMAIN,     /*!< Remaining time for rebinding. */
    DHCPC_CMD_CONFIG_ATTEMPTS,        /*!< Number of configuration attempts. */
    DHCPC_CMD_GET_IFNAME,             /*!< Interface name associated with the DHCP client. */
    DHCPC_CMD_FSM_STATE,              /*!< DHCP client's FSM state. */
    DHCPC_CMD_IP_ADDR,                /*!< IP address obtained from the server. */
    DHCPC_CMD_IP_MASK,                /*!< Subnet mask obtained from the server. */
    DHCPC_CMD_ROUTERS,                /*!< List of routers obtained from the server. */
    DHCPC_CMD_DNS_SVRS,               /*!< List of DNS servers obtained from the server. */
    DHCPC_CMD_DHCP_SVR,               /*!< IP addresses of the DHCP servers. */
    DHCPC_CMD_MAX                     /*!< Maximum value of the elements (not used). */
};


/**
 * @brief Represents the different types of modules in the DHCP client.
 *
 * This enumeration contains modules which defines embedded cable modem, router related functionality,
 * embedded multimedia terminal adapter. 
 */
/*!< Represents the different modules within the DHCPv4 client. */
enum DHCPC_MODULE{
    DHCPC_ECM,      /*!< Embedded Cable Modem (ECM) module. */
#if !defined (NO_MTA_FEATURE_SUPPORT)
    DHCPC_EROUTER,  /*!< Router module. */
    DHCPC_EMTA      /*!< Embedded Multimedia Terminal Adapter (eMTA) module. */
#else
    DHCPC_EROUTER   /*!< Router module. Keeps ordinal 1 in this arm too, because it is written once in each arm rather than moved between them. */
#endif
};

#define MAX_IPV4_ADDR_LIST_NUMBER        4  //!< Maximum number of IPv4 addresses in the list  

/**********************************************************************
                STRUCTURE DEFINITIONS
**********************************************************************/
/**
 * @brief A list of IPv4 addresses reported by this interface.
 *
 * The caller allocates the structure and the accessor fills it in; `number`
 * bounds the meaningful part of `addrList`, whose entries are integers rather
 * than text, on the terms the member comment states.
 * @see dhcp4c_get_ert_dns_svrs
 */
typedef struct {
    int number; /*!< Count of entries the accessor filled in `addrList`, from 0 to `MAX_IPV4_ADDR_LIST_NUMBER`. Bounds how much of `addrList` the caller may read. */
    unsigned int addrList[MAX_IPV4_ADDR_LIST_NUMBER];  /*!< The addresses, `MAX_IPV4_ADDR_LIST_NUMBER` entries of `unsigned int`, each holding one address packed into a single 32-bit word rather than held as a dotted-quad string: a rendering such as `192.168.0.1` is what a caller formats from such a word for a human reader, never what is stored here. This interface does not specify the byte order of a word, so a caller confirms it with the vendor implementation rather than assuming host or network order. */

} ipv4AddrList_t;

/** @} */  //END OF GROUP DHCPV4C_HAL_TYPES

/**
 * @addtogroup DHCPV4C_HAL_APIS
 * @{
 *
 * The accessors below share one contract. It is stated here once and summarised
 * on each declaration, so a caller reading a single block still sees the whole
 * agreement.
 *
 * **What this family is.** These 27 accessors report values that DHCPv4 clients
 * running for the E-Router, the ECM and, on builds that keep the eMTA, the eMTA
 * have already learned: lease timers, a configuration attempt count, an
 * interface name, a client state, addresses and a DNS server list. They are
 * expressed in plain C types - `int`, `unsigned int`, `char` - and they report
 * address lists in this header's own `ipv4AddrList_t`. The `dhcpv4c_*` family in
 * `dhcpv4c_api.h` covers the same values under the RDK compatibility macros
 * `INT`, `UINT` and `CHAR` and reports lists in `dhcpv4c_ip_list_t`. The two
 * families are matched name for name, so the choice between them is a matter of
 * which types agree with the caller's own build; neither offers a value the
 * other lacks. Because neither header includes the other, a caller cannot hand
 * an `ipv4AddrList_t` to a `dhcpv4c_*` accessor.
 *
 * **Types this header owns.** `DHCPC_CMD`, `DHCPC_MODULE`,
 * `MAX_IPV4_ADDR_LIST_NUMBER` and `ipv4AddrList_t` are declared in the
 * `DHCPV4C_HAL_TYPES` group above. Only the last two reach a signature: no
 * accessor takes a `DHCPC_CMD` or a `DHCPC_MODULE`, so those two enumerations
 * describe the value and module space this interface covers rather than
 * selecting anything at a call site. A caller picks a value by choosing a
 * function name.
 *
 * **Argument convention.** Each accessor takes exactly one output location and
 * writes one value through it. The caller allocates that location in every
 * case - a scalar, a `char` buffer or an `ipv4AddrList_t` - and keeps ownership
 * of it; no accessor allocates storage on the caller's behalf, and none hands
 * the caller anything to release. **Whether an accessor retains the pointer
 * beyond the call is not specified by this interface**, and no declaration here
 * withdraws a retained pointer, so a caller keeps the location allocated and
 * unmoved after the call returns rather than assuming a lifetime that ends with
 * it, and settles the point with its vendor implementation before it reuses or
 * releases the storage. That is the single position this repository takes on
 * retention: the Memory Model topic of this repository's HAL specification
 * records the same absence, and neither it nor these declarations turns
 * non-retention into a guarantee a caller may rely on. What the Memory Model
 * topic does carry as a repository-wide obligation is that any string this
 * interface produces be zero-terminated.
 *
 * **Return domain, and where its two names live.** Every accessor returns
 * `int`, and the interface uses exactly two values: success, `0`, and failure,
 * `-1`. The symbolic names for them, `STATUS_SUCCESS` and `STATUS_FAILURE`, are
 * defined in the sibling header `dhcpv4c_api.h` at lines 103-109 as
 * `#ifndef`-guarded fallbacks. **This header defines neither, and it includes
 * nothing**, so a translation unit that includes only `dhcp4cApi.h` does not see
 * those two names: it must either include `dhcpv4c_api.h` as well or obtain
 * equivalent definitions from its own compatibility layer, and where it does
 * the latter it must confirm they carry the same two values. Comparing a return
 * value against `0` works in every translation unit, whichever route it takes.
 *
 * **What a failure tells a caller, and what it does not.** This interface
 * defines no granular error enumeration, and it does not enumerate the
 * conditions that produce a failure either: the one failure value reports
 * every one of them, a caller cannot tell them apart from the return value
 * alone, and no error string, last-error call or diagnostic code is declared
 * anywhere here to make up the difference. What is open to a caller is to
 * re-check the pointer and, for the interface-name accessors, the buffer size
 * it passed; to read the matching state accessor, which reports the client's
 * own state independently of this call; and otherwise to treat the value as
 * unavailable for now and read it again later. Substituting a default - a zero
 * lease, a `0.0.0.0` address, an empty server list - for a value this
 * interface declined to report is the one response that turns a reported
 * failure into a wrong answer. The Internal Error Handling topic of this
 * repository's HAL specification requires every failure, an internal condition
 * such as exhausted memory included, to be reported synchronously through this
 * return value, so a caller never has to look elsewhere for an error it did
 * not see here.
 *
 * **Blocking.** **This interface does not establish whether an accessor may
 * block.** No declaration here states the duration of a call, a condition
 * under which one may wait, or what an accessor does when the network hardware
 * or the DHCPv4 client is not yet ready, and nothing else in this repository
 * settles the point, so there is no blocking behaviour for a caller to rely on
 * or for a test to assert. No bound is stated either: no accessor takes a
 * timeout argument, no numeric timeout, completion-time target or upper bound
 * is stated anywhere here, and there is no cancellation call, no abort and no
 * asynchronous variant, so a call already issued cannot be withdrawn. What a
 * call does deliver is its result, synchronously, through the return value at
 * the call site - which says where the result appears, not when. The
 * consequences are the caller's: place the call where a delay is survivable,
 * impose and enforce your own bound if the calling context needs one, and do
 * not let a test assert a completion time this interface does not state. The
 * Blocking calls topic of the HAL specification states this and nothing else.
 *
 * **Thread safety.** The Threading Model topic states that this interface is not
 * required to be thread safe and places serialisation on the calling module. The
 * Process Model topic adds that these accessors are expected to be called from
 * several processes and that the vendor implementation must protect against
 * that; that obligation belongs to the implementer and does not relieve a caller
 * of serialising its own threads.
 *
 * **Reading several values.** The accessors are independent and impose no
 * ordering: any of them may be the first call a caller makes, and none has to
 * precede another. The client keeps running between two calls, so a caller
 * assembling a view of DHCPv4 state must expect the parts to disagree - a state
 * read as bound alongside a remaining lease time that has since reached zero,
 * for instance. This interface offers no atomic multi-value read and no
 * snapshot.
 *
 * **What this group does not declare.** There is no initialization, teardown,
 * open, close, start, stop, renew or release entry point and no handle or
 * context type, which is why every block below states that there is nothing to
 * call first. There is no setter: the whole family is read-only. There is no
 * callback type and no registration function, so a caller that needs to know
 * when a lease changes polls the accessors it cares about - the Asynchronous
 * Notification Model topic of the specification states the same absence, and its
 * Persistence Model topic states that this interface is not required to persist
 * any setting. And there is no DHCPv4 server accessor: the superproject
 * inventory describes this repository as covering a DHCPv4 client and server and
 * attributes lease acquisition, renewal, release and event notification to this
 * header, but no such entry point is declared here or in `dhcpv4c_api.h`, so
 * that description is context to be read against these declarations rather than
 * a surface to be looked for.
 */

/**
 * @brief Retrieves the lease time the E-Router's DHCPv4 server granted.
 *
 * This is the whole duration of the current lease, fixed when the server granted
 * it. It does not count down; dhcp4c_get_ert_remain_lease_time() reports the part
 * still to run.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the granted
 *                      lease time in seconds, up to 604800 - seven days. The
 *                      accessor writes it. Retention beyond the call is not
 *                      specified by this interface, so a caller keeps the
 *                      location allocated and unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the granted lease time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The lease time was read and written through `pValue`.
 * @retval STATUS_FAILURE - The lease time was not retrieved, and this interface
 *                          does not enumerate the conditions that produce this
 *                          code. A caller re-checks the pointer it passed and
 *                          otherwise reads the value again later rather than
 *                          treating the lease as zero-length.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_remain_lease_time
 */
int dhcp4c_get_ert_lease_time(unsigned int *pValue);

/**
 * @brief Retrieves how much of the E-Router's current DHCP lease is left to run.
 *
 * The value falls as the lease ages, so two calls seconds apart legitimately
 * disagree, and it reaches zero when the lease expires. It never exceeds the
 * granted lease time reported by dhcp4c_get_ert_lease_time().
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the
 *                      remaining lease time in seconds. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining lease time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining lease time was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The remaining lease time was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later; a failure
 *                          here is not a report that the lease has expired,
 *                          which is a successful read of zero.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_lease_time
 * @see dhcp4c_get_ert_remain_renew_time
 */
int dhcp4c_get_ert_remain_lease_time(unsigned int *pValue);

/**
 * @brief Retrieves how long until the E-Router's DHCPv4 client starts renewing.
 *
 * Renewal is the client's first attempt to extend the lease, made to the server
 * that granted it. This value counts down to that moment, so it is never greater
 * than the remaining lease time and never greater than the remaining rebind
 * time.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the seconds
 *                      remaining before renewal begins. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining renewal time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining renewal time was read and written
 *                          through `pValue`.
 * @retval STATUS_FAILURE - The remaining renewal time was not retrieved, and
 *                          this interface does not enumerate the conditions
 *                          that produce this code. A caller re-checks the
 *                          pointer it passed and otherwise reads again later.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_remain_rebind_time
 */
int dhcp4c_get_ert_remain_renew_time(unsigned int *pValue);

/**
 * @brief Retrieves how long until the E-Router's DHCPv4 client starts rebinding.
 *
 * Rebinding is the fallback when renewal with the granting server has not
 * succeeded: the client solicits any reachable server instead. This value counts
 * down to that moment, so it sits between the remaining renewal time and the
 * remaining lease time.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the seconds
 *                      remaining before rebinding begins. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining rebind time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining rebind time was read and written
 *                          through `pValue`.
 * @retval STATUS_FAILURE - The remaining rebind time was not retrieved, and
 *                          this interface does not enumerate the conditions
 *                          that produce this code. A caller re-checks the
 *                          pointer it passed and otherwise reads again later.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_remain_renew_time
 */
int dhcp4c_get_ert_remain_rebind_time(unsigned int *pValue);

/**
 * @brief Retrieves how many times the E-Router's DHCPv4 client has tried to
 *        configure its interface.
 *
 * The count is a diagnostic rather than a lease value: a number that keeps
 * climbing while dhcp4c_get_ert_fsm_state() stays away from the bound state says
 * the client is not reaching a server. This interface does not specify when, or
 * whether, the vendor implementation clears the count.
 *
 * @param[out] pValue - Caller-allocated `int` that receives the attempt count. A
 *                      count is not negative, and this interface specifies no
 *                      upper bound on it, so a caller must not size a fixed
 *                      display field from a bound the interface never gave.
 *                      Retention beyond the call is not specified by this
 *                      interface, so a caller keeps the location allocated and
 *                      unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the attempt count. On failure its content
 *       is not specified by this interface, so a caller must not assume the
 *       previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The attempt count was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The attempt count was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed; a failure that persists leaves the count
 *                          unavailable, and a caller must not read it as a
 *                          count of zero.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_fsm_state
 */
int dhcp4c_get_ert_config_attempts(int *pValue);

/**
 * @brief Writes the name of the E-Router network interface into a caller-supplied
 *        buffer.
 *
 * The name is whatever the underlying platform calls the E-Router interface, so
 * it is vendor-specific: a caller passes it back to the system - to a socket
 * option, a routing call or a log line - rather than matching it against a fixed
 * string.
 *
 * @param[out] pName - Caller-allocated buffer of **at least 64 bytes**, which
 *                     receives the interface name as a zero-terminated string,
 *                     as the Memory Model topic of this repository's HAL
 *                     specification requires of every string this module
 *                     produces. The 64-byte figure is the only size this
 *                     interface states, and the accessor writes no more than the
 *                     buffer holds. Retention beyond the call is not specified
 *                     by this interface, so a caller keeps the buffer allocated
 *                     and unmoved after it returns.
 * @pre `pName` addresses at least 64 writable bytes. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the buffer holds the interface name, zero-terminated as that
 *       obligation requires. On failure its content is not specified by this
 *       interface, so a caller must not read a failed call's buffer as a string,
 *       and in particular must not pass it to a string function.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The interface name was written into `pName`.
 * @retval STATUS_FAILURE - The interface name was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer
 *                          and the buffer size it passed and otherwise treats
 *                          the name as unavailable.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @warning The 64-byte minimum is a contract this interface cannot enforce. The
 *          accessor receives a bare `char *` and no length, so a shorter buffer
 *          is undetectable from inside the implementation and risks an overrun;
 *          avoiding it is the caller's responsibility alone.
 * @see dhcp4c_get_ecm_ifname
 */
int dhcp4c_get_ert_ifname(char *pName);

/**
 * @brief Retrieves the state of the E-Router's DHCPv4 client state machine.
 *
 * The state says where the client stands in acquiring or holding a lease, which
 * is what separates "no address yet" from "address held" when another accessor
 * in this group fails. It is the value a caller reads first when diagnosing a
 * failure elsewhere in the E-Router set.
 *
 * @param[out] pValue - Caller-allocated `int` that receives the client state as
 *                      one of the six values listed below. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 *
 * @note Values this interface defines for `*pValue`:
 *  - 1: RELEASED or INIT_REBOOT
 *  - 2: INIT_SELECTING
 *  - 3: REQUESTING
 *  - 4: REBINDING
 *  - 5: BOUND
 *  - 6: RENEWING or RENEW_REQUESTED
 *
 * @note The six values above are the whole domain, and they are plain integers:
 *       this interface declares no enumeration for them, so a caller compares
 *       against the literals rather than against a named type. `DHCPC_CMD` and
 *       `DHCPC_MODULE` are unrelated to them - the first names kinds of
 *       information this interface reports, the second names modules - and
 *       neither may be used to interpret this value. Values 1 and 6 each cover
 *       two protocol states that this interface does not separate, so a released
 *       client cannot be told from one rebooting into INIT_REBOOT, nor a
 *       renewing client from one whose renewal has been requested.
 * @note This interface specifies the values but not the transitions between
 *       them: it states no legal successor for any value, no dwell time and no
 *       guaranteed ordering. A caller must not infer a sequence from the list
 *       above, and takes any ordering it needs from the DHCPv4 protocol
 *       specification and the vendor's documentation instead.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds one of the six values above. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The client state was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The client state was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later; it must not
 *                          read a failure as any particular state, and least of
 *                          all as "not bound".
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_fsm_state
 */
int dhcp4c_get_ert_fsm_state(int *pValue);

/**
 * @brief Retrieves the IPv4 address currently assigned to the E-Router
 *        interface.
 *
 * The address is the one the client holds under its current lease, so it is
 * meaningful only while a lease is held; dhcp4c_get_ert_fsm_state() is what tells
 * a caller whether that is the case.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the address
 *                      packed into a single 32-bit word rather than formatted as
 *                      a dotted-quad string. This interface does not specify the
 *                      byte order of that word, so a caller confirms it with the
 *                      vendor implementation instead of assuming host or network
 *                      order. Retention beyond the call is not specified by
 *                      this interface, so a caller keeps the location
 *                      allocated and unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the assigned address. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The address was read and written through `pValue`.
 * @retval STATUS_FAILURE - The address was not retrieved, and this interface
 *                          does not enumerate the conditions that produce this
 *                          code. A caller re-checks the pointer it passed and
 *                          otherwise treats the address as not yet assigned
 *                          rather than reading the location as 0.0.0.0.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_mask
 * @see dhcp4c_get_ert_gw
 */
int dhcp4c_get_ert_ip_addr(unsigned int *pValue);

/**
 * @brief Retrieves the subnet mask of the E-Router interface.
 *
 * The mask belongs with the address dhcp4c_get_ert_ip_addr() reports; together
 * they define which destinations the E-Router reaches directly and which it
 * sends to the gateway.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the mask
 *                      packed into a single 32-bit word in the same form as an
 *                      address, not as a prefix length. This interface does not
 *                      specify the byte order of that word, so a caller confirms
 *                      it with the vendor implementation instead of assuming
 *                      host or network order. Retention beyond the call is not
 *                      specified by this interface, so a caller keeps the
 *                      location allocated and unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the subnet mask. On failure its content is
 *       not specified by this interface, so a caller must not assume the
 *       previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The subnet mask was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The subnet mask was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise treats the mask as unavailable;
 *                          falling back to a classful default would invent a
 *                          subnet the lease never described.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_ip_addr
 */
int dhcp4c_get_ert_mask(unsigned int *pValue);

/**
 * @brief Retrieves the IPv4 address of the E-Router's default gateway.
 *
 * This is the router the DHCPv4 server offered along with the lease, to which
 * the E-Router sends traffic destined outside its own subnet. A lease that
 * carried no router option leaves nothing for this accessor to report.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the gateway
 *                      address packed into a single 32-bit word. This interface
 *                      does not specify the byte order of that word, so a caller
 *                      confirms it with the vendor implementation instead of
 *                      assuming host or network order. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the gateway address. On failure its content
 *       is not specified by this interface, so a caller must not assume the
 *       previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The gateway address was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The gateway address was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise treats the gateway as unknown
 *                          rather than assuming one exists somewhere on the
 *                          subnet.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_ip_addr
 */
int dhcp4c_get_ert_gw(unsigned int *pValue);

/**
 * @brief Retrieves the DNS servers offered to the E-Router with its current
 *        lease.
 *
 * The list is what the DHCPv4 server supplied, in the order it supplied them, so
 * a resolver tries them in that order. A lease may carry none, which this
 * accessor reports as a successful read of an empty list rather than as a
 * failure.
 *
 * @param[out] pList - Caller-allocated `ipv4AddrList_t`. The accessor sets
 *                     `pList->number` to the count of servers it wrote and fills
 *                     that many entries of `pList->addrList`, never more than
 *                     `MAX_IPV4_ADDR_LIST_NUMBER`; the caller reads entry `0`
 *                     through entry `pList->number - 1` and no further, even
 *                     though the array is always the full width. The accessor
 *                     allocates nothing. Retention beyond the call is not
 *                     specified by this interface, so a caller keeps the
 *                     structure allocated and unmoved after it returns.
 * @pre `pList` addresses storage for one `ipv4AddrList_t` that the callee may
 *      write. The caller need not pre-initialize it. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success `pList->number` and that many entries of `pList->addrList`
 *       are set. On failure the structure's content is not specified by this
 *       interface, so a caller must not read `number` after a failed call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The server list was read and written through `pList`.
 *                          A `number` of 0 is a valid success and means the lease
 *                          offered no DNS server.
 * @retval STATUS_FAILURE - The server list was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later rather than
 *                          treating the list as empty, which is a different
 *                          fact.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see ipv4AddrList_t
 * @see dhcp4c_get_ecm_dns_svrs
 */
int dhcp4c_get_ert_dns_svrs(ipv4AddrList_t *pList);

/**
 * @brief Retrieves the IPv4 address of the DHCPv4 server serving the E-Router.
 *
 * This is the server that granted the current lease and the one the client
 * approaches first when it renews. Until the client has reached a server there
 * is no address to report.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the server
 *                      address packed into a single 32-bit word. This interface
 *                      does not specify the byte order of that word, so a caller
 *                      confirms it with the vendor implementation instead of
 *                      assuming host or network order. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the server's address. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The server address was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The server address was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and may read dhcp4c_get_ert_fsm_state(),
 *                          which reports the client's own state independently
 *                          of this call, rather than inferring a reason for the
 *                          failure.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_fsm_state
 */
int dhcp4c_get_ert_dhcp_svr(unsigned int *pValue);

/**
 * @brief Retrieves the lease time the ECM's DHCPv4 server granted.
 *
 * The Embedded Cable Modem runs its own DHCPv4 client and takes its own lease on
 * the DOCSIS side, so this value has no relationship to the E-Router lease
 * reported by dhcp4c_get_ert_lease_time(). Like that one, it is the whole
 * duration the server granted, fixed at grant time, and does not count down.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the granted
 *                      lease time in seconds. This header states the
 *                      604800-second - seven day - ceiling for the E-Router
 *                      value only and states no separate ceiling for the ECM's,
 *                      so a caller must not treat that figure as a bound the
 *                      interface guarantees here. Retention beyond the
 *                      call is not specified by this interface, so a
 *                      caller keeps the location allocated and unmoved
 *                      after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the granted lease time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The lease time was read and written through `pValue`.
 * @retval STATUS_FAILURE - The lease time was not retrieved, and this interface
 *                          does not enumerate the conditions that produce this
 *                          code. A caller re-checks the pointer it passed and
 *                          otherwise reads again later rather than treating the
 *                          lease as zero-length.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_remain_lease_time
 */
int dhcp4c_get_ecm_lease_time(unsigned int *pValue);

/**
 * @brief Retrieves how much of the ECM's current DHCP lease is left to run.
 *
 * The value falls as the ECM's lease ages and reaches zero when it expires. It
 * never exceeds the granted lease time reported by dhcp4c_get_ecm_lease_time(),
 * and it moves independently of the E-Router's timers.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the
 *                      remaining lease time in seconds. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining lease time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining lease time was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The remaining lease time was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later; a failure
 *                          here is not a report that the lease has expired,
 *                          which is a successful read of zero.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_lease_time
 * @see dhcp4c_get_ecm_remain_renew_time
 */
int dhcp4c_get_ecm_remain_lease_time(unsigned int *pValue);

/**
 * @brief Retrieves how long until the ECM's DHCPv4 client starts renewing.
 *
 * Renewal is the ECM client's first attempt to extend its lease with the server
 * that granted it. This value counts down to that moment, so it is never greater
 * than the ECM's remaining lease time and never greater than its remaining
 * rebind time.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the seconds
 *                      remaining before renewal begins. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining renewal time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining renewal time was read and written
 *                          through `pValue`.
 * @retval STATUS_FAILURE - The remaining renewal time was not retrieved, and
 *                          this interface does not enumerate the conditions
 *                          that produce this code. A caller re-checks the
 *                          pointer it passed and otherwise reads again later.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_remain_rebind_time
 */
int dhcp4c_get_ecm_remain_renew_time(unsigned int *pValue);

/**
 * @brief Retrieves how long until the ECM's DHCPv4 client starts rebinding.
 *
 * Rebinding is the ECM client's fallback when renewal with the granting server
 * has not succeeded: it solicits any reachable server instead. This value counts
 * down to that moment, so it sits between the ECM's remaining renewal time and
 * its remaining lease time.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the seconds
 *                      remaining before rebinding begins. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining rebind time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining rebind time was read and written
 *                          through `pValue`.
 * @retval STATUS_FAILURE - The remaining rebind time was not retrieved, and
 *                          this interface does not enumerate the conditions
 *                          that produce this code. A caller re-checks the
 *                          pointer it passed and otherwise reads again later.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_remain_renew_time
 */
int dhcp4c_get_ecm_remain_rebind_time(unsigned int *pValue);

/**
 * @brief Retrieves how many times the ECM's DHCPv4 client has tried to configure
 *        its interface.
 *
 * As with the E-Router count, this is a diagnostic: a number that keeps climbing
 * while dhcp4c_get_ecm_fsm_state() stays away from the bound state says the ECM's
 * client is not reaching a server. This interface does not specify when, or
 * whether, the vendor implementation clears the count.
 *
 * @param[out] pValue - Caller-allocated `int` that receives the attempt count. A
 *                      count is not negative, and this interface specifies no
 *                      upper bound on it. Retention beyond the call is not
 *                      specified by this interface, so a caller keeps the
 *                      location allocated and unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the attempt count. On failure its content
 *       is not specified by this interface, so a caller must not assume the
 *       previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The attempt count was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The attempt count was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed; a failure that persists leaves the count
 *                          unavailable, and a caller must not read it as a
 *                          count of zero.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_fsm_state
 */
int dhcp4c_get_ecm_config_attempts(int *pValue);

/**
 * @brief Writes the name of the ECM network interface into a caller-supplied
 *        buffer.
 *
 * The name is whatever the underlying platform calls the Embedded Cable Modem
 * interface, so it is vendor-specific and distinct from the E-Router's: a caller
 * passes it back to the system rather than matching it against a fixed string.
 *
 * @param[out] pName - Caller-allocated buffer of **at least 64 bytes**, which
 *                     receives the interface name as a zero-terminated string,
 *                     as the Memory Model topic of this repository's HAL
 *                     specification requires of every string this module
 *                     produces. The 64-byte figure is the only size this
 *                     interface states, and the accessor writes no more than the
 *                     buffer holds. Retention beyond the call is not specified
 *                     by this interface, so a caller keeps the buffer allocated
 *                     and unmoved after it returns.
 * @pre `pName` addresses at least 64 writable bytes. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the buffer holds the interface name, zero-terminated as that
 *       obligation requires. On failure its content is not specified by this
 *       interface, so a caller must not read a failed call's buffer as a string,
 *       and in particular must not pass it to a string function.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The interface name was written into `pName`.
 * @retval STATUS_FAILURE - The interface name was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer
 *                          and the buffer size it passed and otherwise treats
 *                          the name as unavailable.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @warning The 64-byte minimum is a contract this interface cannot enforce. The
 *          accessor receives a bare `char *` and no length, so a shorter buffer
 *          is undetectable from inside the implementation and risks an overrun;
 *          avoiding it is the caller's responsibility alone.
 * @see dhcp4c_get_ert_ifname
 */
int dhcp4c_get_ecm_ifname(char *pName);

/**
 * @brief Retrieves the state of the ECM's DHCPv4 client state machine.
 *
 * The state says where the ECM's client stands in acquiring or holding its own
 * lease, and it moves independently of the E-Router's. It is the value a caller
 * reads first when another ECM accessor fails, because it distinguishes a client
 * that has nothing to report yet from one whose value could not be read.
 *
 * @param[out] pValue - Caller-allocated `int` that receives the client state as
 *                      one of the six values listed below. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 *
 * @note Values this interface defines for `*pValue`:
 *  - 1: RELEASED or INIT_REBOOT
 *  - 2: INIT_SELECTING
 *  - 3: REQUESTING
 *  - 4: REBINDING
 *  - 5: BOUND
 *  - 6: RENEWING or RENEW_REQUESTED
 *
 * @note The domain is the same six plain integers dhcp4c_get_ert_fsm_state()
 *       reports, and this interface declares no enumeration for them, so a caller
 *       compares against the literals rather than against a named type. Neither
 *       `DHCPC_CMD` nor `DHCPC_MODULE` describes these values: the first names
 *       kinds of information this interface reports and the second names modules,
 *       and using either to interpret this value would mis-read it. Values 1 and
 *       6 each cover two protocol states this interface does not separate.
 * @note This interface specifies the values but not the transitions between
 *       them: it states no legal successor for any value, no dwell time and no
 *       guaranteed ordering, so a caller must not infer a sequence from the list
 *       above.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds one of the six values above. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The client state was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The client state was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later; it must not
 *                          read a failure as any particular state.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ert_fsm_state
 */
int dhcp4c_get_ecm_fsm_state(int *pValue);

/**
 * @brief Retrieves the IPv4 address currently assigned to the ECM interface.
 *
 * The address is the one the ECM's client holds under its current lease, so it is
 * meaningful only while that lease is held; dhcp4c_get_ecm_fsm_state() is what
 * tells a caller whether that is the case.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the address
 *                      packed into a single 32-bit word rather than formatted as
 *                      a dotted-quad string. This interface does not specify the
 *                      byte order of that word, so a caller confirms it with the
 *                      vendor implementation instead of assuming host or network
 *                      order. Retention beyond the call is not specified by
 *                      this interface, so a caller keeps the location
 *                      allocated and unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the assigned address. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The address was read and written through `pValue`.
 * @retval STATUS_FAILURE - The address was not retrieved, and this interface
 *                          does not enumerate the conditions that produce this
 *                          code. A caller re-checks the pointer it passed and
 *                          otherwise treats the address as not yet assigned
 *                          rather than reading the location as 0.0.0.0.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_mask
 * @see dhcp4c_get_ecm_gw
 */
int dhcp4c_get_ecm_ip_addr(unsigned int *pValue);

/**
 * @brief Retrieves the subnet mask of the ECM interface.
 *
 * The mask belongs with the address dhcp4c_get_ecm_ip_addr() reports; together
 * they define which destinations the ECM reaches directly and which it sends to
 * its gateway.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the mask
 *                      packed into a single 32-bit word in the same form as an
 *                      address, not as a prefix length. This interface does not
 *                      specify the byte order of that word, so a caller confirms
 *                      it with the vendor implementation instead of assuming
 *                      host or network order. Retention beyond the call is not
 *                      specified by this interface, so a caller keeps the
 *                      location allocated and unmoved after it returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the subnet mask. On failure its content is
 *       not specified by this interface, so a caller must not assume the
 *       previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The subnet mask was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The subnet mask was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise treats the mask as unavailable;
 *                          falling back to a classful default would invent a
 *                          subnet the lease never described.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_ip_addr
 */
int dhcp4c_get_ecm_mask(unsigned int *pValue);

/**
 * @brief Retrieves the IPv4 address of the ECM's default gateway.
 *
 * This is the router the ECM's DHCPv4 server offered along with its lease, to
 * which the ECM sends traffic destined outside its own subnet. A lease that
 * carried no router option leaves nothing for this accessor to report.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the gateway
 *                      address packed into a single 32-bit word. This interface
 *                      does not specify the byte order of that word, so a caller
 *                      confirms it with the vendor implementation instead of
 *                      assuming host or network order. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the gateway address. On failure its content
 *       is not specified by this interface, so a caller must not assume the
 *       previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The gateway address was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The gateway address was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise treats the gateway as unknown
 *                          rather than assuming one exists somewhere on the
 *                          subnet.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_ip_addr
 */
int dhcp4c_get_ecm_gw(unsigned int *pValue);

/**
 * @brief Retrieves the DNS servers offered to the ECM with its current lease.
 *
 * The list is what the ECM's DHCPv4 server supplied, in the order it supplied
 * them, and it is independent of the E-Router's list. A lease may carry none,
 * which this accessor reports as a successful read of an empty list rather than
 * as a failure.
 *
 * @param[out] pList - Caller-allocated `ipv4AddrList_t`. The accessor sets
 *                     `pList->number` to the count of servers it wrote and fills
 *                     that many entries of `pList->addrList`, never more than
 *                     `MAX_IPV4_ADDR_LIST_NUMBER`; the caller reads entry `0`
 *                     through entry `pList->number - 1` and no further, even
 *                     though the array is always the full width. The accessor
 *                     allocates nothing. Retention beyond the call is not
 *                     specified by this interface, so a caller keeps the
 *                     structure allocated and unmoved after it returns.
 * @pre `pList` addresses storage for one `ipv4AddrList_t` that the callee may
 *      write. The caller need not pre-initialize it. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success `pList->number` and that many entries of `pList->addrList`
 *       are set. On failure the structure's content is not specified by this
 *       interface, so a caller must not read `number` after a failed call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The server list was read and written through `pList`.
 *                          A `number` of 0 is a valid success and means the lease
 *                          offered no DNS server.
 * @retval STATUS_FAILURE - The server list was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later rather than
 *                          treating the list as empty, which is a different
 *                          fact.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see ipv4AddrList_t
 * @see dhcp4c_get_ert_dns_svrs
 */
int dhcp4c_get_ecm_dns_svrs(ipv4AddrList_t *pList);

/**
 * @brief Retrieves the IPv4 address of the DHCPv4 server serving the ECM.
 *
 * This is the server that granted the ECM's current lease and the one its client
 * approaches first when it renews. Until that client has reached a server there
 * is no address to report.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the server
 *                      address packed into a single 32-bit word. This interface
 *                      does not specify the byte order of that word, so a caller
 *                      confirms it with the vendor implementation instead of
 *                      assuming host or network order. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the server's address. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The server address was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The server address was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and may read dhcp4c_get_ecm_fsm_state(),
 *                          which reports the client's own state independently
 *                          of this call, rather than inferring a reason for the
 *                          failure.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @see dhcp4c_get_ecm_fsm_state
 */
int dhcp4c_get_ecm_dhcp_svr(unsigned int *pValue);

#if !defined (NO_MTA_FEATURE_SUPPORT)
/**
 * @brief Retrieves how much of the eMTA's current DHCP lease is left to run.
 *
 * The Embedded Multimedia Terminal Adapter runs a third DHCPv4 client with a
 * lease of its own, unrelated to the E-Router's and the ECM's. This value falls
 * as that lease ages and reaches zero when it expires.
 *
 * The eMTA is the narrowest of the three modules in this interface: it exposes
 * only the three remaining-time accessors below. There is no eMTA accessor for a
 * granted lease time, a configuration attempt count, an interface name, a client
 * state, an address, a mask, a gateway, a DNS server list or a server address,
 * so a caller that needs to know why an eMTA timer is unavailable cannot read an
 * eMTA state to find out - the state accessors exist for the E-Router and the ECM
 * only.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the
 *                      remaining lease time in seconds. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining lease time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining lease time was read and written through
 *                          `pValue`.
 * @retval STATUS_FAILURE - The remaining lease time was not retrieved, and this
 *                          interface does not enumerate the conditions that
 *                          produce this code. A caller re-checks the pointer it
 *                          passed and otherwise reads again later; a failure
 *                          here is not a report that the lease has expired,
 *                          which is a successful read of zero.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @note Declared only where `NO_MTA_FEATURE_SUPPORT` is undefined, which is the
 *       compile-time variation the `Optional Components` and
 *       `Variability Management` topics of that specification describe. On a
 *       build that defines the flag this accessor does not exist and a reference
 *       to it fails to compile, so a caller supporting both builds guards its own
 *       use with the same condition. The flag also removes the `DHCPC_EMTA`
 *       enumerator, and it must be applied consistently to the caller and to the
 *       implementation the caller links against.
 * @see dhcp4c_get_emta_remain_renew_time
 */
int dhcp4c_get_emta_remain_lease_time(unsigned int *pValue);

/**
 * @brief Retrieves how long until the eMTA's DHCPv4 client starts renewing.
 *
 * Renewal is the eMTA client's first attempt to extend its lease with the server
 * that granted it. This value counts down to that moment, so it is never greater
 * than the eMTA's remaining lease time and never greater than its remaining
 * rebind time.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the seconds
 *                      remaining before renewal begins. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining renewal time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining renewal time was read and written
 *                          through `pValue`.
 * @retval STATUS_FAILURE - The remaining renewal time was not retrieved, and
 *                          this interface does not enumerate the conditions
 *                          that produce this code. A caller re-checks the
 *                          pointer it passed and otherwise reads again later.
 *                          This interface declares no eMTA state accessor, so a
 *                          caller has nothing here with which to qualify a
 *                          repeated failure and treats the value as
 *                          unavailable.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @note Declared only where `NO_MTA_FEATURE_SUPPORT` is undefined; on a build
 *       that defines the flag this accessor does not exist and a reference to it
 *       fails to compile, so a caller supporting both builds guards its own use
 *       with the same condition.
 * @see dhcp4c_get_emta_remain_rebind_time
 */
int dhcp4c_get_emta_remain_renew_time(unsigned int *pValue);

/**
 * @brief Retrieves how long until the eMTA's DHCPv4 client starts rebinding.
 *
 * Rebinding is the eMTA client's fallback when renewal with the granting server
 * has not succeeded: it solicits any reachable server instead. This value counts
 * down to that moment, so it sits between the eMTA's remaining renewal time and
 * its remaining lease time.
 *
 * @param[out] pValue - Caller-allocated `unsigned int` that receives the seconds
 *                      remaining before rebinding begins. Retention beyond the
 *                      call is not specified by this interface, so a caller
 *                      keeps the location allocated and unmoved after it
 *                      returns.
 * @pre `pValue` addresses storage the callee may write. Nothing has to be
 *      initialized first: this interface declares no initialization call.
 * @post On success the location holds the remaining rebind time. On failure its
 *       content is not specified by this interface, so a caller must not assume
 *       the previous value survived the call.
 * @returns Status of the operation, as one of the two values below.
 * @retval STATUS_SUCCESS - The remaining rebind time was read and written
 *                          through `pValue`.
 * @retval STATUS_FAILURE - The remaining rebind time was not retrieved, and
 *                          this interface does not enumerate the conditions
 *                          that produce this code. A caller re-checks the
 *                          pointer it passed and otherwise reads again later.
 * @note Blocking: this interface does not establish whether this accessor may
 *       block, and it states no bound on how long a call may take - no timeout
 *       argument, no cancellation call, no asynchronous variant. A caller
 *       places the call where a delay is survivable and imposes and enforces
 *       its own bound if it needs one, and a test must not assert a completion
 *       time this interface does not state. The group contract above states
 *       this position for the whole family.
 * @note Not thread safe by contract: the `Threading Model` topic
 *       of that specification places serialisation on the calling
 *       module, and its `Process Model` topic places protection against
 *       concurrent callers on the implementation.
 * @note Declared only where `NO_MTA_FEATURE_SUPPORT` is undefined; on a build
 *       that defines the flag this accessor does not exist and a reference to it
 *       fails to compile, so a caller supporting both builds guards its own use
 *       with the same condition.
 * @see dhcp4c_get_emta_remain_renew_time
 */
int dhcp4c_get_emta_remain_rebind_time(unsigned int *pValue);
#endif
/** @} */  //END OF GROUP DHCPV4C_HAL_APIS

#endif
