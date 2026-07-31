#ifndef __ARHT_HSGMII_H_
#define __ARHT_HSGMII_H_


#include <linux/phy.h>
#include <linux/mii.h>
#include <linux/platform_device.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/version.h>

#include "airoha_eth.h"

extern struct airoha_eth *glb_eth;

#define MAX_MDIO_BUS 2
#define EN8811_JUDGE_INPUT_ITFNAME(str)  (((strstr((str), "itf="))) ? EN8811_TRUE: EN8811_FALSE)
#define MAX_BUF     64
#define MAX_CMD_LEN 32
#define MAX_ARGS    10
#define INVALID_ITF_NAME "None"
#define STRING_LEN 40
#define CMD_LEN 16

#define phydev_mdio_bus(_dev) (_dev->mdio.bus)
#define phydev_addr(_dev) (_dev->mdio.addr)
#define phydev_dev(_dev) (&_dev->mdio.dev)

struct downshift_cfg {
	uint8_t enable;
	uint8_t retry_limit;
};

struct an_mdi_cfg {
  uint8_t top_spd;
  uint8_t eee_spd;
  uint8_t fr_spd;
  uint8_t thp_byp;
  uint8_t port_type;
  uint8_t ms_en;
  uint8_t ms_config;
  uint8_t nstd_pbo;
  struct downshift_cfg smt_spd;
  uint8_t trd_ovrd;
  uint8_t trd_swap;
  uint8_t cfr;
};

struct arht_mdio_priv {
	struct mii_bus *mii_bus;
};

struct parsed_cmd {
    char cmd[MAX_CMD_LEN];
    long args[MAX_ARGS];
    int argc;
};

typedef struct _parse_data{
	struct phy_device* phyDev;
	struct mii_bus *mbus;
	uint8_t phyAddr;
	char cmd[CMD_LEN];
	char param[CMD_LEN];
	char subparam[CMD_LEN];
	char phyItfName[CMD_LEN];
}PARSE_DATA_T;

typedef enum
{
    AIR_PORT_MODE_FORCE_100,
    AIR_PORT_MODE_FORCE_1000,
    AIR_PORT_MODE_FORCE_2500,
    AIR_PORT_MODE_AUTONEGO,
    AIR_PORT_MODE_POWER_DOWN,
    AIR_PORT_MODE_POWER_UP,
    AIR_PORT_MODE_POWER_STATE,
    AIR_PORT_MODE_LAST = 0xff,
} AIR_PORT_MODE_T;

typedef enum
{
    EN8811_LOG_OFF = 0,
    EN8811_LOG_ERROR,
    EN8811_LOG_WARNING,
    EN8811_LOG_INFO,
    EN8811_LOG_DBG,
    EN8811_LOG_DUMP_DATA, 
    EN8811_LOG_MAX_LEVEL
} EN8811_LOG_LEVEL;

