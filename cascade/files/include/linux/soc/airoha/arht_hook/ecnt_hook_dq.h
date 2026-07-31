#ifndef _ECNT_HOOK_DQ_H
#define _ECNT_HOOK_DQ_H

#include "ecnt_hook.h"
#include "ecnt_hook_dq_type.h"

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

typedef enum {
	ECNT_DQ_API,
}DQ_Api_SubType_t;


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
	

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline unsigned int DQ_API_QUEUE_DISABLE(DQ_SEL dq_sel,uint32_t chn_id,uint32_t q_id,int disable)
{
	ecnt_dq_data_t in_data;
	int ret=0;

	in_data.function_id = DQ_FUNCTION_QUEUE_DISABLE;
	in_data.dq_sel = dq_sel;
	in_data.chn_id = chn_id;
	in_data.q_id = q_id;
	in_data.api_data.q_disable = disable;
	ret = __ECNT_HOOK(ECNT_DQ, ECNT_DQ_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return ECNT_CONTINUE;
	else
		return in_data.retValue;
}

static inline unsigned int DQ_API_GET_QVLD(DQ_SEL dq_sel,uint32_t chn_id,uint32_t q_id,int* qvld)
{
	ecnt_dq_data_t in_data;
	int ret=0;

	in_data.function_id = DQ_FUNCTION_GET_QVLD;
	in_data.dq_sel = dq_sel;
	in_data.chn_id = chn_id;
	in_data.q_id = q_id;
	ret = __ECNT_HOOK(ECNT_DQ, ECNT_DQ_API, (struct ecnt_data *)&in_data);
	*qvld = in_data.api_data.qvld;
	if(ret != ECNT_HOOK_ERROR)
		return ECNT_CONTINUE;
	else
		return in_data.retValue;
}

#endif


