
// gcc -O2 -c ads1115.c -Wall
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "../ads1115/ads1115.h"
#include "../TSL2561/tsl2561.h"
#include "../lsm9ds1/lsm9ds1.h"

volatile done = 0;
void handler(int signum)
{
    done = 1;
}

int main()
{
    char command;

    struct sigaction action;
    action.sa_handler = handler;
    sigaction(SIGINT, &action, NULL);

    ads1115 *adc = (ads1115 *)malloc(sizeof(ads1115));
    tsl2561 *css = (tsl2561 *)malloc(sizeof(tsl2561));
    tsl2561 *css1 = (tsl2561 *)malloc(sizeof(tsl2561));
    lsm9ds1 *mag = (lsm9ds1 *)malloc(sizeof(lsm9ds1));

    snprintf(mag->fname, 40, "/dev/i2c-1");

    int adc_stat = 0;
    int css_stat = 0;
    int mag_stat = 0;

    uint16_t adc_config_reg_data;
    int16_t *adc_conv_reg_data;
    ads1115_config adc_conf;

    uint8_t css_config_reg_data;

    adc_conv_reg_data = (int16_t *)(malloc(4 * sizeof(int16_t)));

    // Initialize devices
    adc_stat = ads1115_init(adc, ADS1115_S_ADDR);
    css_stat = tsl2561_init(css, TSL2561_ADDR_LOW);
    css_stat = tsl2561_init(css1, TSL2561_ADDR_FLOAT);
    mag_stat = lsm9ds1_init(mag, 0x6b, 0x1e);
    printf("*************************ADS1115*************************\n");
    // 1A) [READ] *original* contents from configuration register
    adc_stat = ads1115_read_config(adc, &adc_config_reg_data);

    // 1B) [PRINT] *original* contents of configuration register
    if (adc_stat)
    {
        adc_conf.raw = adc_config_reg_data;
        printf("ADS1115 CONFIG REGISTER - ORIGINAL CONTENTS\n");
        printf("RAW CONTENTS: [%04X]\n", adc_conf.raw);
    }
    else
    {
        perror("[ERROR] Could not read from configuration register.");
    }

    // 2A) [WRITE] *target* settings to configuration register

    adc_conf.raw = 0x0000;

    adc_conf.os = 0;
    adc_conf.mux = 0;
    adc_conf.pga = 1;
    adc_conf.mode = 1;
    adc_conf.dr = 7;
    adc_conf.comp_mode = 0;
    adc_conf.comp_pol = 0;
    adc_conf.comp_lat = 0;
    adc_conf.comp_que = 3;

    adc_stat = ads1115_configure(adc, adc_conf);

    // 2B) [PRINT] *target* settings for configuration register
    if (adc_stat)
    {
        printf("SET ADS1115 CONFIGURATION REGISTER\n");
        printf("RAW CONTENTS: [%04X]\n", adc_conf.raw);
    }
    else
    {
        perror("[ERROR] Could not write to configuration register.");
    }

    // 3A) [READ] *updated* contents from configuration register
    adc_stat = ads1115_read_config(adc, &adc_config_reg_data);

    // 3B) [PRINT] *contents* of configuration register
    if (adc_stat)
    {
        adc_conf.raw = adc_config_reg_data;
        printf("ADS1115 CONFIG REGISTER - UPDATED CONTENTS\n");
        printf("RAW CONTENTS: [%04X]\n", adc_conf.raw);
    }
    else
    {
        perror("[ERROR] Could not read from configuration register.");
    }
    printf("*************************ADS1115*************************\n");
    /******************************************************************************/
    /******************************************************************************/
    printf("\n*************************TSL2561*************************\n");

    // css_stat = tsl2561_read_config(css, &css_config_reg_data);

    // if (css_stat)
    // {
    //     printf("TSL2561 TIMING REGISTER - ORIGINAL CONTENTS\n");
    //     printf("RAW CONTENTS: [%02X]\n", css_config_reg_data);
    // }
    // else
    // {
    //     perror("[ERROR] Could not read from timing register.");
    // }

    css_stat = tsl2561_configure(css);
    css_stat = tsl2561_configure(css1);
    printf("\n*************************TSL2561*************************\n");
    // printf("\n*************************LSM9DS1*************************\n");
    // MAG_DATA_RATE drate ;
    // drate.data_rate = 0b101 ;
    // drate.fast_odr = 0 ;
    // drate.operative_mode = 0b11 ;
    // drate.temp_comp = 1 ;
    // MAG_RESET rst ;
    // rst.full_scale = 0b00 ;
    // rst.reboot = 0 ;
    // rst.soft_rst = 0 ;
    // MAG_DATA_READ dread ;
    // dread.bdu = 0 ;
    // dread.fast_read = 0 ;
    // mag_stat = lsm9ds1_config_mag(mag, drate, rst, dread) ;
    // printf("\n*************************LSM9DS1*************************\n");
    char c;
    do
    {
        printf("Press enter key to continue...");
        scanf("%c", &c);
    } while (c != '\n');

    while (!done)
    {
        adc_stat = ads1115_read_data(adc, adc_conv_reg_data);

        // 4B) [PRINT] data from conversion register
        if (adc_stat)
        {
            for (int i = 0; i < 4; i++)
            {
                printf("ADC CHN [%d]: DATA = [%04X]\n", i, adc_conv_reg_data[i]);
            }
        }

        // uint8_t data[4];
        // int css_stat = tsl2561_read_i2c_data(css, data);
        // for (int ii = 0; ii < 2; ii++)
        // {
        //     printf("CSS CHN [%d]: DATA = [%04X]\n", ii, ((uint16_t *)data)[ii]);
        // }
        printf("Lux 0x29: %08d\n", tsl2561_get_lux(css));
        printf("Lux 0x39: %08d\n", tsl2561_get_lux(css1));
        short magData[3];
        lsm9ds1_read_mag(mag, magData);
        printf("Bx: %d By: %d Bz: %d\n", magData[0], magData[1], magData[2]);
        usleep(14000);
    }
    printf("Freeing...\n");
    ads1115_destroy(adc);
    tsl2561_destroy(css);
    tsl2561_destroy(css1);
    lsm9ds1_destroy(mag);
    return 0;
}