#include "api/libcryp.h"
#include "api/regutils.h"
#include "api/libdma_regs.h"
#include "api/libdma.h"
#include "api/syscall.h"
#include "api/print.h"

static int is_busy(void)
{
    return get_reg(r_CORTEX_M_CRYP_SR, CRYP_SR_BUSY);
}

static enum crypto_mode current_mode;

void cryp_set_keylen(enum crypto_key_len  key_len)
{
    set_reg(r_CORTEX_M_CRYP_CR, key_len, CRYP_CR_KEYSIZE);
}


void cryp_set_iv(const uint8_t * iv)
{
    write_reg_value(r_CORTEX_M_CRYP_IVxLR(0), to_big32(*(uint32_t *) iv));
    iv += 4;
    write_reg_value(r_CORTEX_M_CRYP_IVxRR(0), to_big32(*(uint32_t *) iv));
    iv += 4;
    write_reg_value(r_CORTEX_M_CRYP_IVxLR(1), to_big32(*(uint32_t *) iv));
    iv += 4;
    write_reg_value(r_CORTEX_M_CRYP_IVxRR(1), to_big32(*(uint32_t *) iv));
    iv += 4;
}

static void cryp_set_datatype(uint8_t datatype)
{
    set_reg(r_CORTEX_M_CRYP_CR, datatype, CRYP_CR_DATATYPE);
}

void cryp_set_mode(enum crypto_algo mode)
{
    set_reg(r_CORTEX_M_CRYP_CR, mode, CRYP_CR_ALGOMODE);
    while (is_busy())
        continue;
}

static void set_dir(enum crypto_dir dir)
{
    set_reg(r_CORTEX_M_CRYP_CR, dir, CRYP_CR_ALGODIR);
}

void enable_crypt(void)
{
    set_reg_bits(r_CORTEX_M_CRYP_CR, CRYP_CR_CRYPEN_Msk);
}

static void disable_crypt(void)
{
    clear_reg_bits(r_CORTEX_M_CRYP_CR, CRYP_CR_CRYPEN_Msk);
}

static void flush_fifos(void)
{
    set_reg(r_CORTEX_M_CRYP_CR, 1, CRYP_CR_FFLUSH);
}

static int is_out_fifo_not_empty(void)
{
    return get_reg(r_CORTEX_M_CRYP_SR, CRYP_SR_OFNE);
}

static int is_in_fifo_not_empty(void)
{
    return get_reg(r_CORTEX_M_CRYP_SR, CRYP_SR_IFEM);
}

void cryp_wait_for_emtpy_fifos(void)
{
    while (get_reg_value(r_CORTEX_M_CRYP_SR, CRYP_SR_OFNE_Msk | CRYP_SR_IFEM_Msk, 0) != CRYP_SR_IFEM_Msk) {
        continue;
    }
}

static int is_in_fifo_not_full(void)
{
    return get_reg(r_CORTEX_M_CRYP_SR, CRYP_SR_IFNF);
}

void cryp_disable_dma(void)
{
    clear_reg_bits(r_CORTEX_M_CRYP_DMACR, CRYP_DMACR_DIEN_Msk);
    clear_reg_bits(r_CORTEX_M_CRYP_DMACR, CRYP_DMACR_DOEN_Msk);
}


void cryp_enable_dma(void)
{
    set_reg_bits(r_CORTEX_M_CRYP_DMACR, CRYP_DMACR_DIEN_Msk);
    set_reg_bits(r_CORTEX_M_CRYP_DMACR, CRYP_DMACR_DOEN_Msk);
}

enum crypto_dir cryp_get_dir(void)
{
    return (enum crypto_dir)get_reg_value(r_CORTEX_M_CRYP_CR, CRYP_CR_ALGODIR_Msk, CRYP_CR_ALGODIR_Pos);
}

void cryp_set_key(const uint8_t * key, enum crypto_key_len key_len)
{
    set_reg(r_CORTEX_M_CRYP_CR, key_len, CRYP_CR_KEYSIZE);

    key += (16 + (8 * key_len) - 4);
    write_reg_value(r_CORTEX_M_CRYP_KxRR(3), to_big32(*(uint32_t *) key));
    key -= 4;
    write_reg_value(r_CORTEX_M_CRYP_KxLR(3), to_big32(*(uint32_t *) key));
    key -= 4;
    write_reg_value(r_CORTEX_M_CRYP_KxRR(2), to_big32(*(uint32_t *) key));
    key -= 4;
    write_reg_value(r_CORTEX_M_CRYP_KxLR(2), to_big32(*(uint32_t *) key));
    key -= 4;

    if (key_len == CRYP_CR_KEYSIZE_256 || key_len == CRYP_CR_KEYSIZE_192) {
        write_reg_value(r_CORTEX_M_CRYP_KxRR(1), to_big32(*(uint32_t *) key));
        key -= 4;
        write_reg_value(r_CORTEX_M_CRYP_KxLR(1), to_big32(*(uint32_t *) key));
        key -= 4;
    }

    if (key_len == CRYP_CR_KEYSIZE_256) {
        write_reg_value(r_CORTEX_M_CRYP_KxRR(0), to_big32(*(uint32_t *) key));
        key -= 4;
        write_reg_value(r_CORTEX_M_CRYP_KxLR(0), to_big32(*(uint32_t *) key));
        key -= 4;
    }
}
/*
** configure, in both CRYP_CFG & CRYP_USER mode. beware to
** set key to 0 in CRYP_USER mode (or it will lead to memory exception)
*/

