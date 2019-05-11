#include "api/libcryp.h"
#include "cryp_regs.h"
#include "libc/regutils.h"
#include "libc/syscall.h"
#include "libc/stdio.h"
#include "libc/nostd.h"
#include "libc/string.h"
#include "libc/arpa/inet.h"

#define CONFIG_USR_DRV_CRYP_DEBUG 0

#define DMA_CRYP		CRYP_USER_DMA_CTRL
#define DMA_CHANNEL_CRYP_IN 	CRYP_USER_DMA_IN_CHANNEL
#define DMA_CHANNEL_CRYP_OUT 	CRYP_USER_DMA_OUT_CHANNEL
#define DMA_STREAM_CRYP_OUT	CRYP_USER_DMA_OUT_STREAM
#define DMA_STREAM_CRYP_IN	CRYP_USER_DMA_IN_STREAM

static volatile bool cryp_is_mapped = false;

static volatile int      dev_cryp_desc = 0;

int cryp_map(void)
{
    if (cryp_is_mapped == false) {
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Mapping cryp\n");
#endif
        uint8_t ret;
        ret = sys_cfg(CFG_DEV_MAP, dev_cryp_desc);
        cryp_is_mapped = true;
        if (ret != SYS_E_DONE) {
#if CONFIG_USR_DRV_CRYP_DEBUG
            printf("Unable to map cryp!\n");
#endif
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

int cryp_unmap(void)
{
    if (cryp_is_mapped) {
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Unmapping cryp\n");
#endif
        uint8_t ret;
        ret = sys_cfg(CFG_DEV_UNMAP, dev_cryp_desc);
        cryp_is_mapped = false;
        if (ret != SYS_E_DONE) {
#if CONFIG_USR_DRV_CRYP_DEBUG
            printf("Unable to unmap cryp!\n");
#endif
            goto err;
        }
    }

    return 0;
err:
    return -1;
}


static int is_busy(void)
{
    return get_reg(r_CORTEX_M_CRYP_SR, CRYP_SR_BUSY);
}

void cryp_set_keylen(enum crypto_key_len  key_len)
{

   while (is_busy())
       continue;
    set_reg(r_CORTEX_M_CRYP_CR, key_len, CRYP_CR_KEYSIZE);
}


void cryp_set_iv(const uint8_t * iv, unsigned int iv_len)
{
    while (is_busy())
       continue;
    if(iv == NULL){
       return;
    }
    /* IV is either 64 bits (for (T)DES) or 128 bits (for AES) */
    if((iv_len != 8) && (iv_len != 16)){
        return;
    }
    write_reg_value(r_CORTEX_M_CRYP_IVxLR(0), htonl(*(const uint32_t *) iv));
    iv += 4;
    write_reg_value(r_CORTEX_M_CRYP_IVxRR(0), htonl(*(const uint32_t *) iv));
    if(iv_len == 16){
        iv += 4;
        write_reg_value(r_CORTEX_M_CRYP_IVxLR(1), htonl(*(const uint32_t *) iv));
        iv += 4;
        write_reg_value(r_CORTEX_M_CRYP_IVxRR(1), htonl(*(const uint32_t *) iv));
        iv += 4;
    }
}

void cryp_get_iv(uint8_t * iv, unsigned int iv_len)
{
    while (is_busy())
       continue;
    if(iv == NULL){
       return;
    }
    /* IV is either 64 bits (for (T)DES) or 128 bits (for AES) */
    if((iv_len != 8) && (iv_len != 16)){
        return;
    }
    *(uint32_t *) iv = htonl(read_reg_value(r_CORTEX_M_CRYP_IVxLR(0)));
    iv += 4;
    *(uint32_t *) iv = htonl(read_reg_value(r_CORTEX_M_CRYP_IVxRR(0)));
    if(iv_len == 16){
        iv += 4;
        *(uint32_t *) iv = htonl(read_reg_value(r_CORTEX_M_CRYP_IVxLR(1)));
        iv += 4;
        *(uint32_t *) iv = htonl(read_reg_value(r_CORTEX_M_CRYP_IVxRR(1)));
        iv += 4;
    }
}

static void cryp_set_datatype(uint8_t datatype)
{
   while (is_busy()){
       continue;
   }
   set_reg(r_CORTEX_M_CRYP_CR, datatype, CRYP_CR_DATATYPE);
}

void cryp_set_mode(enum crypto_algo mode)
{
   while (is_busy()){
       continue;
   }
   set_reg(r_CORTEX_M_CRYP_CR, mode, CRYP_CR_ALGOMODE);
}

static void set_dir(enum crypto_dir dir)
{
   while (is_busy()){
       continue;
    }
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

void cryp_flush_fifos(void)
{
    set_reg(r_CORTEX_M_CRYP_CR, 1, CRYP_CR_FFLUSH);
   while (is_busy()){
       continue;
   }
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
    if(key == NULL){
        return;
    }
    set_reg(r_CORTEX_M_CRYP_CR, key_len, CRYP_CR_KEYSIZE);

    key += (16 + (8 * key_len) - 4);
    write_reg_value(r_CORTEX_M_CRYP_KxRR(3), htonl(*(const uint32_t *) key));
    key -= 4;
    write_reg_value(r_CORTEX_M_CRYP_KxLR(3), htonl(*(const uint32_t *) key));
    key -= 4;
    write_reg_value(r_CORTEX_M_CRYP_KxRR(2), htonl(*(const uint32_t *) key));
    key -= 4;
    write_reg_value(r_CORTEX_M_CRYP_KxLR(2), htonl(*(const uint32_t *) key));
    key -= 4;

    if ((key_len == CRYP_CR_KEYSIZE_256) || (key_len == CRYP_CR_KEYSIZE_192)) {
        write_reg_value(r_CORTEX_M_CRYP_KxRR(1), htonl(*(const uint32_t *) key));
        key -= 4;
        write_reg_value(r_CORTEX_M_CRYP_KxLR(1), htonl(*(const uint32_t *) key));
        key -= 4;
    }

    if (key_len == CRYP_CR_KEYSIZE_256) {
        write_reg_value(r_CORTEX_M_CRYP_KxRR(0), htonl(*(const uint32_t *) key));
        key -= 4;
        write_reg_value(r_CORTEX_M_CRYP_KxLR(0), htonl(*(const uint32_t *) key));
        key -= 4;
    }
    while (is_busy()){
        continue;
    }
    return;
}
/*
** configure, in both CRYP_CFG & CRYP_USER mode. beware to
** set key to 0 in CRYP_USER mode (or it will lead to memory exception)
*/

void cryp_init_injector(const uint8_t * key, enum crypto_key_len key_len)
{
    if (!cryp_is_mapped) {
        uint8_t ret;
        ret = sys_cfg(CFG_DEV_MAP, dev_cryp_desc);
        if (ret != SYS_E_DONE) {
            printf("Unable to map cryp!\n");
            goto err;
        }
        cryp_is_mapped = true;
    }
    disable_crypt();

    if (key) {
      cryp_set_key(key, key_len);
    }

    enable_crypt();
    cryp_flush_fifos();
err:
    return;
}


bool cryp_dir_switched(enum crypto_dir dir)
{
    if (cryp_get_dir() != dir) {
        return true;
    }
    return false;
}

void cryp_init_user(enum crypto_key_len key_len __attribute__((unused)) /* TODO: to be removed */,
               const uint8_t * iv, unsigned int iv_len, enum crypto_algo mode, enum crypto_dir dir)
{
    if (!cryp_is_mapped) {
        sys_cfg(CFG_DEV_MAP, dev_cryp_desc);
    }
    cryp_flush_fifos();
    cryp_set_datatype(CRYP_CR_DATATYPE_BYTES);
    cryp_set_mode(mode);
    set_dir(dir);

    if (iv) {
        disable_crypt();
        cryp_set_iv(iv, iv_len);
        enable_crypt();
    }

    enable_crypt();
    cryp_flush_fifos();
    return;
}



void cryp_init(const uint8_t * key, enum crypto_key_len key_len,
               const uint8_t * iv, unsigned int iv_len, enum crypto_algo mode, enum crypto_dir dir)
{

    disable_crypt();

    // TODO check that the acknowledgement is effective in the buffer
    // config
    if (iv) {
        cryp_set_iv(iv, iv_len);
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
    cryp_flush_fifos();
    return;
}

int cryp_do_no_dma(const uint8_t * data_in, uint8_t * data_out,
                    uint32_t data_len)
{
    uint32_t i, j;

    enable_crypt();

    /* The CRYP FIFO is 8 words deep. Thus we can put
     * two AES blocks inside it.
     */
    i = 0;
    while (i < (data_len / 16)) {
        uint32_t num_states = 1;
        i++;
        /* Consume two AES states if we can */
        if (((data_len / 16) - i) > 1) {
            num_states = 2;
            i++;
        }

        for (j = 0; j < (4 * num_states); j++) {
            write_reg_value(r_CORTEX_M_CRYP_DIN, *(const uint32_t *) data_in);
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

    return 0;
}

static dma_t dma_in;
static dma_t dma_out;


int cryp_do_dma(const uint8_t * bufin, const uint8_t * bufout, uint32_t size, int dma_in_desc, int dma_out_desc)
{
    /* DMA addresses must be word aligned, perform a sanity check */
    if((((uint32_t)bufin % 4) != 0) || (((uint32_t)bufout % 4) != 0)){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, DMA buffers addresses not word aligned! (bufin=%x, bufout=%x)\n", bufin, bufout);
#endif
        goto err;
    }

    cryp_enable_dma();
    e_syscall_ret ret;
    dma_in.dma          = DMA_CRYP;
    dma_in.stream       = DMA_STREAM_CRYP_IN;
    dma_in.channel      = DMA_CHANNEL_CRYP_IN;
    dma_in.dir          = MEMORY_TO_PERIPHERAL;
    dma_in.in_addr      = (physaddr_t) bufin;
    dma_in.out_addr     = (volatile physaddr_t)r_CORTEX_M_CRYP_DIN;
    dma_in.in_prio      = DMA_PRI_MEDIUM;
    dma_in.size         = size;
    dma_in.mode         = DMA_DIRECT_MODE;
    dma_in.mem_inc      = 1;
    dma_in.dev_inc      = 0;
    dma_in.datasize     = DMA_DS_WORD;
    dma_in.mem_burst    = DMA_BURST_INC4;
    dma_in.dev_burst    = DMA_BURST_INC4;
    dma_in.flow_control = DMA_FLOWCTRL_DMA;
    dma_in.in_handler   = (user_dma_handler_t) 0;
    dma_in.out_handler  = (user_dma_handler_t) 0;    /* not used */

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP in...\n");
#endif

    ret =
        sys_cfg(CFG_DMA_RECONF, &dma_in,
                (DMA_RECONF_BUFIN | DMA_RECONF_BUFOUT | DMA_RECONF_BUFSIZE),
                dma_in_desc);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_cfg CFG_DMA_RECONF error!\n");
#endif
        goto err;
    }
#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    // done by INIT_DONE, by kernel.

    dma_out.dma         = DMA_CRYP;
    dma_out.stream      = DMA_STREAM_CRYP_OUT;
    dma_out.channel     = DMA_CHANNEL_CRYP_OUT;
    dma_out.dir         = PERIPHERAL_TO_MEMORY;
    dma_out.in_addr     = (volatile physaddr_t)r_CORTEX_M_CRYP_DOUT;
    dma_out.out_addr    = (physaddr_t) bufout;
    dma_out.out_prio    = DMA_PRI_HIGH;
    dma_out.size        = size;
    dma_out.mode        = DMA_DIRECT_MODE;
    dma_out.mem_inc     = 1;
    dma_out.dev_inc     = 0;
    dma_out.datasize    = DMA_DS_WORD;
    dma_out.mem_burst   = DMA_BURST_INC4;
    dma_out.dev_burst   = DMA_BURST_INC4;
    dma_in.flow_control = DMA_FLOWCTRL_DMA;
    dma_out.in_handler  = (user_dma_handler_t) 0;    /* not used */
    dma_out.out_handler = (user_dma_handler_t) 0;

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP out...\n");
#endif

    ret =
        sys_cfg(CFG_DMA_RECONF, & dma_out,
                (DMA_RECONF_BUFIN | DMA_RECONF_BUFOUT | DMA_RECONF_BUFSIZE),
                dma_out_desc);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_cfg CFG_DMA_RECONF error!\n");
#endif
        goto err;
    }
#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif

    return 0;

err:
    return -1;
}

int cryp_init_dma(user_dma_handler_t handler_in, user_dma_handler_t handler_out, int dma_in_desc,
                   int dma_out_desc)
{
    e_syscall_ret ret;
    cryp_disable_dma();

    dma_in.dma      = DMA_CRYP;
    dma_in.stream   = DMA_STREAM_CRYP_IN;
    dma_in.channel  = DMA_CHANNEL_CRYP_IN;
    dma_in.dir      = MEMORY_TO_PERIPHERAL;
    dma_in.in_addr  = (physaddr_t) 0;
    dma_in.out_addr = (volatile physaddr_t)r_CORTEX_M_CRYP_DIN;
    dma_in.in_prio  = DMA_PRI_MEDIUM;
    dma_in.size     = 0;
    dma_in.mode     = DMA_DIRECT_MODE;
    dma_in.mem_inc  = 1;
    dma_in.dev_inc  = 0;
    dma_in.datasize = DMA_DS_WORD;
    dma_in.mem_burst    = DMA_BURST_INC4;
    dma_in.dev_burst    = DMA_BURST_INC4;
    dma_in.in_handler   = (user_dma_handler_t) handler_in;
    dma_in.out_handler  = (user_dma_handler_t) handler_out;  /* not used */

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP in...\n");
#endif

    ret = sys_cfg(CFG_DMA_RECONF, &dma_in,
                  (DMA_RECONF_HANDLERS | DMA_RECONF_MODE | DMA_RECONF_PRIO),
                  dma_in_desc);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_cfg CFG_DMA_RECONF error!\n");
#endif
        goto err;
    }
#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    // done by INIT_DONE, by kernel.

    dma_out.dma         = DMA_CRYP;
    dma_out.stream      = DMA_STREAM_CRYP_OUT;
    dma_out.channel     = DMA_CHANNEL_CRYP_OUT;
    dma_out.dir         = PERIPHERAL_TO_MEMORY;
    dma_out.in_addr     = (volatile physaddr_t)r_CORTEX_M_CRYP_DOUT;
    dma_out.out_addr    = (physaddr_t) 0;
    dma_out.out_prio    = DMA_PRI_HIGH;
    dma_out.size        = 0;
    dma_out.mode        = DMA_DIRECT_MODE;
    dma_out.mem_inc     = 1;
    dma_out.dev_inc     = 0;
    dma_out.datasize    = DMA_DS_WORD;
    dma_out.mem_burst   = DMA_BURST_INC4;
    dma_out.dev_burst   = DMA_BURST_INC4;
    dma_out.in_handler  = (user_dma_handler_t) handler_in;   /* not used */
    dma_out.out_handler = (user_dma_handler_t) handler_out;

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP out...\n");
#endif

    ret =
        sys_cfg(CFG_DMA_RECONF, &dma_out,
                (DMA_RECONF_HANDLERS | DMA_RECONF_MODE | DMA_RECONF_PRIO),
                dma_out_desc);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_cfg CFG_DMA_RECONF error!\n");
#endif
        goto err;
    }

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    cryp_enable_dma();

    return 0;

err:
    return -1;
}


int cryp_early_init(bool with_dma,
                     cryp_map_mode_t map_mode,
                     enum crypto_usage usage,
                     int *dma_in_desc,
                     int *dma_out_desc)
{
    const char *name = "cryp";
    e_syscall_ret ret = 0;

    device_t dev;
    memset((void*)&dev, 0, sizeof(device_t));
    strncpy(dev.name, name, sizeof (dev.name));
    if (usage == CRYP_USER) {
        dev.address = cryp_user_dev_infos.address;
        dev.size = cryp_user_dev_infos.size;
    } else if (usage == CRYP_CFG){
        dev.address = cryp_cfg_dev_infos.address;
        dev.size = cryp_cfg_dev_infos.size;
    }
    else{
        /* This is an error! */
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, asking for bad usage mode %d!\n", usage);
#endif
        goto err;
    }
    if (map_mode == CRYP_MAP_AUTO) {
      dev.map_mode = DEV_MAP_AUTO;
      cryp_is_mapped = true;
    } else {
      dev.map_mode = DEV_MAP_VOLUNTARY;
    }
    dev.irq_num = 0;
    dev.gpio_num = 0;

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("registering cryp-user driver\n");
#endif
    int dev_cryp_desc_ = dev_cryp_desc;
    ret = sys_init(INIT_DEVACCESS, &dev, (int*)&dev_cryp_desc_);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_init error!\n");
#endif
        goto err;
    }

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif

    if (!with_dma) {
      goto end;
    }
    dma_in.channel = DMA_CHANNEL_CRYP_IN;
    dma_in.dir = MEMORY_TO_PERIPHERAL;
    dma_in.in_addr = (physaddr_t) 0;
    dma_in.out_addr = (volatile physaddr_t)r_CORTEX_M_CRYP_DIN;
    dma_in.in_prio = DMA_PRI_MEDIUM;
    dma_in.dma = DMA_CRYP;
    dma_in.size = 0;
    dma_in.stream = DMA_STREAM_CRYP_IN;
    dma_in.mode = DMA_DIRECT_MODE;
    dma_in.mem_inc = 1;
    dma_in.dev_inc = 0;
    dma_in.datasize = DMA_DS_WORD;
    dma_in.mem_burst = DMA_BURST_INC4;
    dma_in.dev_burst = DMA_BURST_INC4;
    dma_in.in_handler = (user_dma_handler_t) 0;
    dma_in.out_handler = (user_dma_handler_t) 0;    /* not used */

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP in...\n");
#endif

    ret = sys_init(INIT_DMA, &dma_in, dma_in_desc);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_init error!\n");
#endif
        goto err;
    }

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
    // done by INIT_DONE, by kernel.

    dma_out.channel = DMA_CHANNEL_CRYP_OUT;
    dma_out.dir = PERIPHERAL_TO_MEMORY;
    dma_out.in_addr = (volatile physaddr_t)r_CORTEX_M_CRYP_DOUT;
    dma_out.out_addr = (physaddr_t) 0;
    dma_out.out_prio = DMA_PRI_HIGH;
    dma_out.dma = DMA_CRYP;
    dma_out.size = 0;
    dma_out.stream = DMA_STREAM_CRYP_OUT;
    dma_out.mode = DMA_DIRECT_MODE;
    dma_out.mem_inc = 1;
    dma_out.dev_inc = 0;
    dma_out.datasize = DMA_DS_WORD;
    dma_out.mem_burst = DMA_BURST_INC4;
    dma_out.dev_burst = DMA_BURST_INC4;
    dma_out.in_handler = (user_dma_handler_t) 0;    /* not used */
    dma_out.out_handler = (user_dma_handler_t) 0;

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("init DMA CRYP out...\n");
#endif

    ret = sys_init(INIT_DMA, &dma_out, dma_out_desc);
    if(ret != SYS_E_DONE){
#if CONFIG_USR_DRV_CRYP_DEBUG
        printf("Error: DMA CRYP, sys_init error!\n");
#endif
        goto err;
    }

#if CONFIG_USR_DRV_CRYP_DEBUG
    printf("sys_init returns %s !\n", strerror(ret));
#endif
end:
    return 0;

err:
    return -1;
}
