
#include <assert.h>
#include <mm/core_memprot.h>
#include <kernel/pseudo_ta.h>
#include <string.h>

#include <pta_phy_to_vir.h>

static uint32_t get_instance_id(struct tee_ta_session *sess)
{
	return sess->ctx->ops->get_instance_id(sess->ctx);
}




static TEE_Result phy_to_vir(uint32_t instance_id, uint32_t param_types,
			      TEE_Param params[TEE_NUM_PARAMS])
{
	struct mobj *mobj;
	TEE_Result res;
    paddr_t pa;
	void *va;
	int print_IO_cost;
	TEE_Time start, end, cost;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_MEMREF_OUTPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT);

	if (exp_pt != param_types) {
		DMSG("got param_types 0x%x, expected 0x%x",
		     param_types, exp_pt);
		return TEE_ERROR_BAD_PARAMETERS;
	}
	pa = params[0].value.a;
    va = phys_to_virt_tee_ram(pa);
	params[0].value.b = &va;

	// IMSG("Pseudo TA TruzGPS_read_gps buffer size = %u", params[2].memref.size);

	if (print_IO_cost) {
		res = tee_time_get_sys_time(&start);
		if (res != TEE_SUCCESS)
			return res;
	}

	
	

	if (print_IO_cost) {
		res = tee_time_get_sys_time(&end);
		if (res != TEE_SUCCESS)
			return res;
		time_diff(start, end, &cost);
		params[3].value.a = cost.seconds;
		params[3].value.b = cost.millis;
	}

	return res;
}

typedef TEE_Result (*ta_func)(uint32_t instance_id, uint32_t param_types,
			      TEE_Param params[TEE_NUM_PARAMS]);

static const ta_func ta_funcs[] = {
	[PTA_PHY_TO_VIR] = phy_to_vir,
};

/*
 * Trusted Application Entry Points
 */

static TEE_Result pta_phy_to_vir_open_session(uint32_t param_types __unused,
			TEE_Param pParams[TEE_NUM_PARAMS] __unused,
			void **sess_ctx)
{
	struct tee_ta_session *s;

	/* Check that we're called from a TA */
	s = tee_ta_get_calling_session();
	if (!s)
		return TEE_ERROR_ACCESS_DENIED;

	*sess_ctx = (void *)(vaddr_t)get_instance_id(s);

	return TEE_SUCCESS;
}

static void pta_phy_to_vir_close_session(void *sess_ctx)
{
/*
	TEE_Result res;
	struct thread_param tpm = {
		.attr = THREAD_PARAM_ATTR_VALUE_IN, .u.value = {
			.a = OPTEE_RPC_SOCKET_CLOSE_ALL, .b = (vaddr_t)sess_ctx,
		},
	};
	res = thread_rpc_cmd(OPTEE_RPC_CMD_SOCKET, 1, &tpm);
	if (res != TEE_SUCCESS)
		DMSG("OPTEE_RPC_SOCKET_CLOSE_ALL failed: %#" PRIx32, res);
*/
}

static TEE_Result pta_phy_to_vir_invoke_command(void *sess_ctx, uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	if (cmd_id < ARRAY_SIZE(ta_funcs) && ta_funcs[cmd_id])
		return ta_funcs[cmd_id]((vaddr_t)sess_ctx, param_types, params);

	return TEE_ERROR_NOT_IMPLEMENTED;
}

pseudo_ta_register(.uuid = PTA_PHY_TO_VIR_UUID, .name = "phy_to_vir",
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
		   .open_session_entry_point = pta_phy_to_vir_open_session,
		   .close_session_entry_point = pta_phy_to_vir_close_session,
		   .invoke_command_entry_point = pta_phy_to_vir_invoke_command);

