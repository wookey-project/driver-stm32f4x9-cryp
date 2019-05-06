#ifndef CRYP_REGS_H
#define CRYP_REGS_H

#include "libc/types.h"

#include "generated/cryp_user.h"
#include "generated/cryp_cfg.h"

/* The CRYP base is common to USER and CFG */
#define CRYP_BASE			CRYP_USER_BASE

#define r_CORTEX_M_CRYP_CR		REG_ADDR(CRYP_BASE + 0x00)
#define r_CORTEX_M_CRYP_SR		REG_ADDR(CRYP_BASE + 0x04)
#define r_CORTEX_M_CRYP_DIN		REG_ADDR(CRYP_BASE + 0x08)
#define r_CORTEX_M_CRYP_DOUT		REG_ADDR(CRYP_BASE + 0x0c)
#define r_CORTEX_M_CRYP_DMACR		REG_ADDR(CRYP_BASE + 0x10)
#define r_CORTEX_M_CRYP_IMSCR		REG_ADDR(CRYP_BASE + 0x14)
#define r_CORTEX_M_CRYP_RISR		REG_ADDR(CRYP_BASE + 0x18)
#define r_CORTEX_M_CRYP_MISR		REG_ADDR(CRYP_BASE + 0x1c)
#define r_CORTEX_M_CRYP_KxLR(n)	REG_ADDR(CRYP_BASE + 0x20 + ((n) * 8))
#define r_CORTEX_M_CRYP_KxRR(n)	REG_ADDR(CRYP_BASE + 0x24 + ((n) * 8))
#define r_CORTEX_M_CRYP_IVxLR(n)	REG_ADDR(CRYP_BASE + 0x40 + ((n) * 8))
#define r_CORTEX_M_CRYP_IVxRR(n)	REG_ADDR(CRYP_BASE + 0x44 + ((n) * 8))

/* CRYP control register */
#define CRYP_CR_ALGODIR_Pos	2
#define CRYP_CR_ALGODIR_Msk	((uint32_t)0x1 << CRYP_CR_ALGODIR_Pos)
#	define CRYP_CR_ALGODIR_ENCRYPT		0
#	define CRYP_CR_ALGODIR_DECRYPT		1
#define CRYP_CR_ALGOMODE_Pos	3
#define CRYP_CR_ALGOMODE_Msk	((uint32_t)0x7 << CRYP_CR_ALGOMODE_Pos)
#	define CRYP_CR_ALGOMODE_TDES_ECB	0
#	define CRYP_CR_ALGOMODE_TDES_CBC	1
#	define CRYP_CR_ALGOMODE_DES_ECB		2
#	define CRYP_CR_ALGOMODE_DES_CBC		3
#	define CRYP_CR_ALGOMODE_AES_ECB		4
#	define CRYP_CR_ALGOMODE_AES_CBC		5
#	define CRYP_CR_ALGOMODE_AES_CTR		6
#	define CRYP_CR_ALGOMODE_AES_KEY_PREPARE	7
#define CRYP_CR_DATATYPE_Pos	6
#define CRYP_CR_DATATYPE_Msk	((uint32_t)0x3 << CRYP_CR_DATATYPE_Pos)
#	define CRYP_CR_DATATYPE_WORDS		0
#	define CRYP_CR_DATATYPE_HALF_WORDS	1
#	define CRYP_CR_DATATYPE_BYTES		2
#	define CRYP_CR_DATATYPE_BITS		3
#define CRYP_CR_KEYSIZE_Pos	8
#define CRYP_CR_KEYSIZE_Msk	((uint32_t)0x3 << CRYP_CR_KEYSIZE_Pos)
#	define CRYP_CR_KEYSIZE_128		0
#	define CRYP_CR_KEYSIZE_192		1
#	define CRYP_CR_KEYSIZE_256		2
#define CRYP_CR_FFLUSH_Pos	14
#define CRYP_CR_FFLUSH_Msk	((uint32_t)1 << CRYP_CR_FFLUSH_Pos)
#define CRYP_CR_CRYPEN_Pos	15
#define CRYP_CR_CRYPEN_Msk	((uint32_t)1 << CRYP_CR_CRYPEN_Pos)

/* CRYP status register */
#define CRYP_SR_IFEM_Pos	0
#define CRYP_SR_IFEM_Msk	((uint32_t)1 << CRYP_SR_IFEM_Pos)
#define CRYP_SR_IFNF_Pos	1
#define CRYP_SR_IFNF_Msk	((uint32_t)1 << CRYP_SR_IFNF_Pos)
#define CRYP_SR_OFNE_Pos	2
#define CRYP_SR_OFNE_Msk	((uint32_t)1 << CRYP_SR_OFNE_Pos)
#define CRYP_SR_OFFU_Pos	3
#define CRYP_SR_OFFU_Msk	((uint32_t)1 << CRYP_SR_OFFU_Pos)
#define CRYP_SR_BUSY_Pos	4
#define CRYP_SR_BUSY_Msk	((uint32_t)1 << CRYP_SR_BUSY_Pos)

/* CRYP DMA control register */
#define CRYP_DMACR_DIEN_Pos	0
#define CRYP_DMACR_DIEN_Msk	((uint32_t)1 << CRYP_DMACR_DIEN_Pos)
#define CRYP_DMACR_DOEN_Pos	1
#define CRYP_DMACR_DOEN_Msk	((uint32_t)1 << CRYP_DMACR_DOEN_Pos)

/* CRYP interrupt mask set/clear register */
#define CRYP_IMSCR_INIM_Pos	0
#define CRYP_IMSCR_INIM_Msk	((uint32_t)1 << CRYP_IMSCR_INIM_Pos)
#define CRYP_IMSCR_OUTIM_Pos	1
#define CRYP_IMSCR_OUTIM_Msk	((uint32_t)1 << CRYP_IMSCR_OUTIM_Pos)

/* CRYP raw interrupt status register */
#define CRYP_RISR_INRIS_Pos	0
#define CRYP_RISR_INRIS_Msk	((uint32_t)1 << CRYP_RISR_INRIS_Pos)
#define CRYP_RISR_OUTRIS_Pos	1
#define CRYP_RISR_OUTRIS_Msk	((uint32_t)1 << CRYP_RISR_OUTRIS_Pos)

/* CRYP masked interrupt status register */
#define CRYP_MISR_INMIS_Pos	0
#define CRYP_MISR_INMIS_Msk	((uint32_t)1 << CRYP_MISR_INMIS_Pos)
#define CRYP_MISR_OUTMIS_Pos	1
#define CRYP_MISR_OUTMIS_Msk	((uint32_t)1 << CRYP_MISR_OUTMIS_Pos)
#endif                          /* CRYP_REGS_H */
