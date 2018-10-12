#ifndef CRYP_H
#define CRYP_H

#include "api/types.h"

#define CRYP_BASE			0x50060000

#define r_CORTEX_M_CRYP_CR		REG_ADDR(CRYP_BASE + 0x00)
#define r_CORTEX_M_CRYP_SR		REG_ADDR(CRYP_BASE + 0x04)
#define r_CORTEX_M_CRYP_DIN		REG_ADDR(CRYP_BASE + 0x08)
#define r_CORTEX_M_CRYP_DOUT		REG_ADDR(CRYP_BASE + 0x0c)
#define r_CORTEX_M_CRYP_DMACR		REG_ADDR(CRYP_BASE + 0x10)
#define r_CORTEX_M_CRYP_IMSCR		REG_ADDR(CRYP_BASE + 0x14)
#define r_CORTEX_M_CRYP_RISR		REG_ADDR(CRYP_BASE + 0x18)
#define r_CORTEX_M_CRYP_MISR		REG_ADDR(CRYP_BASE + 0x1c)
#define r_CORTEX_M_CRYP_KxLR(n)	REG_ADDR(CRYP_BASE + 0x20 + (n * 8))
#define r_CORTEX_M_CRYP_KxRR(n)	REG_ADDR(CRYP_BASE + 0x24 + (n * 8))
#define r_CORTEX_M_CRYP_IVxLR(n)	REG_ADDR(CRYP_BASE + 0x40 + (n * 8))
#define r_CORTEX_M_CRYP_IVxRR(n)	REG_ADDR(CRYP_BASE + 0x44 + (n * 8))

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

/*
** Cryp user mode (no key injection authorized) or crypto configurator
** mode
*/
enum crypto_usage {
  CRYP_USER = 0,
  CRYP_CFG,
};

/* Benchmode, with key injection from crypto user toward configurator via
 * IPC (require configurator to be configured in the same way), or in
 * prod mode, asking the configurator to inject the key based on the
 * smartcard content
 */
enum crypto_mode {
    CRYP_BENCHMODE,
    CRYP_PRODMODE,
};

enum crypto_key_len {
    KEY_128,
    KEY_192,
    KEY_256
};

enum crypto_algo {
    TDES_ECB,
    TDES_CBC,
    DES_ECB,
    DES_CBC,
    AES_ECB,
    AES_CBC,
    AES_CTR,
    AES_KEY_PREPARE
};

enum crypto_dir {
    ENCRYPT,
    DECRYPT
};

/**
 * encrypt_no_dma - Encrypt/Decrypt data without DMA
 * @data_in: Address of the buffer where this function will read data. The
 * buffer size is @data_len.
 * @data_out: Address of the buffer where encrypted/decrypted data will be
 * written. The buffer size must be at least @data_len.
 * @data_len: Number of bytes to encrypt/decrypt.
 *
 * You should call aes_init before calling this function.
 *
 * This function returns only when data is encrypted or decrypted (depending on
 * the direction given in aes_init).
 *
 */
void soc_encrypt_no_dma(const uint8_t * data_in, uint8_t * data_out,
                        uint32_t data_len);

/**
 * encrypt_dma - Encrypt/Decrypt data using DMA
 * @data_in: Address of the buffer where this function will read data. The
 * buffer size is @data_len.
 * @data_out: Address of the buffer where encrypted/decrypted data will be
 * written. The buffer size must be at least @data_len.
 * @data_len: Number of bytes to encrypt/decrypt.
 * @dma_in_complete: Callback called when the IN DMA transfer (@data_in to
 * CRYP) is finished.
 * @dma_out_complete: Callback called when the OUT DMA transfer (CRYP to
 * @data_out) is finished.
 *
 * You should call aes_init before calling this function.
 *
 * This function returns as soon as the DMA transfer is enabled. You should not
 * read in @data_out buffer before @transfer_complete is called.
 *
 */
//void soc_encrypt_dma(const uint8_t *data_in, uint8_t *data_out, uint32_t data_len,
//       void (*dma_in_complete)(void), void (*dma_out_complete)(void));

void cryp_set_key(const uint8_t * key, enum crypto_key_len key_len);

void cryp_set_iv(const uint8_t * iv);

void cryp_enable_dma(void);

void enable_crypt(void);

/**
 * aes_init - Initialize an encryption/decryption
 * @key: Address of the key. The buffer that contains the key must have a size
 * of 128, 192 or 256 bytes depending on the vluae of @key_len.
 * @key_len: The size of the key.
 * @iv: Address of the initialization vector. The buffer size must be at least
 * 128 bytes.
 * @mode: Algorithm: TDES_ECB, TDES_CBC, DES_ECB, DES_CBC, AES_ECB, AES_CBC or
 * AES_CTR.
 * @dir: Direction: ENCRYPT or DECRYPT.
 *
 * This function should be called before calling encrypt_dma or encrypt_no_dma.
 */

void cryp_init_user(enum crypto_key_len key_len,
               const uint8_t * iv, enum crypto_algo mode, enum crypto_dir dir);

void cryp_init_injector(const uint8_t * key, enum crypto_key_len key_len);

void cryp_init(const uint8_t * key, enum crypto_key_len key_len,
               const uint8_t * iv, enum crypto_algo mode, enum crypto_dir dir);

/* initialize DMA streams for cryp (not runnable, should be reconf later) */
void cryp_early_init(bool with_dma,
                     enum crypto_usage usage,
                     enum crypto_mode mode,
                     int * dma_in_desc,
                     int * dma_out_desc);

/* configure the DMA streams with proper informations (handlers, buffers...) */
void cryp_init_dma(void *handler_in, void *handler_out, int dma_in_desc,
                   int dma_out_desc);

/*
 * start cryp with no DMA support
 */
void cryp_do_no_dma(const uint8_t * data_in, uint8_t * data_out,
                    uint32_t data_len);

/*
 * start cryp using DMA (this requires libdma)
 */
void cryp_do_dma(const uint8_t * bufin, const uint8_t * bufout, uint32_t size,
                 int dma_in_desc, int dma_out_desc);


enum crypto_dir cryp_get_dir(void);

bool cryp_dir_switched(enum crypto_dir dir);

void cryp_set_mode(enum crypto_algo mode);

void cryp_wait_for_emtpy_fifos(void);
#endif                          /* CRYP_H */