void cryp_init_injector(const uint8_t * key, enum crypto_key_len key_len)
{
//    disable_crypt();

    if (key) {
      cryp_set_key(key, key_len);
    }

    while (is_busy())
        continue;
    enable_crypt();
    flush_fifos();
}


bool cryp_dir_switched(enum crypto_dir dir)
{
    if (cryp_get_dir() != dir) {
        return true;
    }
    return false;
}

void cryp_init_user(enum crypto_key_len key_len,
               const uint8_t * iv, enum crypto_algo mode, enum crypto_dir dir)
{
//    disable_crypt();

    while (is_busy())
        continue;
    cryp_set_datatype(CRYP_CR_DATATYPE_BYTES);
    key_len = key_len;
    cryp_set_mode(mode);
    set_dir(dir);

    if (iv) {
        while (is_busy())
            continue;
        cryp_set_iv(iv);
    }

    enable_crypt();
    flush_fifos();
}



void cryp_init(const uint8_t * key, enum crypto_key_len key_len,
               const uint8_t * iv, enum crypto_algo mode, enum crypto_dir dir)
{

    disable_crypt();

    // TODO check that the acknowledgement is effective in the buffer
    // config
    if (iv) {
        cryp_set_iv(iv);
    }
    if (key) {
        cryp_set_key(key, key_len);
    }
    cryp_set_datatype(CRYP_CR_DATATYPE_BYTES);
    set_dir(dir);

    /* Prepare key when decryption is asked (except for CTR mode) */
    if (key && (dir == DECRYPT) && (mode != AES_CTR)) {
        cryp_set_mode(AES_KEY_PREPARE);
        enable_crypt();
        while (is_busy())
            continue;
    }
    cryp_set_mode(mode);

    enable_crypt();
    flush_fifos();
#if 0
        while (is_busy())
            continue;
    disable_crypt();

    // TODO check that the acknowledgement is effective in the buffer
    // config
    if (iv) {
        while (is_busy())
            continue;
        cryp_set_iv(iv);
    }
    if (key) {
        while (is_busy())
            continue;
        cryp_set_key(key, key_len);
    }
    cryp_set_datatype(CRYP_CR_DATATYPE_BYTES);

    if ((get_dir() != dir) || key != 0) {
        while (is_busy())
            continue;
        set_dir(dir);
        while (is_busy())
            continue;
       if (key_len) {
           cryp_set_keylen(key_len);
           while (is_busy())
               continue;
       } 
    //    printf("preparing key...\n");
        cryp_set_mode(AES_KEY_PREPARE);
        while (is_busy())
            continue;
    }
#if 0
    /* Prepare key when decryption is asked (except for CTR mode) */
    if ((dir == DECRYPT) && (mode != AES_CTR)) {
        enable_crypt();
        while (is_busy())
            continue;
    }
#endif
    while (is_busy())
        continue;
    cryp_set_mode(mode);

    while (is_busy())
        continue;
    enable_crypt();
    flush_fifos();
#endif
}

void cryp_do_no_dma(const uint8_t * data_in, uint8_t * data_out,
                    uint32_t data_len)
{
    uint32_t i, j;

    enable_crypt();

    /* The CRYP FIFO is 8 words deep. Thus we can put
     * two AES blocks inside it.
     */
    i = 0;
    while (i < data_len / 16) {
        uint32_t num_states = 1;
        i++;
        /* Consume two AES states if we can */
        if (((data_len / 16) - i) > 1) {
            num_states = 2;
            i++;
        }

        for (j = 0; j < (4 * num_states); j++) {
            write_reg_value(r_CORTEX_M_CRYP_DIN, *(uint32_t *) data_in);
            data_in += 4;
        }
        while (!is_out_fifo_not_empty())
            continue;
        for (j = 0; j < (4 * num_states); j++) {
            *(uint32_t *) data_out = read_reg_value(r_CORTEX_M_CRYP_DOUT);
            data_out += 4;
        }

        while (!is_in_fifo_not_full())
            continue;
    }

    while (is_busy())
        continue;
}

