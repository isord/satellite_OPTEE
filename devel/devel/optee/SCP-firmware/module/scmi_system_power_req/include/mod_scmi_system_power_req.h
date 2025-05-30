/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SCMI_SYSTEM_POWER_REQ_H
#define MOD_SCMI_SYSTEM_POWER_REQ_H

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMISystemPowerRequester SCMI System Power Requester
 * \{
 */

/*!
 * \brief Set state configurations
 */
struct scmi_sys_power_req_state_set_a2p {
    /*! System Power command flags*/
    uint32_t flags;

    /*! The state being transitioned to.*/
    uint32_t system_state;
};

/* Supported notifications */
enum mod_system_power_req_notifications {
    MOD_SYS_POWER_REQ_STATE_CHANGE_NOTIFICATION,
    MOD_SYS_POWER_REQ_NOTIFICATION_COUNT,
};

/* System change notification */
static const fwk_id_t mod_scmi_system_power_notification_system_power_change =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ,
        MOD_SYS_POWER_REQ_STATE_CHANGE_NOTIFICATION);

/*!
 * \brief System Power Requester module restricted interface.
 *
 * \details The interface the system power requester module exposes to a
 *          restricted set of modules. The set of modules that are allowed
 *          to access this interface is defined by the module configuration
 *          data.
 */
struct mod_system_power_requester_api {
    /*!
     * \brief Request an asynchronous power state transition.
     *
     * \warning Successful completion of this function does not indicate
     *      completion of a transition, but instead that a request has been
     *      submitted.
     *
     * \param id ID to determine which element to set the state for.
     *
     * \param resp_requested True if the caller wants to be notified with an
     *      event response at the end of the request processing.
     *
     * \param state State the system power requester has to put and request
     *      from the rest of the system.
     *
     * \param flags The SCMI System Power command flags to denote if graceful
     *      shutdown or not.
     *
     * \retval ::FWK_PENDING The power state transition request was submitted.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_req_state)(
        fwk_id_t id,
        bool response_requested,
        uint32_t state,
        uint32_t flags);

    /*!
     * \brief Get the state of all the elements of the System Power
     *        Requester.
     *
     * \param[out] state The System Power Requester.
     *
     * \retval ::FWK_SUCCESS The system power requester state was returned.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `state` parameter was a null pointer value.
     *
     * \return Status code representing the result of the operation.
     *
     */
    int (*get_req_state)(fwk_id_t id, uint32_t *state);

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /*!
     * \brief Subscribe to scmi notifications of system power state change
     *        Used for agents.
     * \param id ID to determine which element to subscribe
     *
     * \retval ::FWK_SUCCESS The subscription message is sent successfully.
     *
     * \retval ::FWK_PENDING The power state transition request was submitted.
     *
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     */
    int (*notification_subscribe)(fwk_id_t id);
#endif
};

/*!
 * \brief API indices
 */
enum mod_sys_power_req_api_idx {
    /*! API used for sending SCMI commands and receive responses */
    MOD_SYS_POW_REQ_API_IDX_SCMI_REQ,
    /*! API used to set and get the state from another module */
    MOD_SYS_POW_REQ_API_IDX_REQ,
    MOD_SYS_POW_REQ_API_IDX_COUNT,
};

/*!
 * \brief SCMI system power platform configuration
 */
struct mod_scmi_system_power_req_dev_config {
    /*!
     * \brief SCMI Service ID
     *
     * \details The service ID which corresponds to the required
     *      channel in the transport layer.
     */
    fwk_id_t service_id;

    /*!
     * \brief System Power State at Startup
     *
     * \details The state at which these should be initialised to at the start
     * of world.
     */
    uint32_t start_state;
};

/*! Identifier of the system power req API */
static const fwk_id_t mod_sys_power_req_api_id = FWK_ID_API_INIT(
    FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ,
    MOD_SYS_POW_REQ_API_IDX_REQ);

/*! Identifier of the system power req SCMI API */
static const fwk_id_t mod_sys_power_req_scmi_api_id = FWK_ID_API_INIT(
    FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ,
    MOD_SYS_POW_REQ_API_IDX_SCMI_REQ);

/*!
 * System Power Requestor set state request event index
 */
#define MOD_SCMI_SPR_EVENT_IDX_SET_STATE 0

/*!
 * \brief Read request event identifier.
 *
 * \details Clients which expect to receive a response event from this module
 *      should use this identifier to properly identify the response.
 */
static const fwk_id_t system_power_requester_set_state_request =
    FWK_ID_EVENT_INIT(
        FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ,
        MOD_SCMI_SPR_EVENT_IDX_SET_STATE);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_SYSTEM_POWER_REQ_H */
