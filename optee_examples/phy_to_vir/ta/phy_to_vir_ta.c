/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <phy_to_vir_ta.h>
#include <pta_phy_to_vir.h>
/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */

static TEE_TASessionHandle pta_session = TEE_HANDLE_NULL;
static const TEE_UUID pta_phy_to_vir_uuid = PTA_PHY_TO_VIR_UUID;
static uint32_t pta_param_types;
static TEE_Param pta_params[TEE_NUM_PARAMS];
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	TEE_Result res;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;
	res = TEE_OpenTASession(&pta_phy_to_vir_uuid, TEE_TIMEOUT_INFINITE,
					0, NULL, &pta_session, NULL);
		if (res != TEE_SUCCESS) {
			IMSG("TEE_OpenTASession to Pseudo TA failed.");
			return res;
		}
		IMSG("TA Open Pseudo TA session successfully.");

		/* Prepare the TEE parameters */
		pta_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						  TEE_PARAM_TYPE_VALUE_INPUT,
						  TEE_PARAM_TYPE_MEMREF_OUTPUT,
						  TEE_PARAM_TYPE_VALUE_OUTPUT);
	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}



static TEE_Result get_virtual_address_from_pta(uint32_t param_types,
		TEE_Param params[4]			)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	TEE_Result res;
	unsigned long pa;
	// DMSG("TA get_virtual_address_from_pta has been called");
	pa = 0xfe000000;
	TEE_MemFill(pta_params, 0, sizeof(TEE_Param) * 4);
	

	/* Set the TEE parameters */
	pta_params[0].value.a = pa;		   	/* physical */
	pta_params[0].value.b = 0;			/* virtual */


	/* Invoke the PTA to get GPS data from serial port */
	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				  PTA_PHY_TO_VIR,
				  pta_param_types, pta_params, NULL);

	if (res != TEE_SUCCESS) {
		EMSG("TEE_InvokeTACommand failed with code 0x%x", res);
		return res;
	}

	EMSG("virtual address is  0x%x", pta_params[0].value.b);

	return res;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_PHY_TO_VIR:
		return get_virtual_address_from_pta(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