static dma_t dma_in;
static dma_t dma_out;


void cryp_do_dma(const uint8_t * bufin, const uint8_t * bufout, uint32_t size, int dma_in_desc, int dma_out_desc)
{

    cryp_enable_dma();
    e_syscall_ret ret;
    dma_in.dma          = DMA2;
    dma_in.stream       = DMA2_STREAM_CRYP_IN;
    dma_in.channel      = DMA2_CHANNEL_CRYP;
    dma_in.dir          = MEMORY_TO_PERIPHERAL;
    dma_in.in_addr      = (physaddr_t) bufin;
    dma_in.out_addr     = (volatile physaddr_t)r_CORTEX_M_CRYP_DIN;
    dma_in.in_prio      = DMA_PRI_MEDIUM;
    dma_in.size         = size;
    dma_in.mode         = DMA_DIRECT_MODE;
    dma_in.mem_inc      = 1;
    dma_in.dev_inc      = 0;
    dma_in.datasize     = DMA_DS_WORD;
    dma_in.mem_burst    = INCR4;
    dma_in.dev_burst    = INCR4;
    dma_in.flow_control = DMA_FLOWCTRL_DMA;
    dma_in.in_handler   = (user_dma_handler_t) 0;
    dma_in.out_handler  = (user_dma_handler_t) 0;    /* not used */

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP in...\n");
#endif

    // FIXME - handling ret value
    ret =
        sys_cfg(CFG_DMA_RECONF, &dma_in,
                (DMA_RECONF_BUFIN | DMA_RECONF_BUFOUT | DMA_RECONF_BUFSIZE),
                dma_in_desc);

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    // done by INIT_DONE, by kernel.
    // enable_crypt_dma(DMA2, DMA2_STREAM_CRYP_IN);

    dma_out.dma         = DMA2;
    dma_out.stream      = DMA2_STREAM_CRYP_OUT;
    dma_out.channel     = DMA2_CHANNEL_CRYP;
    dma_out.dir         = PERIPHERAL_TO_MEMORY;
    dma_out.in_addr     = (volatile physaddr_t)r_CORTEX_M_CRYP_DOUT;
    dma_out.out_addr    = (physaddr_t) bufout;
    dma_out.out_prio    = DMA_PRI_HIGH;
    dma_out.size        = size;
    dma_out.mode        = DMA_DIRECT_MODE;
    dma_out.mem_inc     = 1;
    dma_out.dev_inc     = 0;
    dma_out.datasize    = DMA_DS_WORD;
    dma_out.mem_burst   = INCR4;
    dma_out.dev_burst   = INCR4;
    dma_in.flow_control = DMA_FLOWCTRL_DMA;
    dma_out.in_handler  = (user_dma_handler_t) 0;    /* not used */
    dma_out.out_handler = (user_dma_handler_t) 0;

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP out...\n");
#endif

    // FIXME - handling ret value
    ret =
        sys_cfg(CFG_DMA_RECONF, & dma_out,
                (DMA_RECONF_BUFIN | DMA_RECONF_BUFOUT | DMA_RECONF_BUFSIZE),
                dma_out_desc);
#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
}

void cryp_init_dma(void *handler_in, void *handler_out, int dma_in_desc,
                   int dma_out_desc)
{
    e_syscall_ret ret;
    //enable_crypt();
    //cryp_enable_dma();
    cryp_disable_dma();

    dma_in.dma      = DMA2;
    dma_in.stream   = DMA2_STREAM_CRYP_IN;
    dma_in.channel  = DMA2_CHANNEL_CRYP;
    dma_in.dir      = MEMORY_TO_PERIPHERAL;
    dma_in.in_addr  = (physaddr_t) 0;
    dma_in.out_addr = (volatile physaddr_t)r_CORTEX_M_CRYP_DIN;
    dma_in.in_prio  = DMA_PRI_MEDIUM;
    dma_in.size     = 0;
    dma_in.mode     = DMA_DIRECT_MODE;
    dma_in.mem_inc  = 1;
    dma_in.dev_inc  = 0;
    dma_in.datasize = DMA_DS_WORD;
    dma_in.mem_burst    = INCR4;
    dma_in.dev_burst    = INCR4;
    dma_in.in_handler   = (user_dma_handler_t) handler_in;
    dma_in.out_handler  = (user_dma_handler_t) handler_out;  /* not used */

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP in...\n");
#endif

    // FIXME - handling ret value
    ret = sys_cfg(CFG_DMA_RECONF, &dma_in,
                  (DMA_RECONF_HANDLERS | DMA_RECONF_MODE | DMA_RECONF_PRIO),
                  dma_in_desc);

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    // done by INIT_DONE, by kernel.
    // enable_crypt_dma(DMA2, DMA2_STREAM_CRYP_IN);

    dma_out.dma         = DMA2;
    dma_out.stream      = DMA2_STREAM_CRYP_OUT;
    dma_out.channel     = DMA2_CHANNEL_CRYP;
    dma_out.dir         = PERIPHERAL_TO_MEMORY;
    dma_out.in_addr     = (volatile physaddr_t)r_CORTEX_M_CRYP_DOUT;
    dma_out.out_addr    = (physaddr_t) 0;
    dma_out.out_prio    = DMA_PRI_HIGH;
    dma_out.size        = 0;
    dma_out.mode        = DMA_DIRECT_MODE;
    dma_out.mem_inc     = 1;
    dma_out.dev_inc     = 0;
    dma_out.datasize    = DMA_DS_WORD;
    dma_out.mem_burst   = INCR4;
    dma_out.dev_burst   = INCR4;
    dma_out.in_handler  = (user_dma_handler_t) handler_in;   /* not used */
    dma_out.out_handler = (user_dma_handler_t) handler_out;

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP out...\n");
#endif

    // FIXME - handling ret value
    ret =
        sys_cfg(CFG_DMA_RECONF, &dma_out,
                (DMA_RECONF_HANDLERS | DMA_RECONF_MODE | DMA_RECONF_PRIO),
                dma_out_desc);

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    cryp_enable_dma();
}


