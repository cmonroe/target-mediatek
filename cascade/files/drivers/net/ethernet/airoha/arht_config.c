// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author:  2024 AIROHA Inc
 */


#include "arht_hsgmii.h"

PhySerdes_t serdes_map[SERDES_MAX_IDX] = {
    [SERDES_PCIE0_IDX] = {.phy_addr = 0, .serdes_id = SERDES_PCIE0_IDX},
    [SERDES_PCIE1_IDX] = {.phy_addr = 0, .serdes_id = SERDES_PCIE1_IDX},
    [SERDES_USB_IDX]   = {.phy_addr = 0, .serdes_id = SERDES_USB_IDX},
    [SERDES_AE_IDX]    = {.phy_addr = 0, .serdes_id = SERDES_AE_IDX},
    [SERDES_ETH_IDX]   = {.phy_addr = 15, .serdes_id = SERDES_ETH_IDX},
};

PhySerdes_t serdes_map_aeon[SERDES_MAX_IDX] = {
    [SERDES_PCIE0_IDX] = {.phy_addr = 0, .serdes_id = SERDES_PCIE0_IDX},
    [SERDES_PCIE1_IDX] = {.phy_addr = 0, .serdes_id = SERDES_PCIE1_IDX},
    [SERDES_USB_IDX]   = {.phy_addr = 0, .serdes_id = SERDES_USB_IDX},
    [SERDES_AE_IDX]    = {.phy_addr = 0, .serdes_id = SERDES_AE_IDX},
    [SERDES_ETH_IDX]   = {.phy_addr = 31, .serdes_id = SERDES_ETH_IDX},
};

char *itfname_info[MAX_SERDES_NUM] = {
    [SERDES_PCIE0_IDX] = "eth4",
    [SERDES_PCIE1_IDX] = "eth5",
    [SERDES_USB_IDX]   = "eth3",
    [SERDES_AE_IDX]    = "pon",
    [SERDES_ETH_IDX]   = "eth2",
};

char *serdes_info[MAX_SERDES_NUM] ={
    [SERDES_PCIE0_IDX] = "SERDES_PCIE0",
    [SERDES_PCIE1_IDX] = "SERDES_PCIE1",
    [SERDES_USB_IDX] = "SERDES_USB",
    [SERDES_AE_IDX] = "SERDES_PON",
    [SERDES_ETH_IDX] = "SERDES_ETH",
};


