/*
 * Stage 1 measurement instrument. Samples the SAADC channels from app.overlay
 * at 1 kHz and answers line commands over USB CDC ACM. README.md documents
 * the protocol.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>
#include <hal/nrf_power.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define RATE_HZ                 1000
#define PERIOD_US               (USEC_PER_SEC / RATE_HZ)
#define NOISE_SAMPLES           1000
#define STREAM_MAX_MS           (UINT32_MAX / RATE_HZ)
#define CMD_LINE_MAX            32
#define UF2_BOOTLOADER_GPREGRET 0x57

struct channel {
    const char *name;
    struct adc_dt_spec spec;
};

struct command {
    const char *name;
    void (*run)(const char *arg);
};

struct sample_run {
    uint32_t n;
    uint32_t late;
    int err;
};

struct noise_acc {
    int64_t sum;
    int64_t sum_sq;
    int16_t min;
    int16_t max;
};

typedef void (*sample_sink)(void *ctx, uint32_t idx, int16_t raw);

static const struct channel channels[] = {
        {"ain0", ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ain0)},
        {"vdd", ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), vdd)},
};

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static int setup_err;
static int cal_err;

/*
 * Polls one input byte. Returns 0 with the byte in c, or -1 when none is
 * waiting. An LF right after a CR belongs to that CR's line terminator, so it
 * is consumed here and neither ends an empty line nor stops a sampler.
 */
static int console_read(unsigned char *c)
{
    static bool after_cr;
    bool paired_lf;

    do {
        if (uart_poll_in(console, c) != 0) {
            return -1;
        }
        paired_lf = after_cr && *c == '\n';
        after_cr = *c == '\r';
    } while (paired_lf);
    return 0;
}

static const struct channel *find_channel(const char *name)
{
    for (size_t i = 0; i < ARRAY_SIZE(channels); i++) {
        if (strcmp(channels[i].name, name) == 0) {
            return &channels[i];
        }
    }
    return NULL;
}

/*
 * Reads spec once per PERIOD_US on an absolute schedule, so a slow sink delays
 * one sample instead of shifting every later one. Stops after count samples
 * (0 means no limit) or at the first input byte, which it consumes. A sample
 * is late when it starts a full period or more after its slot.
 */
static struct sample_run sample_at_rate(const struct adc_dt_spec *spec, uint32_t count,
                                        sample_sink sink, void *ctx)
{
    int16_t raw;
    struct adc_sequence seq = {.buffer = &raw, .buffer_size = sizeof(raw)};
    struct sample_run run = {0};
    unsigned char c;
    int64_t slot_us;

    run.err = adc_sequence_init_dt(spec, &seq);
    if (run.err) {
        return run;
    }
    slot_us = k_ticks_to_us_floor64(k_uptime_ticks());
    while ((count == 0 || run.n < count) && console_read(&c) != 0) {
        k_sleep(K_TIMEOUT_ABS_US(slot_us));
        if (k_ticks_to_us_floor64(k_uptime_ticks()) >= slot_us + PERIOD_US) {
            run.late++;
        }
        run.err = adc_read_dt(spec, &seq);
        if (run.err) {
            return run;
        }
        sink(ctx, run.n, raw);
        run.n++;
        slot_us += PERIOD_US;
    }
    return run;
}

static int32_t fullscale_mv(const struct adc_dt_spec *spec)
{
    int32_t mv = BIT(spec->resolution);

    adc_raw_to_millivolts_dt(spec, &mv);
    return mv;
}

static void noise_add(void *ctx, uint32_t idx, int16_t raw)
{
    struct noise_acc *acc = ctx;

    ARG_UNUSED(idx);
    acc->sum += raw;
    acc->sum_sq += (int64_t)raw * raw;
    acc->min = MIN(acc->min, raw);
    acc->max = MAX(acc->max, raw);
}

static void stream_print(void *ctx, uint32_t idx, int16_t raw)
{
    ARG_UNUSED(ctx);
    printk("S,%u,%d\n", idx, raw);
}

static void cmd_info(const char *arg)
{
    ARG_UNUSED(arg);
    for (size_t i = 0; i < ARRAY_SIZE(channels); i++) {
        const struct adc_dt_spec *spec = &channels[i].spec;

        printk("CHANNEL,name=%s,resolution=%u,oversampling=%u,acq_us=%u,fullscale_mv=%d\n",
               channels[i].name, spec->resolution, 1U << spec->oversampling,
               (unsigned int)ADC_ACQ_TIME_VALUE(spec->channel_cfg.acquisition_time),
               fullscale_mv(spec));
    }
    printk("INFO,app=stage1-adc,rate_hz=%d,noise_n=%d,setup_err=%d,cal_err=%d\n", RATE_HZ,
           NOISE_SAMPLES, setup_err, cal_err);
}

/*
 * Sigma uses n - 1 and exact integer sums: 1000 samples of 12 bits keep
 * n * sum_sq below 2^45.
 */