void cryp_early_init(bool with_dma,
                     enum crypto_usage usage,
                     enum crypto_mode mode,
                     int *dma_in_desc,
                     int *dma_out_desc)
{
    const char *name = "cryp";
    e_syscall_ret ret = 0;

    current_mode = mode;
    device_t dev = { 0 };
    int      dev_desc = 0;
    strncpy(dev.name, name, sizeof (dev.name));
    dev.address = 0x50060000;
    if (usage == CRYP_USER) {
        dev.size = 0x100;
    } else {
        dev.size = 0x1000;
    }
    dev.map_mode = DEV_MAP_AUTO;
    dev.irq_num = 0;
    dev.gpio_num = 0;

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("registering cryp-user driver\n");
#endif

    // FIXME - proper handling of ret
    ret = sys_init(INIT_DEVACCESS, &dev, &dev_desc);

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif

    if (!with_dma) {
      goto end;
    }
    dma_in.channel = DMA2_CHANNEL_CRYP;
    dma_in.dir = MEMORY_TO_PERIPHERAL;
    dma_in.in_addr = (physaddr_t) 0;
    dma_in.out_addr = (volatile physaddr_t)r_CORTEX_M_CRYP_DIN;
    dma_in.in_prio = DMA_PRI_MEDIUM;
    dma_in.dma = DMA2;
    dma_in.size = 0;
    dma_in.stream = DMA2_STREAM_CRYP_IN;
    dma_in.mode = DMA_DIRECT_MODE;
    dma_in.mem_inc = 1;
    dma_in.dev_inc = 0;
    dma_in.datasize = DMA_DS_WORD;
    dma_in.mem_burst = INCR4;
    dma_in.dev_burst = INCR4;
    dma_in.in_handler = (user_dma_handler_t) 0;
    dma_in.out_handler = (user_dma_handler_t) 0;    /* not used */

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP in...\n");
#endif

    // FIXME - proper handling of ret and of dma_in_desc (if < 0)
    ret = sys_init(INIT_DMA, &dma_in, dma_in_desc);

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    // done by INIT_DONE, by kernel.
    // enable_crypt_dma(DMA2, DMA2_STREAM_CRYP_IN);

    dma_out.channel = DMA2_CHANNEL_CRYP;
    dma_out.dir = PERIPHERAL_TO_MEMORY;
    dma_out.in_addr = (volatile physaddr_t)r_CORTEX_M_CRYP_DOUT;
    dma_out.out_addr = (physaddr_t) 0;
    dma_out.out_prio = DMA_PRI_HIGH;
    dma_out.dma = DMA2;
    dma_out.size = 0;
    dma_out.stream = DMA2_STREAM_CRYP_OUT;
    dma_out.mode = DMA_DIRECT_MODE;
    dma_out.mem_inc = 1;
    dma_out.dev_inc = 0;
    dma_out.datasize = DMA_DS_WORD;
    dma_out.mem_burst = INCR4;
    dma_out.dev_burst = INCR4;
    dma_out.in_handler = (user_dma_handler_t) 0;    /* not used */
    dma_out.out_handler = (user_dma_handler_t) 0;

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP out...\n");
#endif

    // FIXME - proper handling of ret and of dma_out_desc (if < 0)
    ret = sys_init(INIT_DMA, &dma_out, dma_out_desc);

#ifdef CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
end:
    return;
}
