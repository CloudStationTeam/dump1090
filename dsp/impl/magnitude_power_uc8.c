#include <math.h>
#include <endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <inttypes.h>

#include "dsp/helpers/tables.h"

/* Convert UC8 values to unsigned 16-bit magnitudes */

void STARCH_IMPL(magnitude_power_uc8, twopass) (const uc8_t *in, uint16_t *out, unsigned len, double *out_level, double *out_power)
{
#if STARCH_ALIGNMENT > 1
    starch_magnitude_uc8_aligned(in, out, len);
    starch_mean_power_u16_aligned(out, len, out_level, out_power);
#else
    starch_magnitude_uc8(in, out, len);
    starch_mean_power_u16(out, len, out_level, out_power);
#endif
}

void STARCH_IMPL(magnitude_power_uc8, lookup) (const uc8_t *in, uint16_t *out, unsigned len, double *out_level, double *out_power)
{
    const uint16_t * const restrict mag_table = get_uc8_mag_table();

    const uc8_u16_t * restrict in_align = (const uc8_u16_t *) STARCH_ALIGNED(in);
    uint16_t * restrict out_align = STARCH_ALIGNED(out);

    uint64_t sum_level = 0;
    uint64_t sum_power = 0;

    unsigned len1 = len;
    while (len1--) {
        uint16_t mag = mag_table[in_align[0].u16];
        out_align[0] = mag;
        sum_level += mag;
        sum_power += (uint32_t)mag * mag;

        out_align += 1;
        in_align += 1;
    }

    *out_level = sum_level / 65536.0 / len;
    *out_power = sum_power / 65536.0 / 65536.0 / len;
}

void STARCH_IMPL(magnitude_power_uc8, lookup_unroll_4) (const uc8_t *in, uint16_t *out, unsigned len, double *out_level, double *out_power)
{
    const uint16_t * const restrict mag_table = get_uc8_mag_table();

    const uc8_u16_t * restrict in_align = (const uc8_u16_t *) STARCH_ALIGNED(in);
    uint16_t * restrict out_align = STARCH_ALIGNED(out);

    uint64_t sum_level = 0;
    uint64_t sum_power = 0;

    unsigned len4 = len >> 2;
    unsigned len1 = len & 3;

    while (len4--) {
        uint16_t mag0 = mag_table[in_align[0].u16];
        uint16_t mag1 = mag_table[in_align[1].u16];
        uint16_t mag2 = mag_table[in_align[2].u16];
        uint16_t mag3 = mag_table[in_align[3].u16];

        out_align[0] = mag0;
        out_align[1] = mag1;
        out_align[2] = mag2;
        out_align[3] = mag3;

        sum_level = sum_level + mag0 + mag1 + mag2 + mag3;
        sum_power = sum_power + (uint32_t)mag0 * mag0 + (uint32_t)mag1 * mag1 + (uint32_t)mag2 * mag2 + (uint32_t)mag3 * mag3;

        out_align += 4;
        in_align += 4;
    }

    while (len1--) {
        uint16_t mag = mag_table[in_align[0].u16];

        out_align[0] = mag;

        sum_level = sum_level + mag;
        sum_power = sum_power + mag * mag;

        out_align += 1;
        in_align += 1;
    }

    *out_level = sum_level / 65536.0 / len;
    *out_power = sum_power / 65536.0 / 65536.0 / len;
}
