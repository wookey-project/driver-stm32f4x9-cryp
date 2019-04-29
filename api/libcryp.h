#ifndef CRYP_H
#define CRYP_H

#include "libc/types.h"
#include "libc/syscall.h"

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

typedef enum {
    CRYP_MAP_AUTO,
    CRYP_MAP_VOLUNTARY
} cryp_map_mode_t;


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

int cryp_map(void);
int cryp_unmap(void);

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

void cryp_set_iv(const uint8_t * iv, unsigned int iv_len);

void cryp_get_iv(uint8_t * iv, unsigned int iv_len);

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
               const uint8_t * iv, unsigned int iv_len, enum crypto_algo mode, enum crypto_dir dir);

void cryp_init_injector(const uint8_t * key, enum crypto_key_len key_len);

void cryp_init(const uint8_t * key, enum crypto_key_len key_len,
               const uint8_t * iv, unsigned int iv_len, enum crypto_algo mode, enum crypto_dir dir);

/* initialize DMA streams for cryp (not runnable, should be reconf later) */
int cryp_early_init(bool with_dma,
                     cryp_map_mode_t map_mode,
                     enum crypto_usage usage,
                     int * dma_in_desc,
                     int * dma_out_desc);

/* configure the DMA streams with proper informations (handlers, buffers...) */
int cryp_init_dma(user_dma_handler_t handler_in, user_dma_handler_t handler_out, int dma_in_desc,
                   int dma_out_desc);

/*
 * start cryp with no DMA support
 */
int cryp_do_no_dma(const uint8_t * data_in, uint8_t * data_out,
                    uint32_t data_len);

/*
 * start cryp using DMA (this requires libdma)
 */
int cryp_do_dma(const uint8_t * bufin, const uint8_t * bufout, uint32_t size,
                 int dma_in_desc, int dma_out_desc);


enum crypto_dir cryp_get_dir(void);

bool cryp_dir_switched(enum crypto_dir dir);

void cryp_set_mode(enum crypto_algo mode);

void cryp_wait_for_emtpy_fifos(void);

void cryp_flush_fifos(void);
#endif                          /* CRYP_H */
