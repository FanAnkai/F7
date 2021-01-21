#include "ads1120.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ADC_SPI_TIMEOUT 0xFFFF

#define ADC_BUFFER_SIZE 3
uint8_t adc_rx_data[ADC_BUFFER_SIZE];
uint8_t adc_tx_data[ADC_BUFFER_SIZE];
uint8_t stav = 0;
double uref = 1;

extern SPI_HandleTypeDef hspi2;

// setting one register

uint8_t rx_data[100] = {0};


HAL_StatusTypeDef adc_set_reg(uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef eok = HAL_ERROR;
    
    uint8_t config[2];
    config[0] = ADC_CMD_WREG | (reg << 2); // nn + 1
    config[1] = value;
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    eok = HAL_SPI_TransmitReceive(&hspi2, config, rx_data, 2, ADC_SPI_TIMEOUT);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    
    while(eok != HAL_OK);
    return HAL_OK;
}

//setting more register
HAL_StatusTypeDef adc_set_regs(uint8_t start_reg, uint8_t number_reg, uint8_t values[])
{
    uint8_t config[5];
    uint8_t i;
    HAL_StatusTypeDef eok = HAL_ERROR;

    config[0] = ADC_CMD_WREG | (start_reg << 2) | (number_reg - 1); // nn + 1

    for (i = 0; i < number_reg; i++)
    {
        config[i + 1] = values[i];
    }
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    eok = HAL_SPI_TransmitReceive(&hspi2, config, rx_data, number_reg + 1, ADC_SPI_TIMEOUT);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    
    while(eok != HAL_OK);
    
    return HAL_OK;
}

//reset ADC
HAL_StatusTypeDef adc_reset(void)
{
    HAL_StatusTypeDef eok = HAL_ERROR;
    uint8_t config[] = { ADC_CMD_RESET };
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    eok = HAL_SPI_TransmitReceive(&hspi2, config, rx_data, 1, ADC_SPI_TIMEOUT);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    
    while(eok != HAL_OK);
    
    return HAL_OK;
}

//start conversion
HAL_StatusTypeDef adc_start(void)
{
    HAL_StatusTypeDef eok = HAL_ERROR;
    
    uint8_t config[] = { ADC_CMD_START };
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    eok = HAL_SPI_TransmitReceive(&hspi2, config, rx_data, 1, ADC_SPI_TIMEOUT);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    
    while(eok != HAL_OK);
    return HAL_OK;
}

// function for read measurement data
HAL_StatusTypeDef adc_read_data(void)
{
    uint8_t config[] = { ADC_CMD_RDATA };
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_SPI_TransmitReceive(&hspi2, config, rx_data, 1, ADC_SPI_TIMEOUT);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    
    return HAL_OK;
}

//function for conversion impedance to temperature
double adc_res_to_temp(double res)
{

    double a, b;

    a = (res * ADC_PT100_CONST_C7) + ADC_PT100_CONST_C6;
    a = (a * res) + ADC_PT100_CONST_C5;
    a = (a * res) + 1;
    b = (res * ADC_PT100_CONST_C4) + ADC_PT100_CONST_C3;
    b = (b * res) + ADC_PT100_CONST_C2;
    b = (b * res) + ADC_PT100_CONST_C1;
    b = b * res;

    return (b / a) + ADC_PT100_CONST_C0;
}

//function for calculate average temperature from X sample
double adc_average_temp(double temperature)
{
#define ADC_AVERAGE_BUFFER_SIZE 20

    static double average_buffer[ADC_AVERAGE_BUFFER_SIZE];
    static uint8_t average_index = 0;
    static uint8_t average_start = 1;

    uint8_t sum_index;
    double sum;

    if (average_start)
    {
        average_start = 0;
        for (average_index = 0; average_index < ADC_AVERAGE_BUFFER_SIZE; average_index++)
            average_buffer[average_index] = 0;
        average_index = 0;
    }

    average_buffer[average_index++] = temperature;

    if (average_index >= ADC_AVERAGE_BUFFER_SIZE)
        average_index = 0;

    for (sum_index = 0, sum = 0; sum_index < ADC_AVERAGE_BUFFER_SIZE; sum_index++)
        sum += average_buffer[sum_index];

    return sum / ADC_AVERAGE_BUFFER_SIZE;
}