extern int debuglevel;
#define EN8811_LOG(Level,fmt,args...)          \
        {                                   \
            if (Level <= debuglevel)        \
            {                               \
                printk("EN8811@ [%s:%u]==="fmt,__func__,__LINE__,##args);\
            }                               \
        }

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;


typedef enum {
    EN8811_ERROR = -1,
    EN8811_CONTINUE = 1
}EN8811_return_t;

#define    EN8811_READ  0
#define    EN8811_WRITE 1
#define    EN8811_WRITE_0 0
#define    EN8811_WRITE_1 1

//auto-negotation or force 
#define EN8811_AUTONEGO_REG 0       //reg0
#define EN8811_AUTONEGO_BIT (1<<12) //reg0
#define EN8811_AN_COMPLETE_REG  1     //reg1
#define EN8811_AN_COMPLETE_BIT (1<<5) //reg1

//restart an mode
#define EN8811_RestartAN_REG 0      //reg0
#define EN8811_RestartAN_BIT (1<<9) //reg0

//full duplex or hslf duplex
#define EN8811_DUPLEX_REG 0      //reg0
#define EN8811_DUPLEX_BIT (1<<8) //reg0
#define EN8811_FULL_DUPLEX 1
#define EN8811_HALF_DUPLEX 0

//status
#define EN8811_STATUS_REG 1      //reg0x1
#define EN8811_STATUS_BIT (1<<2) //reg0x1

//flow control
#define EN8811_FC_REG 4          //reg4
#define EN8811_FC_BIT (1<<10)    //reg4

//speed of autonego mode
#define EN8811_SPEED_100M_REG1 4      //reg4
#define EN8811_SPEED_100M_REG2 5      //reg5
#define EN8811_SPEED_100M_BIT (1<<8)  //reg4,reg5
    
#define EN8811_SPEED_1000M_REG1 9        //reg9
#define EN8811_SPEED_1000M_BIT1 (1<<9)  //reg9
#define EN8811_SPEED_1000M_REG2 10       //reg10
#define EN8811_SPEED_1000M_BIT2 (1<<11) //reg10
    
#define EN8811_SPEED_2500M_REG_BUCKPBUS 0x109D4
#define EN8811_SPEED_2500M_BIT (1<<4) //reg24

//speed of force mode    
#define EN8811_SPEED_FORCE_REG 0        //reg0
#define EN8811_SPEED_FORCE_BIT1 (1<<6)  //reg0
#define EN8811_SPEED_FORCE_BIT2 (1<<13) //reg0

#define EN8811_FIRMWARE_REG_BUCKPBUS 0x3B3C

//an mode to advertise
#define EN8811_ADV_2500M_REG_CL45_DEV 0x07 
#define EN8811_ADV_2500M_REG_CL45_REG 0x20
#define EN8811_ADV_2500M_BIT          (1<<7)

#define EN8811_ADV_1000M_REG 9
#define EN8811_ADV_1000M_BIT (1<<9)

#define EN8811_ADV_100M_REG 4
#define EN8811_ADV_100M_BIT (1<<8)
#define PHY_SUPPORT_CL45 1

#define MIN_PHY_ADDR_EN8811 8
#define MAX_PHY_ADDR_EN8811 15
#define MIN_PHY_ADDR_AN8831 28
#define MAX_PHY_ADDR_AN8831 31
#define INVALID_PHY_ADDR (MAX_PHY_ADDR_AN8831+1)
#define MAX_PHY_DEVICES  (MAX_PHY_ADDR_EN8811+1) 

typedef enum{
    EN8811_FALSE = 0,
    EN8811_TRUE =  1
}EN8811_BOOL;

typedef enum{
    EN8811_PHY_OFF = 0,
    EN8811_PHY_ON = 1
}PHY_STATUS_t;

#define MAX_SERDES_NUM SERDES_MAX_IDX

typedef struct{
    uint8_t phy_addr;
    uint8_t serdes_id;
}PhySerdes_t;

extern char *serdes_info[MAX_SERDES_NUM];
#define GET_SERDES_NAME(serdesID) (serdes_info[(serdesID)])
extern char *itfname_info[MAX_SERDES_NUM];
extern PhySerdes_t serdes_map[SERDES_MAX_IDX];
extern PhySerdes_t serdes_map_aeon[SERDES_MAX_IDX];

#define PHYSERDES_MAP_SIZE (sizeof(serdes_map) / sizeof(serdes_map[0]))
#define GET_SERDES_ID(idx) (serdes_map[(idx)].serdes_id)
#define GET_SERDES_PHYADDR(idx) (serdes_map[(idx)].phy_addr)

/*start of defines for structures type to manage phy*/
typedef struct _phy_priv_data{
    struct phy_device *phy_dev;
    uint8_t phy_addr;
    uint8_t serdes_id;
    uint32_t  phy_id;
    uint8_t phy_status;
}Phy_priv_data_t;

typedef struct _mdio_priv_data{
    struct mii_bus *mii_bus;
    Phy_priv_data_t phy_info[MAX_PHY_DEVICES];  /*save every phy device's detailed information*/
    uint8_t num_phy_register;                   /*number of registered phy devices*/
    uint8_t phy_addr_list[MAX_PHY_DEVICES];     /*save phy address of registered phy devices*/
    uint8_t phy_registers[MAX_PHY_DEVICES];     /*save serdes_id of registered phy devices, index is phy address*/
    uint8_t phy_addr_default;                   /*default value  for first phy address*/
}Mdio_priv_data_t; /*copy form dmio_arht.c*/

#define MDIO_BUS_PRT(priv)       ((priv)->mii_bus)
#define MDIO_PHY_NUM(priv)       ((priv)->num_phy_register)
#define MDIO_REG_ADDR(priv,idx)   ((priv)->phy_addr_list[(idx)]) /*save registered phy address*/
#define MDIO_REG_SDID(priv,idx)   ((priv)->phy_registers[(idx)]) /*save registered serdes id*/
#define MDIO_PHY_DEFAULT(priv)   ((priv)->phy_addr_default)
#define PHY_DEV_PTR(priv,idx)    ((priv)->phy_info[(idx)].phy_dev)
#define PHY_DEV_ADDR(priv,idx)   ((priv)->phy_info[(idx)].phy_addr)
#define PHY_SERDES_ID(priv,idx)  ((priv)->phy_info[(idx)].serdes_id)
#define PHY_DEV_ID(priv,idx)     ((priv)->phy_info[(idx)].phy_id)
#define PHY_STATUS(priv,idx)     ((priv)->phy_info[(idx)].phy_status)

#define SET_SERDES_ID_SYNC(priv,idx,id) \
    do {\
        ((priv)->phy_info[(idx)].serdes_id = (id));\
        ((priv)->phy_registers[(idx)] = (id));\
    }while(0)

#define GET_PHY_DEV(priv,phyAddr) PHY_DEV_PTR(priv,phyAddr)
#define GET_PHY_STATUS(priv,phyAddr) PHY_STATUS(priv,phyAddr)
#define GET_DEFAULT_PHYADDR(priv)      MDIO_PHY_DEFAULT(priv)

/*end of defines for structures type to manage phy*/

#define CHECK_BASE(ptr,msg) \
    (((ptr) != NULL) ? EN8811_TRUE : \
    (printk("EN8811@ [%s:%d] %s\n",__func__,__LINE__,msg), EN8811_FALSE))
#define CHECK_POINTER(ptr)      CHECK_BASE(ptr,"ptr is NULL")
#define CHECK_PHY_PRIV(ptr)     CHECK_BASE(ptr,"hsgmii_priv_p is NULL")
#define CHECK_PHY_DEV(ptr)      CHECK_BASE(ptr,"PHY device is NULL")

#define CHECK_PHY_ADDR(addr) \
    (((addr) >= MIN_PHY_ADDR_EN8811) && ((addr) <= MAX_PHY_ADDR_EN8811) ? EN8811_TRUE : \
    (printk("EN8811@ [%s:%d] PHY address[%d] is invalid\n",__func__,__LINE__,(addr)), EN8811_FALSE))

#define CHECK_SERDES_ID(id) \
    (((id) < SERDES_MAX_IDX) ? EN8811_TRUE : \
    (printk("EN8811@ [%s:%d] Serdes id[%d] is invalid\n",__func__,__LINE__,(id)), EN8811_FALSE))

void* mem_alloc(int size);
void mem_free(void* ptr);
uint8_t init_hsgmii_priv(Mdio_priv_data_t *mdio_priv_data_p);
void print_phy_registeredInfo(Mdio_priv_data_t *mdio_priv_data_p);
void print_phy_privdataInfo(Mdio_priv_data_t *mdio_priv_data_p);

uint8_t check_phy_itfname_valid(Mdio_priv_data_t *mdio_priv_data_p, char *itfname);
uint8_t get_phyAddr_by_ItfName(char *iftname);
struct phy_device* get_phyDev_by_ItfName(Mdio_priv_data_t *mdio_priv_data_p, char *iftname);
char* get_ItfName_by_phyAddr(Mdio_priv_data_t *mdio_priv_data_p, uint8_t phyAddr);
void get_default_ItfName(Mdio_priv_data_t *mdio_priv_data_p, char* phyItfName);
int airoha_hsgmii_init(void);
void airoha_hsgmii_exit(void);
uint8_t check_phy_itfname_valid(Mdio_priv_data_t *mdio_priv_data_p, char *itfname);
char* get_ItfName_by_phyAddr(Mdio_priv_data_t *mdio_priv_data_p, uint8_t phyAddr);
struct phy_device* get_phyDev_by_ItfName(Mdio_priv_data_t *mdio_priv_data_p, char *iftname);
void get_default_ItfName(Mdio_priv_data_t *mdio_priv_data_p, char* phyItfName);

extern struct phy_device **as21xx_dev_all;
#define MII_MMD_ACC_CTL_REG         0x0d
#define MII_MMD_ADDR_DATA_REG       0x0e
#define MMD_OP_MODE_DATA            BIT(14)

#define AEON_BOOT_ADDR             (0X2000 >> 1)
#define AEON_READ_SIZE             4096

#define AN8831_PHY_NUM             2
#define AN8831_DRIVER_VERSION_BOOT      "v1.8.4"

#define AN8831_PHY_ID1             0x7500
#define AN8831_PHY_ID2             0x9410
#define AN8831_PHY_ID              ((AN8831_PHY_ID1 << 16) | AN8831_PHY_ID2)
#define AN8831_PHY_ID_patch        ((AN8831_PHY_ID1 << 16) | AN8831_PHY_ID1)

#define AN8831_PHY_ID_0            ((AN8831_PHY_ID1 << 16) | 0x9402)
#define AN8831_PHY_ID_1            ((AN8831_PHY_ID1 << 16) | 0x9412)
#define AN8831_PHY_ID_2            ((AN8831_PHY_ID1 << 16) | 0x9422)
#define AN8831_PHY_ID_3            ((AN8831_PHY_ID1 << 16) | 0x9432)
#define AN8831_PHY_ID_4            ((AN8831_PHY_ID1 << 16) | 0x9442)
#define AN8831_PHY_ID_5            ((AN8831_PHY_ID1 << 16) | 0x9452)
#define AN8831_PHY_ID_6            ((AN8831_PHY_ID1 << 16) | 0x9462)
#define AN8831_PHY_ID_7            ((AN8831_PHY_ID1 << 16) | 0x9472)
#define AN8831_PHY_ID_8            ((AN8831_PHY_ID1 << 16) | 0x9482)
#define AN8831_PHY_ID_9            ((AN8831_PHY_ID1 << 16) | 0x9492)

#define LED_NUM                    5

#define LED_OFF_1                    0x0
#define LED_ON_NG_BLINK_ACT        0x1
#define LED_ON_FE_GE_BLINK_ACT     0x2
#define LED_ON_LINK_EST            0x3
#define LED_ON_LINK_EST_BLINK_ACT  0x5
#define LED_ON_NG_BLINK_FE_GE      0x6

struct led_custom_cfg {
  char led_event[LED_NUM];
  char blink_duration;
};

/* IPC registers */
#define IPC_NB_OPCODE         0x6
#define IPC_PAYLOAD_NB        0x5
#define IPC_NB_STATUS         0x4
#define IPC_CMD_PARITY        0x8000

#define IPC_TIMEOUT           2000000000 // 2 seconds

/// IPC status codes (4b)
#define IPC_STS_CMD_RCVD    0x1
#define IPC_STS_CMD_PROCESS 0x2
#define IPC_STS_CMD_SUCCESS 0x4
#define IPC_STS_CMD_ERROR   0x8
#define IPC_STS_SYS_BUSY    0xE
#define IPC_STS_SYS_READY   0xF

// Config parameters are included in IPC message, only 5 16bits left.
#define IPC_CMD_CFG_DIRECT        0x4
// Get parameter configs
#define IPC_CMD_CFG_PARAM_GET_VAR 0x5
// Get register configs
#define IPC_CMD_CFG_PARAM_GET_REG 0x6

/// IPC command opcodes
#define IPC_CMD_RMEM32         0x4   ///< read data from mem
#define IPC_CMD_BULK_READ      0xB   ///< Read bulk data from memory
#define IPC_CMD_NG_TESTMODE    0x1B  ///< Set NG test mode and tone
#define IPC_CMD_TEMP_MON       0x15  ///< Temperature monitoring function

#define AN8831_FW_LOAD_ADDR 0x81db5000
#define FLASH_CHIP_SIZE 0x00200000
#define IMAGE1_HDR_OFST 0x10000
#define IMAGE1_OFST 0x10400
#define IMAGE2_HDR_OFST 0x80000
#define IMAGE2_OFST 0x80400
#define MEM_WORD_SIZE 4
#define BOOT_LOADER_BIN "bootloader_all.bin"
#define FLASH_BIN "flash_burn.bin"
#define CLR_FLASH_IMAGE "clr_flash_image.bin"
#define ERASE_MODE_SECTOR 1
#define ERASE_MODE_BLOCK 2
#define FLASH_SECTOR_SIZE 4096

/* Regsiter addresses and fields */
/* Registers used in MDIO boot */
#define AEON_REG_ADDR_OFFSET 0xEA000000
#define AN_REG_GIGA_STD_STATUS_BASEADDR (AEON_REG_ADDR_OFFSET + 0xFFFC2)
#define IPC_CMD_BASEADDR (AEON_REG_ADDR_OFFSET + 0x3CB002)
#define IPC_STS_BASEADDR (AEON_REG_ADDR_OFFSET + 0x3CB004)
#define IPC_DATA0_BASEADDR (AEON_REG_ADDR_OFFSET + 0x3CB010)

/** @name Opcode
 * @brief Introduction of opcodes.
 * @{
 * 16-bit register IPC_CMD_BASEADDR is laid out as follows:
 * [1 bit parity][4 bits reserved][5 bits size][6 bits opcode].
 *
 * The following opcodes indicate the last 6 bits above.
 */
#define IPC_CMD_NOOP 0x0 ///< Do nothing.
#define IPC_CMD_INFO 0x1 ///< Get Firmware Version
#define IPC_CMD_SYS_CPU 0x2 ///< SYS_CPU
#define IPC_CMD_RMEM16 0x3 ///< Read date from MEM.
#define IPC_CMD_BULK_DATA 0xA ///< Pass bulk data in ipc registers.
#define IPC_CMD_BULK_WRITE 0xC ///< Write bulk data to memory
#define IPC_CMD_FLASH_WRITE 0xE ///< Write memory data to flash
#define IPC_CMD_FLASH_ERASE 0xF ///< Erase flash
#define IPC_CMD_LOG 0x13 ///< Read log
#define IPC_CMD_CFG_PARAM 0x1A ///< Write config parameters to memory
#define IPC_CMD_CFG_IRQ 0x22 ///< Cfg IRQ Output
#define IPC_CMD_SET_LED 0x23 ///< Set led
/** @} */
// IRQ Opcode
#define IPC_CMD_IRQ_EN 0x0
#define IPC_CMD_IRQ_QUERY 0x1
#define IPC_CMD_IRQ_CLR 0x2

/** @name Sub_command
 * @brief Introduction of sub_commands of opcodes
 * @{
 */
#define IPC_CMD_INFO_VERSION 0x1 ///< Sub-command of IPC_CMD_INFO.
#define IPC_CMD_SYS_REBOOT 0x3 ///< Sub-command of IPC_CMD_SYS_CPU, reboot phy.
#define IPC_CMD_SYS_IMAGE_OFST                                                 \
	0x4 ///< Sub-command of IPC_CMD_SYS_CPU, get flash image offset.
#define IPC_CMD_SYS_CPU_IMAGE_CHECK                                            \
	0x5 ///< Sub-command of IPC_CMD_SYS_CPU, CRC check after upgrading flash image.
#define IPC_CMD_SYS_CPU_PHY_ENABLE                                             \
	0x6 ///< Sub-command of IPC_CMD_SYS_CPU, enable/disable phy.
#define IPC_CMD_CFG_DIRECT                                                     \
	0x4 ///< Sub-command of IPC_CMD_CFG_PARAM, configure parameters of AN, DPC, phy_ctrl.
#define IPC_CMD_READ_LOG_CLEAR 0x3 ///< Sub-command of IPC_CMD_LOG.
/** @} */

/** @name Cfg_module
 * @brief Introduction of modules of sub_commands
 * @{
 */
enum custom_direct_cfg_module {
	CFG_NG_PHYCTRL = 1,
	CFG_CU_AN = 2,
	CFG_SDS_PCS = 3,
	CFG_AUTO_EEE = 4,
	CFG_SDS_PMA = 5,
	CFG_DPC_RA = 6,
	CFG_DPC_PKT_CHK = 7,
	CFG_DPC_SDS_WAIT_ETH = 8,
	CFG_WDT = 9,
	CFG_SDS_RESTART_AN = 10,
	CFG_TEMP_MON = 11,
	CFG_WOL = 12,
	CFG_SMI_COMMAND = 13,
	CFG_CABLE_DIAG = 14,
	CFG_EYE_DRAW = 15,
	CFG_MAC_CNT = 16,
};
/** @} */

/** @name Feature
 * @brief Introduction of features of modules
 * @{
 */
#define IPC_CMD_CFG_PHYCTRL_PAUSED 0x2 ///< Sub-command of CFG_NG_PHYCTRL.
#define IPC_CMD_CFG_TX_FULLSCALE 0x3 ///< Sub-command of CFG_NG_PHYCTRL.
#define IPC_CMD_CFG_NG_TESTMODE 0x4 ///< Sub-command of CFG_NG_PHYCTRL.
#define IPC_CMD_GET_TX_FULLSCALE 0x5 ///< Sub-command of CFG_NG_PHYCTRL.
#define MDI_CFG_MAN_MDI 0x0 ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_ENABLE 0x1 ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_DUPLEX 0x2 ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_EEE_SPD 0x3 ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_FR_SPD 0x4 ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_DS 0x6 ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_RESTART 0xa ///< Sub-command of CFG_CU_AN.
#define MDI_CFG_CU_AN_AEON_OUI 0xb ///< Sub-command of CFG_CU_AN.
#define IPC_CMD_CU_AN_TOP_SPD 0xc ///< Sub-command of CFG_CU_AN.
#define IPC_CMD_CU_AN_MS_CFG 0xd ///< Sub-command of CFG_CU_AN.
#define IPC_CMD_CU_AN_TRD_SWAP 0xe ///< Sub-command of CFG_CU_AN.
#define IPC_CMD_CU_AN_GET_MS_CFG 0xf ///< Sub-command of CFG_CU_AN.
#define IPC_CMD_CU_AN_CFR 0x10 ///< Sub-command of CFG_CU_AN.
#define IPC_CMD_CABLE_DIAG_CHAN_LEN 0x1 ///< Sub-command of CFG_CABLE_DIAG.
#define IPC_CMD_CABLE_DIAG_PPM_OFST 0x2 ///< Sub-command of CFG_CABLE_DIAG.
#define IPC_CMD_CABLE_DIAG_SNR_MARG 0x3 ///< Sub-command of CFG_CABLE_DIAG.
#define IPC_CMD_CABLE_DIAG_CHAN_SKW 0x4 ///< Sub-command of CFG_CABLE_DIAG.
#define IPC_CMD_EYE_SCAN_GET 0x0 ///< Sub-command of CFG_EYE_DRAW.
#define IPC_CMD_EYE_SCAN 0x1 ///< Sub-command of CFG_EYE_DRAW.
#define IPC_CMD_MAC_TOT 0x0 ///< Sub-command of CFG_MAC_CNT.
#define IPC_CMD_MAC_CRC 0x1 ///< Sub-command of CFG_MAC_CNT.
/** @} */

/** @name Status
 * @brief Introduction of IPC status
 * @{
 * 16-bit register IPC_STS_BASEADDR is laid out as follows:
 * [1 bit parity][5 bits size][6 bits opcode][4 bits status]
 *
 * The following status indicates the last 4 bits above.
 */
#define IPC_STS_CMD_RCVD 0x1
#define IPC_STS_CMD_PROCESS 0x2
#define IPC_STS_CMD_SUCCESS 0x4
#define IPC_STS_CMD_ERROR 0x8
#define IPC_STS_SYS_BUSY 0xE
#define IPC_STS_SYS_READY 0xF
/** @} */
/** @name IPC Function
 * @brief Introduction of IPC functions
 * @{
 * 
 1. **Opcode : IPC_CMD_NOOP**

 * Api : void aeon_ipc_sync_parity(struct phy_device *phydev);
 * 
 2. **Opcode : IPC_CMD_INFO**

 * Sub-command : IPC_CMD_INFO_VERSION

 * Api : void aeon_ipc_get_fw_version(char *version, struct phy_device *phydev);

 * param : version The string to get FW version.
 * 
 3. **Opcode : IPC_CMD_SYS_CPU**

 * 3.1 Sub-command : IPC_CMD_SYS_REBOOT

 * Api : void aeon_ipc_set_sys_reboot(struct phy_device *phydev);
 * 
 * 3.2 Sub-command : IPC_CMD_SYS_CPU_PHY_ENABLE
 * 
 * Api : void aeon_ipc_phy_enable_mode(unsigned short enable, struct phy_device *phydev);
 * 
 * param : enable Enable/disable PHY.
 * 
 * 3.3 Sub-command : IPC_CMD_SYS_CPU_IMAGE_CHECK, IPC_CMD_SYS_IMAGE_OFST
 * 
 * Api : int aeon_ipc_sys_cpu_info(unsigned short sub_cmd, unsigned int flash_addr, unsigned int mem_addr, struct phy_device *phydev)
 * 
 * param : sub_cmd IPC_CMD_SYS_CPU_IMAGE_CHECK / IPC_CMD_SYS_IMAGE_OFST
 * 
 * param : flash_addr Flash address to write.
 * 
 * param : mem_addr Memory address to write.
 * 
 4. **Opcode : IPC_CMD_BULK_DATA**

 * Api : void aeon_ipc_send_bulk_data(unsigned short bw_type, unsigned short size, void *data, struct phy_device *phydev);

 * param : bw_type Type of bit-width of data.

 * param : size Size of data.

 * param : data Data to write to IPC data registers.
 * 
 5. **Opcode : IPC_CMD_BULK_WRITE**

 * Api : void aeon_ipc_send_bulk_write(unsigned int mem_addr, unsigned int size, struct phy_device *phydev);

 * param : mem_addr Memory address to write.

 * param : size Size of data.
 * 
 6. **Opcode : IPC_CMD_FLASH_WRITE**

 * Api : void aeon_ipc_write_flash(unsigned int flash_addr, unsigned int mem_addr, unsigned short size, struct phy_device *phydev);
 
 * param : flash_addr Flash address to write.

 * param : mem_addr Memory address of the data we want to write to flash.

 * param : size Size of data.
 * 
 7. **Opcode : IPC_CMD_FLASH_ERASE**

 * Api : void aeon_ipc_erase_flash(unsigned int flash_addr, unsigned int size, unsigned short mode, struct phy_device *phydev);
 * 
 * param : flash_addr Flash address to erase.
 * 
 * param : size Size of data.
 * 
 * param : mode Mode of erasing. 1 : sector erase, 2 : block erase.
 * 
 8. **Opcode : IPC_CMD_CFG_PARAM**

 * Sub-command : IPC_CMD_CFG_DIRECT

 * 8.1 Cfg-module : CFG_NG_PHYCTRL

 * 8.1.1 Feature : IPC_CMD_CFG_PHYCTRL_PAUSED

 * Api : void aeon_ipc_set_fsm_mode(unsigned short fsm, unsigned short mode, struct phy_device *phydev);

 * param : fsm Macro of FSMs.

 * param : mode FSM running mode.

 * 8.1.2 Feature : IPC_CMD_CFG_TX_FULLSCALE
 * 
 * Api : void aeon_ipc_set_tx_fullscale_delta(unsigned short speed, unsigned short *delta, struct phy_device *phydev);
 * 
 * param : 4(100M), 8(1G), 16(2.5G), 32(5G), 64(10G).
 * 
 * param : Value to set.
 *
 * 8.1.3 Feature : IPC_CMD_CFG_NG_TESTMODE
 
 * Api : void aeon_ipc_ng_test_mode(unsigned short test_mode, unsigned short tone, struct phy_device *phydev);

 * param : test_mode Test mode to be set.

 * param : tone Test tone to be set.
 * 
 * 8.1.4 Feature : IPC_CMD_GET_TX_FULLSCALE
 
 * Api : void aeon_ipc_get_tx_fullscale_delta(unsigned short speed, unsigned short *delta, struct phy_device *phydev);

 * param : speed Speed mode.

 * param : delta Array to get tx_fullscale.

 * 8.2 Cfg-module : CFG_CU_AN
 * 
 * 8.2.1 Feature : MDI_CFG_CU_AN_EEE_SPD
 * 
 * Api : void aeon_cu_an_set_eee_spd(unsigned short speed_mode, struct phy_device *phydev);
 * 
 * param : speed_mode 7 bits [10G eee en] [5G eee en] [2.5G eee en] [1G eee en] [100m eee] [0] [0].
 * 
 * note : If you set EEE abilities of all speeds, speed_mode = 0b1111100(0x7C).
 * 
 * 8.2.2 Feature : MDI_CFG_CU_AN_FR_SPD
 * 
 * Api : void aeon_cu_an_set_fast_retrain(unsigned short speed_mode, unsigned short thp_bypass, struct phy_device *phydev);
 * 
 * param : speed_mode 7 bits [10G fr] [5G fr] [2.5G fr] [0] [0] [0] [0].
 * 
 * param : thp_bypass 2 bits [5G thp_bypass] [2.5G thp_bypass]
 * 
 * note : If you set FR abilities and thp_bypass of all speeds, speed_mode = 0b1110000(0x70), thp_bypass = 3.
 * 
 * 8.2.3 Feature : MDI_CFG_CU_AN_DS
 * 
 * Api : void aeon_cu_an_enable_downshift(unsigned short enable, unsigned short retry_limit, struct phy_device *phydev);
 * 
 * param : enable Enable/disable downshift.
 * 
 * param : retry_limit Limited failure times of training.
 * 
 * 8.2.4 Feature : MDI_CFG_CU_AN_RESTART
 * 
 * Api : void aeon_cu_an_restart(struct phy_device *phydev);
 * 
 * 8.2.5 Feature : MDI_CFG_CU_AN_AEON_OUI
 * 
 * Api : void aeon_cu_an_enable_aeon_oui(unsigned short nstd_pbo, struct phy_device *phydev);
 * 
 * param : nstd_pbo 2 bits [max_pbo] [min_pbo]
 * 
 * note : If you want to enable max_pbo and min_pbo, nstd_pbo = 3.
 * 
 * 8.2.6 Feature : IPC_CMD_CU_AN_TOP_SPD
 * 
 * Fucntion : void aeon_cu_an_set_top_spd(unsigned short top_spd, struct phy_device *phydev);
 * 
 * param : top_spd 7 bits [10G] [5G] [2.5G] [1G] [100m] [0] [0].
 * IPC_CMD_RMEM32signed short ms_man_en, unsigned short ms_man_val, struct phy_device *phydev);
 * 
 * param : port_type 0: single port, 1: multi port.
 * 
 * param : ms_man_en 0: disable manual m/s, 1: enable manual m/s.
 * 
 * param : ms_man_val 0: slave, 1: master
 * 
 * 8.2.8 Feature : IPC_CMD_CU_AN_TRD_SWAP
 * 
 * Api : void aeon_cu_an_set_trd_swap(unsigned short en, unsigned short trd_swap, struct phy_device *phydev);
 * 
 * param : trd_swap 1 : enable TRD swap, 0 : disable TRD swap.
 * 
 * 8.2.9 Feature : IPC_CMD_CU_AN_GET_MS_CFG
 * 
 * Api : void aeon_cu_an_get_ms_cfg(unsigned short *ms_related_cfg, struct phy_device *phydev);
 * 
 * param : ms_related_cfg Array to put ms_cfg, ms_related_cfg[0] indicate port_type, ms_related_cfg[1] indicates ms_man_en, ms_related_cfg[2] indicates ms_man_val.
 * 
 * 8.2.10 Feature : IPC_CMD_CU_AN_CFR
 * 
 * Api : void aeon_cu_an_set_cfr(unsigned short cfr, struct phy_device *phydev);
 * 
 * param : cfr 1 : enable CFR, 0 : disable CFR.
 * 
 * 8.3 Cfg-module : CFG_SDS_PCS
 * 
 * Api : void aeon_sds_pcs_set_cfg(unsigned short pcs_mode, unsigned short sds_spd, struct phy_device *phydev);
 * 
 * param : pcs_mode 1 : 64/66B, 0 : 8B/10B.
 * 
 * param : sds_spd 3 : 10G, 2 : 5G, 1 : 2.5G, 0 : 1G.
 * 
 * 8.4 Cfg-module : CFG_AUTO_EEE
 * 
 * Api : void aeon_auto_eee_cfg(unsigned short enable, unsigned int idle_th, struct phy_device *phydev);
 * 
 * param : enable 1 : enable auto-eee, 0 : disable auto-eee.
 * 
 * param : idle_th idle threshhold
 * 
 * 8.5 Cfg-module : CFG_SDS_PMA
 * 
 * Api : void aeon_sds_pma_set_cfg(unsigned short vga_adapt, unsigned short slc_adapt, unsigned short ctle_adapt, unsigned short dfe_adapt, struct phy_device *phydev);

 * param : vga_adapt 1 : enable VGA adaptation, 0 : disable VGA adaptation.

 * param : slc_adapt 1 : enable slicer adaptation, 0 : disable slicer adaptation.

 * param : ctle_adapt 1 : enable CTLE adaptation, 0 : disable CTLE adaptation.

 * param : dfe_adapt 1 : enable DFE adaptation, 0 : disable DFE adaptation.
 * 
 * 8.6 Cfg-module : CFG_DPC_RA
 * 
 * Api : void aeon_dpc_ra_enable(struct phy_device *phydev);
 * 
 * 8.7 Cfg-module : CFG_DPC_PKT_CHK
 * 
 * Api : void aeon_pkt_chk_cfg(unsigned short enable, struct phy_device *phydev);
 * 
 * param : enable Enable/disable packet checker.
 * 
 * 8.8 Cfg-module : CFG_DPC_SDS_WAIT_ETH
 * 
 * Api : void aeon_sds_wait_eth_cfg(unsigned short sds_wait_eth_delay, struct phy_device *phydev);
 * 
 * param : sds_wait_eth_delay Delay.
 * 
 * 8.9 Cfg-module : CFG_WDT
 * 
 * Api : void aeon_ipc_set_wdt(unsigned short en, struct phy_device *phydev);
 * 
 * param : en Enable/disable WDT.
 * 
 * 8.10 Cfg-module : CFG_SDS_RESTART_AN
 * 
 * Api : void aeon_sds_restart_an(struct phy_device *phydev);
 * 
 * 8.11 Cfg-module : IPC_CMD_TEMP_MON
 * 
 * Api : void aeon_ipc_temp_monitor(unsigned short sub_cmd, unsigned short params, unsigned short *temperature, struct phy_device *phydev);
 * 
 * param : sub_cmd 1 : start, 2 : stop, 3 : set configuration, 4 : get temperature, 5 : set threshhold
 * 
 * param : params For sub_cmd = 3, 1 indicates continuous sampling, 0 indicates a single sample.
 * 
 * For sub_cmd = 5, params indicates the temperature threshold to be set.
 * 
 * param : temperature To save current temperature got from ipc command.
 * 
 * 8.12 Cfg-module : CFG_WOL
 * 
 * Api : void aeon_ipc_set_wol(unsigned short en, unsigned short *val, struct phy_device *phydev);
 * 
 * param : en 0 : mac addr and passwd return default value, 1 : cfg mac addr, 2 : cfg passwd, 3 : cfg mac addr.
 * 
 * param : val mac addr and passwd for wol.
 * 
 * 8.13 Cfg-module : CFG_SMI_COMMAND
 * 
 * Api : void aeon_ipc_smi_command(unsigned short *val, struct phy_device *phydev);
 * 
 * param : val SMI Command Value.
 * 
 * 8.14 Cfg-module : CFG_CABLE_DIAG
 * 
 * Api : void aeon_ipc_cable_diag(unsigned short sub_cmd, unsigned short *data, struct phy_device *phydev);
 * 
 * param : sub_cmd 1 : get channel length, 2 : get ppm offset, 3 : get snr margin, 4 : get channel skew.
 * 
 * param : data To save the paragrams we want to get.
 * 
 * 8.15 Cfg-module : CFG_EYE_DRAW
 * 
 * Api : void aeon_ipc_eye_scan(unsigned short subcmd, unsigned short *data_rcv, unsigned short sds_id, unsigned short eye_num, struct phy_device *phydev); 
 *
 * param : sub_cmd 0 : get address and sample count about eye scam, 1 : eye scan.
 * 
 * param : data_rcv To save the paragrams we want to get.
 * 
 9. **Opcode : IPC_CMD_SET_LED**
 
 * Api : void aeon_ipc_set_led_cfg(unsigned short led0, unsigned short led1, unsigned short led2, unsigned short led3, unsigned short led4, unsigned short polarity, unsigned short blink, struct phy_device *phydev);

 * param : led0 Behavior of LED0.

 * param : led1 Behavior of LED1.

 * param : led2 Behavior of LED2.

 * param : led3 Behavior of LED3.
 * 
 * param : led4 Behavior of LED4.
 * 
 * param : led5 Behavior of LED5.
 * 
 * param : polarity cfg of these leds.
 * 
 * param : blink Blink rate of these leds.
 *  
 10. **Opcode : IPC_CMD_CFG_IRQ**
 *
 * 10.1 Cfg-module : IPC_CMD_IRQ_EN
 * 
 * Api : aeon_ipc_irq_en(unsigned short *irq, struct phy_device *phydev);
 *
 * param : val[0] enable irq index, val[0] enable irq value.
 * 
 * 10.2 Cfg-module : IPC_CMD_IRQ_QUERY
 * 
 * Api : aeon_ipc_irq_query(unsigned short *irq, struct phy_device *phydev);
 *
 * param : irq trigger .
 * 
 * 10.3 Cfg-module : IPC_CMD_IRQ_CLR
 * 
 * Api : void aeon_ipc_irq_clr(unsigned short val, struct phy_device *phydev);
 *
 * param : val clear irq index.
 * 
 */
#define IPC_PAYLOAD_SIZE 16
#define IPC_NB_OPCODE 0x6
#define IPC_PAYLOAD_NB 0x5
#define IPC_NB_STATUS 0x4
#define IPC_CMD_PARITY 0x8000
#define IPC_TIMEOUT 2000000000
/// The MSB of the status bit is used as a parity bit, toggling each time an
/// IPC command is serviced.
#define IPC_STS_PAR_MASK 0x8000
/// The data layout for the sub-fields in the IPC status register
#define IPC_STS_STS_WIDTH 4
/** @} */

#define PCS_SPEED_SEL_T10G 0x0
#define PCS_SPEED_SEL_T5G 0x8
#define PCS_SPEED_SEL_T2P5G 0x7

#define MDI_CFG_SPD_T10 0x2
#define MDI_CFG_SPD_T100 0x4
#define MDI_CFG_SPD_T1G 0x8
#define MDI_CFG_SPD_T2P5G 0x10
#define MDI_CFG_SPD_T5G 0x20
#define MDI_CFG_SPD_T10G 0x40

#define PCS_MODE_8B_10B 0
#define PCS_MODE_64B_66B 1

#define SDS_DATARATE_1G 0
#define SDS_DATARATE_2P5G 1
#define SDS_DATARATE_5G 2
#define SDS_DATARATE_10G 3

#define SDS_RA_XFI 0
#define SDS_RA_USXGMII 1

#define CFG_FSM_NGPHY 0x81
#define CFG_FSM_GPHY 0x8c
#define CFG_FSM_CU_AN 0x8d
#define CFG_FSM_DPC 0x8e
#define CFG_FSM_SS 0x91

#define MDI 1
#define MDIX 0

enum bitwidth_type {
	BW8 = 0,
	BW16 = 1,
	BW32 = 2,
};

/** @name AEON's private function related to IPC
 * @note These functions shouldn't be called individually
 * @{
*/
/**
 * @brief Get data from to IPC_DATA registers.
 * @param len Data size.
 * @param data Data array to be assigned.
 */
void aeon_receive_ipc_data(struct phy_device *phydev, unsigned short len,
			   unsigned short *data);

/**
 * @brief Build IPC cmd.
 * @note Construct the full command word.
 * 16-bit register is laid out as follows:
 * [1 cmd par][4 reserved][5 size][6 opcode]
 */
void aeon_ipc_build_cmd(unsigned short *cmd, short opcode, short size);

/**
 * @brief Wait until IPC status handshake returns DONE or READY.
 * @return Current IPC status.
 */
unsigned short aeon_ipc_wait_cmd_done(struct phy_device *phydev,
				      unsigned long *ns,
				      unsigned short *ret_size);

/**
 * @brief Write bulk data to some memory.
 * @param mem_addr The memory address to write data.
 * @param size Data size.
 */
void aeon_ipc_send_bulk_write(unsigned int mem_addr, unsigned int size,
			      struct phy_device *phydev);

/**
 * @brief Send bulk data.
 * @param bw_type Type of bit-width.
 * @param size Data size.
 */
void aeon_ipc_send_bulk_data(unsigned short bw_type, unsigned short size,
			     void *data, struct phy_device *phydev);

/**
 * @brief Send IPC commands.
 */
void aeon_send_ipc_msg(struct phy_device *phydev, unsigned int len,
		       unsigned short *val, short opcode, short size);

/** @} */

/** @name AEON's public function related to IPC
 * @note These functions could be called individually
 * @{
*/
/** 3 : get snr margin, 4 : get channel skew.
 * @brief Send IPC command to sync parity.
 */
void aeon_ipc_sync_parity(struct phy_device *phydev);

/**
 * @brief Send IPC command to get FW version.
 */
void aeon_ipc_get_fw_version(char *version, struct phy_device *phydev);

/**
 * @brief Set eee abilities.
 * @param speed_mode 7 bits [10G eee] [5G eee] [2.5G eee] [1G eee] [100m eee] [0] [0].
 */
void aeon_cu_an_set_eee_spd(unsigned short speed_mode,
			    struct phy_device *phydev);

/**
 * @brief Set fast retrain abilities.
 * @param speed_mode 7 bits [10G fr] [5G fr] [2.5G fr] [0] [0] [0] [0].
 * @param thp_bypass 2 bits [5G thp_bypass] [2.5G thp_bypass]
 */
void aeon_cu_an_set_fast_retrain(unsigned short speed_mode,
				 unsigned short thp_bypass,
				 struct phy_device *phydev);

/**
 * @brief Configure donwshift.
 * @param enable Enable/disable downshift.
 * @param retry_limit Limited failure times of training.
 */
void aeon_cu_an_enable_downshift(unsigned short enable,
				 unsigned short retry_limit,
				 struct phy_device *phydev);

/**
 * @brief Send command to restart AN.
 */
void aeon_cu_an_restart(struct phy_device *phydev);

/**
 * @brief Configure PCS mode and speed of serdes.
 * @param pcs_mode 1 : 64/66B, 0 : 8B/10B.
 * @param sds_spd 3 : 10G, 2 : 5G, 1 : 2.5G, 0 : 1G.
 */
void aeon_sds_pcs_set_cfg(unsigned short pcs_mode, unsigned short sds_spd,
			  struct phy_device *phydev);

/**
 * @brief Set non-standard PBO mode.
 * @param nstd_pbo 2 bits [max_pbo] [min_pbo]
 */
void aeon_cu_an_enable_aeon_oui(unsigned short nstd_pbo,
				struct phy_device *phydev);

/**
 * @brief Set auto-eee configuration.
 * @param enable 1 : enable auto-eee, 0 : disable auto-eee.
 * @param idle_th idle threshhold
 */
void aeon_auto_eee_cfg(unsigned short enable, unsigned int idle_th,
		       struct phy_device *phydev);

/**
 * @brief Set ipc command to temperature monitor.
 * @param sub_cmd 1 : start, 2 : stop, 3 : set configuration,
 *  4 : get temperature, 5 : set threshhold
 * @param params For sub_cmd = 3, 1 indicates continuous sampling, 0 indicates a single sample.
 * For sub_cmd = 5, params indicates the temperature threshold to be set.
 * @param temperature To save current temperature got from ipc command.
 */
void aeon_ipc_temp_monitor(unsigned short sub_cmd, unsigned short params,
			   unsigned short *temperature,
			   struct phy_device *phydev);

/**
 * @brief Set serdes adaptations.
 * @param vga_adapt 1 : enable VGA adaptation, 0 : disable VGA adaptation.
 * @param slc_adapt 1 : enable slicer adaptation, 0 : disable slicer adaptation.
 * @param ctle_adapt 1 : enable CTLE adaptation, 0 : disable CTLE adaptation.
 * @param dfe_adapt 1 : enable DFE adaptation, 0 : disable DFE adaptation.
 */
void aeon_sds_pma_set_cfg(unsigned short vga_adapt, unsigned short slc_adapt,
			  unsigned short ctle_adapt, unsigned short dfe_adapt,
			  struct phy_device *phydev);

/**
 * @brief Set USXGMII mode.
 */
void aeon_dpc_ra_enable(struct phy_device *phydev);

/**
 * @brief Set led configuration.
 */
void aeon_ipc_set_led_cfg(unsigned short led0, unsigned short led1,
			  unsigned short led2, unsigned short led3,
			  unsigned short led4, unsigned short polarity,
			  unsigned short blink, struct phy_device *phydev);

/**
 * @brief Send command to set top_speed.
 * @param top_spd 7 bits [10G] [5G] [2.5G] [1G] [100m] [0] [0].
 */
void aeon_cu_an_set_top_spd(unsigned short top_spd, struct phy_device *phydev);

/**
 * @brief Send command to set trd_swap mode.
 * @param en 1 : override, 0 : no overriding.
 * @param trd_swap 1 : enable TRD swap, 0 : disable TRD swap.
 */
void aeon_cu_an_set_trd_swap(unsigned short en, unsigned short trd_swap,
			     struct phy_device *phydev);

/**
 * @brief Send command to set manual master/slave.
 * @param port_type 0: single port, 1: multi port.
 * @param ms_man_en 0: disable manual m/s, 1: enable manual m/s.
 * @param ms_man_val 0: slave, 1: master
 */
void aeon_cu_an_set_ms_cfg(unsigned short port_type, unsigned short ms_man_en,
			   unsigned short ms_man_val,
			   struct phy_device *phydev);

/**
 * @brief Send command to get m/s-related configuration.
 */
void aeon_cu_an_get_ms_cfg(unsigned short *ms_related_cfg,
			   struct phy_device *phydev);

/**
 * @brief Send command to reboot phy.
 */
void aeon_ipc_set_sys_reboot(struct phy_device *phydev);

/**
 * @brief Send command to enable/disable packet checker.
 * @param enable Enable/disable packet checker.
 */
void aeon_pkt_chk_cfg(unsigned short enable, struct phy_device *phydev);

/**
 * @brief Set sds_wait_eth configuration.
 * @param sds_wait_eth_delay Delay.
 */
void aeon_sds_wait_eth_cfg(unsigned short sds_wait_eth_delay,
			   struct phy_device *phydev);

/**
 * @brief Enable/disable phy.
 */
void aeon_ipc_phy_enable_mode(unsigned short enable, struct phy_device *phydev);

/**
 * @brief Write data from memory to flash.
 * @param flash_addr Flash address to write to.
 * @param mem_addr Memory address of data to write.
 * @param size Size of data.
 */
void aeon_ipc_write_flash(unsigned int flash_addr, unsigned int mem_addr,
			  unsigned short size, struct phy_device *phydev);

/**
 * @brief Erase flash.
 * @param flash_addr Flash address to erase.
 * @param size Size of data.
 * @param mode Mode of erasing. 1 : sector erase, 2 : block erase.
 */
void aeon_ipc_erase_flash(unsigned int flash_addr, unsigned int size,
			  unsigned short mode, struct phy_device *phydev);

/**
 * @brief Set WDT.
 * @param en Enable/disable WDT.
 */
void aeon_ipc_set_wdt(unsigned short en, struct phy_device *phydev);

/**
 * @brief Update image.
 * @param include_bootloader 1/0.
 * @note This a demo for dual flash programming feature.
 */
void aeon_burn_image(unsigned char include_bootloader,
		     struct phy_device *phydev);

/**
 * @brief Send short config parameter list to firmware directly.
 */
void aeon_ipc_cfg_param_direct(unsigned int data_len, unsigned short *data,
			       struct phy_device *phydev);

/**
 * @brief Set IPC commands related to CPU INFO.
 */
int aeon_ipc_sys_cpu_info(unsigned short sub_cmd, unsigned int flash_addr,
			  unsigned int mem_addr, struct phy_device *phydev);

/**
 * @brief Set FW FSM running mode.
 */
void aeon_ipc_set_fsm_mode(unsigned short fsm, unsigned short mode,
			   struct phy_device *phydev);

/**
 * @brief Update flash image.
 */
void aeon_update_flash(const char *firmware, unsigned int flash_start,
		       struct phy_device *phydev);

/**
 * @brief Set ipc command for restarting serdes AN.
 */
void aeon_sds_restart_an(struct phy_device *phydev);

/**
 * @brief Set ipc command for NG test mode.
 */
void aeon_ipc_ng_test_mode(unsigned short test_mode, unsigned short tone, struct phy_device *phydev);

/**
 * @brief Enable NG test mode for specific speed mode.
 */
void aeon_ng_test_mode(unsigned short top_spd, unsigned short test_mode, unsigned short tone, struct phy_device *phydev);

/**
 * @brief Enable 1G test mode.
 */
void aeon_1g_test_mode(unsigned short test_mode, struct phy_device *phydev);

/**
 * @brief Enable 100M test mode.
 */
void aeon_100m_test_mode(struct phy_device *phydev);

/**
 * @brief Set WOL.
 * @param en Enable/disable WOL.
 */
void aeon_ipc_set_wol(unsigned short en, unsigned short *val,
	struct phy_device *phydev);

/**
* @brief Set SMI Command.
* @param val command value.
*/
void aeon_ipc_smi_command(unsigned short *val, struct phy_device *phydev);

/**
* @brief Set irq enable.
* @param val irq index and en.
*/
void aeon_ipc_irq_en(unsigned short *val, struct phy_device *phydev);

/**
* @brief Set irq clr.
* @param val irq index.
*/
void aeon_ipc_irq_clr(unsigned short val, struct phy_device *phydev);

/**
* @brief get irq status.
* @param val irq status.
*/
void aeon_ipc_irq_query(unsigned short *irq, struct phy_device *phydev);

/**
* @brief Set IPC commands related to cable diag lite.
*/
void aeon_ipc_cable_diag(unsigned short sub_cmd, unsigned short *data,
   struct phy_device *phydev);

/**
* @brief Configure CISCO fast-retrain.
*/
void aeon_cu_an_set_cfr(unsigned short cfr, struct phy_device *phydev);

/**
 * @brief Set tx fullscale.
 */
void aeon_ipc_set_tx_fullscale_delta(unsigned short speed, unsigned short *delta, struct phy_device *phydev);

/**
 * @brief Get tx_full scale.
 */
void aeon_ipc_get_tx_fullscale_delta(unsigned short speed, unsigned short *delta, struct phy_device *phydev);

/**
 * @brief Get data from memory.
 */
void aeon_ipc_read_mem(unsigned short addr1, unsigned short addr2, unsigned short num,
			unsigned short *params, struct phy_device *phydev);

/**
 * @brief Serdes eye scan.
 */
void aeon_ipc_eye_scan(unsigned short subcmd, unsigned short *data_rcv,
			unsigned short sds_id, unsigned short eye_num,
			struct phy_device *phydev);

/**
 * @brief Clear log.
 */
void aeon_ipc_clear_log(struct phy_device *phydev);

/**
 * @brief Set mac count.
 */
void aeon_ipc_set_mac_cnt(unsigned long long mac_tot_cnt, unsigned long long mac_crc_cnt,
			struct phy_device *phydev);
/** @} */

/**
 * @brief Choose phy device.
 * @param phy_addr Phy address.
 */
void aeon_match_device(unsigned short phy_addr, struct phy_device **phydev);

/**
 * @brief Get phy link status.
 */
int aeon_read_status(struct phy_device *phydev);

/**
 * @brief Register configuration for test mode.
 */
void aeon_man_configure(struct phy_device *phydev);

/* previous prototype for arht_hsgmii.c */
int airphy_set_mode(struct phy_device *phydev, AIR_PORT_MODE_T dbg_mode);
int en8811_proc_init(void);
int en8811_proc_exit(void);
unsigned short aeon_mdio_read_reg(struct phy_device *phydev, unsigned int reg_addr);
void aeon_mdio_write_reg(struct phy_device *phydev, unsigned int reg_addr, unsigned short value);
unsigned short aeon_mdio_read_reg_field(struct phy_device *phydev, unsigned int reg_addr, unsigned short field);
void aeon_mdio_write_reg_field(struct phy_device *phydev, unsigned int reg_addr, unsigned short field, unsigned short value);
void aeon_send_ipc_cmd(struct phy_device *phydev, unsigned short cmd);
void aeon_set_ipc_data_reg(struct phy_device *phydev, unsigned int len, unsigned short *val);
unsigned short aeon_get_ipc_status(struct phy_device *phydev);
void aeon_ipc_parse_sts(unsigned short sts, unsigned short *status, unsigned short* opcode, unsigned short* size, unsigned short *parity);
unsigned int get_par(void);
void aeon_cu_an_enable(unsigned short enable, struct phy_device *phydev);
void aeon_set_man_mdi(struct phy_device *phydev);
void aeon_set_man_duplex(unsigned short duplex, struct phy_device *phydev);
int parse_cmd_args(const char* input, struct parsed_cmd* result, int max_args);
int an8831_phy_is_empty(void);
int an8831_proc_init(void);
int an8831_proc_exit(void);

typedef enum {
	XSI_PCIE0_IDX = 0,
	XSI_PCIE1_IDX = 1,
	XSI_USB_IDX   = 2, 
	XSI_AE_IDX    = 3,
	XSI_ETH_IDX   = 4,
	XSI_IDX_MAX,
}xsi_sel_type;

#define SUPPORT_TX_TOTAL_RATELIMIT (glb_eth && airoha_is_7583(glb_eth))

/* AN7583 */
#define XSI_AE_BASE_7583		0xbfa08000
#define	XSI_PCIE0_BASE_7583	0xbfa04000
#define	XSI_PCIE1_BASE_7583	0xbfa04000
#define	XSI_USB_BASE_7583	0xbfa05000
#define	XSI_ETH_BASE_7583	0xbfa09000

#define RX_RC_CFG_7583 (0xA8)
#define RX_CFG_7583 (0xAC)
#define RC_RD_DATA_L_7583 (0xB0)
#define RC_RD_DATA_H_7583 (0xB4)
#define RC_WR_DATA_L_7583 (0xB8)
#define RC_WR_DATA_H_7583 (0xBC)
#define TOTAL_RX_RC_CFG_7583 (0xA4)
#define TOTAL_RX_CFG_7583 RX_CFG_7583
#define TOTAL_RC_RD_DATA_L_7583 RC_RD_DATA_L_7583
#define TOTAL_RC_RD_DATA_H_7583 RC_RD_DATA_H_7583
#define TOTAL_RC_WR_DATA_L_7583 RC_WR_DATA_L_7583
#define TOTAL_RC_WR_DATA_H_7583 RC_WR_DATA_H_7583
#define TOTAL_TX_RC_CFG_7583 (0xA0)
#define TOTAL_TX_CFG_7583 RX_CFG_7583

/* EN7581 */
#define XSI_AE_BASE_7581		0x1fa08000
#define	XSI_PCIE0_BASE_7581	0x1fa04000
#define	XSI_PCIE1_BASE_7581	0x1fa05000
#define	XSI_USB_BASE_7581	0x1fa07000
#define	XSI_ETH_BASE_7581	0x1fa09000

#define RX_RC_CFG_7581 (0xA0)
#define RX_CFG_7581 (0xA4)
#define RC_RD_DATA_L_7581 (0xA8)
#define RC_RD_DATA_H_7581 (0xAC)
#define RC_WR_DATA_L_7581 (0xA8)
#define RC_WR_DATA_H_7581 (0xAC)
#define TOTAL_RX_RC_CFG_7581 (0xB0)
#define TOTAL_RX_CFG_7581 (0xB4)
#define TOTAL_RC_RD_DATA_L_7581 (0xB8)
#define TOTAL_RC_RD_DATA_H_7581 (0xBC)
#define TOTAL_RC_WR_DATA_L_7581 (0xB8)
#define TOTAL_RC_WR_DATA_H_7581 (0xBC)
#define TOTAL_TX_RC_CFG_7581 RX_RC_CFG_7581
#define TOTAL_TX_CFG_7581 RX_CFG_7581

#define XSI_AE_BASE		(SUPPORT_TX_TOTAL_RATELIMIT ? XSI_AE_BASE_7583 : XSI_AE_BASE_7581)
#define	XSI_PCIE0_BASE	(SUPPORT_TX_TOTAL_RATELIMIT ? XSI_PCIE0_BASE_7583 : XSI_PCIE0_BASE_7581)
#define	XSI_PCIE1_BASE	(SUPPORT_TX_TOTAL_RATELIMIT ? XSI_PCIE1_BASE_7583 : XSI_PCIE1_BASE_7581)
#define	XSI_USB_BASE	(SUPPORT_TX_TOTAL_RATELIMIT ? XSI_USB_BASE_7583 : XSI_USB_BASE_7581)
#define	XSI_ETH_BASE	(SUPPORT_TX_TOTAL_RATELIMIT ? XSI_ETH_BASE_7583 : XSI_ETH_BASE_7581)

#define RX_RC_CFG          (SUPPORT_TX_TOTAL_RATELIMIT ? RX_RC_CFG_7583 : RX_RC_CFG_7581)
#define RX_CFG             (SUPPORT_TX_TOTAL_RATELIMIT ? RX_CFG_7583 : RX_CFG_7581)
#define RC_RD_DATA_L       (SUPPORT_TX_TOTAL_RATELIMIT ? RC_RD_DATA_L_7583 : RC_RD_DATA_L_7581)
#define RC_RD_DATA_H       (SUPPORT_TX_TOTAL_RATELIMIT ? RC_RD_DATA_H_7583 : RC_RD_DATA_H_7581)
#define RC_WR_DATA_L       (SUPPORT_TX_TOTAL_RATELIMIT ? RC_WR_DATA_L_7583 : RC_WR_DATA_L_7581)
#define RC_WR_DATA_H       (SUPPORT_TX_TOTAL_RATELIMIT ? RC_WR_DATA_H_7583 : RC_WR_DATA_H_7581)
#define TOTAL_RX_RC_CFG    (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_RX_RC_CFG_7583 : TOTAL_RX_RC_CFG_7581)
#define TOTAL_RX_CFG       (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_RX_CFG_7583 : TOTAL_RX_CFG_7581)
#define TOTAL_RC_RD_DATA_L (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_RC_RD_DATA_L_7583 : TOTAL_RC_RD_DATA_L_7581)
#define TOTAL_RC_RD_DATA_H (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_RC_RD_DATA_H_7583 : TOTAL_RC_RD_DATA_H_7581)
#define TOTAL_RC_WR_DATA_L (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_RC_WR_DATA_L_7583 : TOTAL_RC_WR_DATA_L_7581)
#define TOTAL_RC_WR_DATA_H (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_RC_WR_DATA_H_7583 : TOTAL_RC_WR_DATA_H_7581)
#define TOTAL_TX_RC_CFG    (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_TX_RC_CFG_7583 : TOTAL_TX_RC_CFG_7581)
#define TOTAL_TX_CFG       (SUPPORT_TX_TOTAL_RATELIMIT ? TOTAL_TX_CFG_7583 : TOTAL_TX_CFG_7581)

#define XSI_IF_STS (0xCC)
#define XSI_IF_STS_F_TXMBI_STOP_STS (1<<0)
#define XSI_IF_STS_F_TXMPI_STOP_STS (1<<1)
#define XSI_IF_STS_F_RXMBI_STOP_STS (1<<2)
#define XSI_IF_STS_F_RXMPI_STOP_STS (1<<3)
#define XSI_IF_STS_F_TXMPI_MASK_STS (1<<4)

#define XSI_GLB_CFG (0x0)
#define XSI_GLB_CFG_F_TXMBI_STOP (1<<0)
#define XSI_GLB_CFG_F_TXMPI_STOP (1<<1)
#define XSI_GLB_CFG_F_RXMBI_STOP (1<<2)
#define XSI_GLB_CFG_F_RXMPI_STOP (1<<3)


#define regRead32(reg)				get_frame_engine_data(reg)
#define regWrite32(reg, wdata)		set_frame_engine_data(reg, wdata)

#define IO_GREG(reg)							regRead32(reg)
#define IO_SREG(reg, value)						regWrite32(reg, value)
#define IO_GMASK(reg, mask, shift)				((regRead32(reg) & mask) >> shift)
#define IO_SMASK(reg, mask, shift, value)		{ u32 t = regRead32(reg); regWrite32(reg, ((t&~(mask))|((value<<shift)&mask))); }
#define IO_SBITS(reg, bit)						{ u32 t = regRead32(reg); regWrite32(reg, (t|bit)); }
#define IO_CBITS(reg, bit)						{ u32 t = regRead32(reg); regWrite32(reg, (t&~(bit))); }

#define xsiGetTxmbiStopSts(index)       (IO_GREG(HSGMII_BASE_REG[index]+XSI_IF_STS) & XSI_IF_STS_F_TXMBI_STOP_STS)
#define xsiSetTxmbiDisable(index)       IO_SBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_TXMBI_STOP)
#define xsiSetTxmbiEnable(index)        IO_CBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_TXMBI_STOP)

#define xsiGetTxmpiStopSts(index)       (IO_GREG(HSGMII_BASE_REG[index]+XSI_IF_STS) & XSI_IF_STS_F_TXMPI_STOP_STS)
#define xsiSetTxmpiDisable(index)       IO_SBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_TXMPI_STOP)
#define xsiSetTxmpiEnable(index)        IO_CBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_TXMPI_STOP)

#define xsiGetRxmbiStopSts(index)       (IO_GREG(HSGMII_BASE_REG[index]+XSI_IF_STS) & XSI_IF_STS_F_RXMBI_STOP_STS)
#define xsiSetRxmbiDisable(index)       IO_SBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_RXMBI_STOP)
#define xsiSetRxmbiEnable(index)        IO_CBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_RXMBI_STOP)

#define xsiGetRxmpiStopSts(index)       (IO_GREG(HSGMII_BASE_REG[index]+XSI_IF_STS) & XSI_IF_STS_F_RXMPI_STOP_STS)
#define xsiSetRxmpiDisable(index)       IO_SBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_RXMPI_STOP)
#define xsiSetRxmpiEnable(index)        IO_CBITS(HSGMII_BASE_REG[index]+XSI_GLB_CFG,XSI_GLB_CFG_F_RXMPI_STOP)

#define XSI_MAC_LOGIC_RST (0x10)
#define XSI_MAC_LOGIC_RST_F_XSI_MAC_LOGIC_RST (1<<0)
#define xsimaclogicrst(index)			IO_SREG(HSGMII_BASE_REG[index]+XSI_MAC_LOGIC_RST,0)	
#define xsimaclogicrstenable(index)		IO_SREG(HSGMII_BASE_REG[index]+XSI_MAC_LOGIC_RST,1)	

#define XSI_CNT_CLR (0x100)
#define XSI_CNT_CLR_F_GLB_CNTCLR (1<<0)
#define xsiClearAllCnt(index)           IO_SBITS(HSGMII_BASE_REG[index]+XSI_CNT_CLR,XSI_CNT_CLR_F_GLB_CNTCLR)

#define HSGMII_RC_CFG_METER_DISABLE	(0x0)
#define HSGMII_RC_CFG_METER_ENABLE	(0x1)
#define HSGMII_RC_CFG_BYTE_MODE		(0x0)
#define HSGMII_RC_CFG_PKT_MODE		(0x1)
#define HSGMII_RC_CFG_FAST_TICK		(0x0)
#define HSGMII_RC_CFG_SLOW_TICK		(0x1)

#define HSGMII_RC_CFG_PARA_MISC		(0x0)
#define HSGMII_RC_CFG_PARA_TOKEN_RATE	(0x1)
#define HSGMII_RC_CFG_PARA_BUCK_SHIFT	(0x2)
#define HSGMII_RC_CFG_PARA_BUCK_CNT	(0x3)
#define HSGMII_RC_CFG_EN				(1<<31)
#define HSGMII_RC_CFG_PARA_TYPE_SHIFT	(28)
#define HSGMII_RC_CFG_PARA_TYPE_MASK	(0x3<<HSGMII_RC_CFG_PARA_TYPE_SHIFT)
#define HSGMII_RC_CFG_ID_SHIFT			(16)
#define HSGMII_RC_CFG_ID_MASK			(0x7<<HSGMII_RC_CFG_ID_SHIFT)
#define HSGMII_RC_CFG_PARA_RWCMD		(1<<0)


#define HSGMII_RC_CFG_PARA_METER_EN	(1<<2)
#define HSGMII_RC_CFG_PARA_PPS_MODE	(1<<1)
#define HSGMII_RC_CFG_PARA_TICK_SEL	(1<<0)

#define RC_TOKEN_RATE_INTEGER_SHIFT		(6)
#define RC_TOKEN_RATE_INTEGER_MASK		(0x3FFFF<<RC_TOKEN_RATE_INTEGER_SHIFT)
#define RC_TOKEN_RATE_FRACTION_MASK		(0x3F)

#define RC_PKT_MODE_BUCKET_SHIFT		(0)
#define RC_BYTE_MODE_BUCKET_SHIFT		(10)

#define HSGMII_RX_UC_RATE 	0
#define HSGMII_RX_BC_RATE 	1
#define HSGMII_RX_MC_RATE 	2
#define HSGMII_RX_TOTAL_RATE 3
#define HSGMII_TX_TOTAL_RATE 4

int xsi_mac_api_do_logic_reset(int hsgmii_index);
int xsi_mac_set_ratelimit(uint hsgmii_index, unsigned int type, uint rate, uint mode);

#endif /* __ARHT_HSGMII_H_ */