static void cmd_noise(const char *arg)
{
    const struct channel *ch = find_channel(arg[0] ? arg : "ain0");
    struct noise_acc acc = {.min = INT16_MAX, .max = INT16_MIN};
    struct sample_run run;

    if (ch == NULL) {
        printk("ERR,unknown channel %s\n", arg);
        return;
    }
    run = sample_at_rate(&ch->spec, NOISE_SAMPLES, noise_add, &acc);
    if (run.err) {
        printk("ERR,adc read failed %d\n", run.err);
        return;
    }
    if (run.n < NOISE_SAMPLES) {
        printk("ERR,noise interrupted after %u samples\n", run.n);
        return;
    }

    double n = run.n;
    double mean = acc.sum / n;
    double var = (n * acc.sum_sq - (double)acc.sum * acc.sum) / (n * (n - 1));

    printk("NOISE,channel=%s,n=%u,mean=%.3f,sigma=%.3f,min=%d,max=%d,mean_mv=%.1f,late=%u\n",
           ch->name, run.n, mean, sqrt(var), acc.min, acc.max,
           mean * fullscale_mv(&ch->spec) / BIT(ch->spec.resolution), run.late);
}

/*
 * No argument streams until the next input byte. A given duration must be a
 * whole number of milliseconds from 1 to STREAM_MAX_MS.
 */
static void cmd_stream(const char *arg)
{
    char *end;
    unsigned long ms = strtoul(arg, &end, 10);
    uint32_t count = 0;
    const struct channel *ch = &channels[0];
    struct sample_run run;

    if (arg[0] != '\0') {
        if (!isdigit((unsigned char)arg[0]) || *end != '\0' || ms == 0 || ms > STREAM_MAX_MS) {
            printk("ERR,bad duration %s; use stream [ms] with ms from 1 to %u\n", arg,
                   (unsigned int)STREAM_MAX_MS);
            return;
        }
        count = ms * RATE_HZ / MSEC_PER_SEC;
    }
    printk("STREAM_START,channel=%s,rate_hz=%d,n=%u\n", ch->name, RATE_HZ, count);
    run = sample_at_rate(&ch->spec, count, stream_print, NULL);
    if (run.err) {
        printk("ERR,adc read failed %d\n", run.err);
    }
    printk("STREAM_END,n=%u,late=%u\n", run.n, run.late);
}

static void cmd_bootloader(const char *arg)
{
    ARG_UNUSED(arg);
    printk("BOOTLOADER\n");
    k_msleep(100);
    nrf_power_gpregret_set(NRF_POWER, 0, UF2_BOOTLOADER_GPREGRET);
    sys_reboot(SYS_REBOOT_COLD);
}

static const struct command commands[] = {
        {"info", cmd_info},
        {"noise", cmd_noise},
        {"stream", cmd_stream},
        {"b", cmd_bootloader},
};

static void dispatch(char *line)
{
    char *arg = strchr(line, ' ');

    if (arg != NULL) {
        *arg++ = '\0';
    } else {
        arg = line + strlen(line);
    }
    for (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
        if (strcmp(commands[i].name, line) == 0) {
            commands[i].run(arg);
            return;
        }
    }
    printk("ERR,unknown command %s; use info, noise [ain0|vdd], stream [ms], b\n", line);
}

/*
 * Blocks until a line ends in CR or LF. Returns its length, 0 for an empty
 * line, or -1 when it overflowed buf and was dropped.
 */
static int read_line(char *buf, size_t size)
{
    size_t len = 0;
    bool overflow = false;
    unsigned char c;

    for (;;) {
        if (console_read(&c) != 0) {
            k_msleep(5);
            continue;
        }
        if (c == '\r' || c == '\n') {
            buf[len] = '\0';
            return overflow ? -1 : (int)len;
        }
        if (len < size - 1) {
            buf[len++] = c;
        } else {
            overflow = true;
        }
    }
}

/*
 * Offset calibration once at boot (nRF52840 errata 86). The device_next CDC
 * ACM driver arms its OUT endpoint only once RX is enabled, so without
 * uart_irq_rx_enable() no command ever arrives.
 */
int main(void)
{
    char line[CMD_LINE_MAX];
    int16_t raw;
    struct adc_sequence cal = {.buffer = &raw, .buffer_size = sizeof(raw), .calibrate = true};
    int len;

    for (size_t i = 0; i < ARRAY_SIZE(channels) && !setup_err; i++) {
        setup_err = adc_channel_setup_dt(&channels[i].spec);
    }
    cal_err = adc_sequence_init_dt(&channels[0].spec, &cal);
    if (!cal_err) {
        cal_err = adc_read_dt(&channels[0].spec, &cal);
    }
    uart_irq_rx_enable(console);

    for (;;) {
        len = read_line(line, sizeof(line));
        if (len < 0) {
            printk("ERR,line longer than %d bytes\n", CMD_LINE_MAX - 1);
        } else if (len > 0) {
            dispatch(line);
        }
    }
    return 0;
}