//function for calculate temperature from measurement sample
double adc_calculate_temp(uint8_t msb, uint8_t lsb)
{
#define ADC_MESSAGE_REDUCTION1 20      // number for reduction speed of print temperature
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage = 0, resistance = 0, temperature = 0;
    double t = 0, r = 0, v = 0;
    int16_t sample = (~((((uint16_t) msb) << 8) + lsb) + 1);//13108;# 100 ohm
    message_reduction++;

    if (sample > ADC_LIMIT_MIN && sample < ADC_LIMIT_MAX)       //range for calculate temperature from measurement sample
    {

        resistance = (((ADC_R_REF * ((double)sample)) / ADC_PRECISION) / 4);
        if (resistance < 100.00001 && resistance > 99.995)
        {
            temperature = 0.0000;
            voltage = resistance * ADC_I_REF;
        }
        else
        {
            temperature = adc_res_to_temp(resistance);//adc_average_temp(adc_res_to_temp(resistance));
            voltage = resistance * ADC_I_REF;
        }

        r = resistance;
        t = temperature;
        v = voltage;

        if (message_reduction == ADC_MESSAGE_REDUCTION1)
        {
            message_reduction = 0;
            snprintf(buffer, 80, "\t%4.4f Ohm", r);
            //debug(buffer);
            snprintf(buffer, 80, "\t%+4.4f dC", t);
            //debug(buffer);
            snprintf(buffer, 80, "\t%+4.4f V", v);
            //debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            //debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r\r", (sample));
            //debug(buffer);
        }

    }

    else    // if range is outside setting temperature (-50 to +250 dC)
        // is evaluated short or open circuit
    {
        if (message_reduction == ADC_MESSAGE_REDUCTION1)
        {
            message_reduction = 0;
            //debug("Sample value error: ");
            //debug("open or short circuit\n\r");
        }
    }

    return t;
}


uint16_t sample = 0;
double adc_calculate_internal_temperature(uint8_t msb, uint8_t lsb)
{
#define ADC_MESSAGE_REDUCTION2 10      // number for reduction speed of print temperature
    static uint16_t message_reduction = 0;

    char buffer[80];
    double i_temp;
    double t = 0;
    sample = ((((uint16_t) msb) << 6) + lsb);

    message_reduction++;

    if (sample <= TEMP_BORDER)  //8192    //border between positive and negative temperature here is positive temperature
    {
        i_temp     = ((TEMP_STEP) * ((double) sample));

        t = i_temp;

        if (sample >= 3200) //temperature is high then 100 dC
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                message_reduction = 0;

                snprintf(buffer, 80, "\t Internal temperature =  %+4.4f dC", t);
                //debug(buffer);
                //debug("\tTEMPERATURE IS VERY HIGH!!!\n\r");
            }
        }
        else    //temperature is between 0 - 100 dC
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                message_reduction = 0;

                snprintf(buffer, 80, "\t Internal temperature\t =  %+4.4f dC", t);
                //debug(buffer);
                snprintf(buffer, 80, "\t%+4.0f  sample in DEC", ((double)sample));
                //debug(buffer);
                snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
                //debug(buffer);

            }
        }
    }
    else if ( sample > TEMP_BORDER && sample < TEMP_LOW)//16385 // here is negative temperature
    {
        sample = ((~(sample)) + 1) - TEMP_CORRECT ;
        i_temp     = ((- TEMP_STEP) * ((double) sample));
        t = i_temp;

        if (sample > TEMP_MINUS_SIXTY) // temperature is low then -60 dC
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                message_reduction = 0;

                snprintf(buffer, 80, "\t Internal temperature =  %+4.4f dC", t);
                //debug(buffer);
                //debug("\tTEMPERATURE IS VERY LOW!!!\n\r");
            }
        }
        else
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                message_reduction = 0;

                snprintf(buffer, 80, "\t Internal temperature\t =  %+4.4f dC", t);
                //debug(buffer);
                snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
                //debug(buffer);
                snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
                //debug(buffer);

            }
        }


    }
    else
    {
        if (message_reduction == ADC_MESSAGE_REDUCTION2)
        {
            message_reduction = 0;
            //debug("Sample value error: ");
            //debug("TEMPERATURE ERROR\n\r");
        }
    }

    return t;
}


void adc_buffer_clear(uint8_t buffer[])
{
    for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++)
        buffer[i] = 0;
}

//function for initialization ADC
void adc_init(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 1;
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN0_AIN1 | ADC_REG0_GAIN4 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_EXTERNAL_REFP0_REFN0 | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();

}

void adc_init_extref(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 2;

    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN2_AVSS | ADC_REG0_GAIN4 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_EXTERNAL_REFP0_REFN0 | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();

}

void adc_init_ain1(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 3;

    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN1_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();
}

void adc_init_ain0(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 4;

    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN0_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();
}

void adc_init_unap(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 6;

    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_MODE_AVDD_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();
}


void adc_init_uref(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 7;

    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_MODE_VREF | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_EXTERNAL_REFP0_REFN0 | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();
}

void adc_init_internal_temperature(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 5;

    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN2_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_ENABLE | ADC_REG1_BCS_OFF,    //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u,             //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED,    //0x80  // 1000 0000
    };

    adc_reset();
    adc_set_regs(ADC_REG0, 4, config);
    adc_start();
}

//function for receive sample from ADC
void adc_get_sample(void)
{
    adc_init_internal_temperature();
    
    memset(adc_tx_data, 0xff, sizeof(adc_tx_data));
    //debug("SPI start receive\n\r");
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_SPI_TransmitReceive(&hspi2, adc_tx_data, adc_rx_data, 3, ADC_SPI_TIMEOUT);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    
    if (stav == 5)
    {
        adc_calculate_internal_temperature(adc_rx_data[0], adc_rx_data[1]);
    }



}
