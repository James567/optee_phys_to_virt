/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2020, Gaga Chang
 */

#ifndef __PTA_PHYTOVIR
#define __PTA_PHYTOVIR

/* 3a8e484e-6535-11eb-ae93-0242ac130002 */
#define PTA_PHY_TO_VIR_UUID { 0x3a8e484e, 0x6535, 0x11eb, { \
			  0xae, 0x93, 0x02, 0x42, 0xac, 0x13, 0x00, 0x02 } }

/*
 * [in]		value[0].a	physical address
 * [out]	value[0].b	virtual address
 */
#define PTA_PHY_TO_VIR		1

#endif /*__PTA_PHYTOVIR */
