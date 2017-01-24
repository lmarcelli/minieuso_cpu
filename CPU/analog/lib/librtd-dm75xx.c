/**
    @file

    @brief
        DM75xx user library source code

    @verbatim
    --------------------------------------------------------------------------
    This file and its contents are copyright (C) RTD Embedded Technologies,
    Inc.  All Rights Reserved.

    This software is licensed as described in the RTD End-User Software License
    Agreement.  For a copy of this agreement, refer to the file LICENSE.TXT
    (which should be included with this software) or contact RTD Embedded
    Technologies, Inc.
    --------------------------------------------------------------------------
    @endverbatim

    $Id: librtd-dm75xx.c 99118 2016-04-28 12:54:37Z rgroner $
*/

#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dm75xx_ioctl.h"
#include "dm75xx_library.h"
#include "dm75xx_registers.h"

/**
 * @defgroup DM75xx_Library_Source DM75xx user library source code
 * @{
 */
/*=============================================================================
Private functions
 =============================================================================*/
/**
 * @defgroup DM75xx_Library_Private_Functions DM75xx user library source code private functions
 * @{
 */

 /**
  * @internal
  */
/**
*******************************************************************************
@brief
    Validate a DSP command

@param
    command

    The DSP Command to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      request is not valid

@note
    This function will consider DMA channel 0 and 1 interrupt sources as invalid
    as these sources are enabled by default.
 *******************************************************************************
 */
int validate_dsp_command(dm75xx_dsp_command_t command)
{
	switch (command) {
	case DM75xx_DSP_CAL_AUTO:
	case DM75xx_DSP_FLASH_DOWNLOAD:
	case DM75xx_DSP_USER_RUN:
	case DM75xx_DSP_USER_UPGRADE:
	case DM75xx_DSP_INT_FLASH_ERASE:
	case DM75xx_DSP_EXT_FLASH_ERASE:
	case DM75xx_DSP_ATTENTION:
	case DM75xx_DSP_CAL_DEFAULT:
	case DM75xx_DSP_CAL_VERSION:
	case DM75xx_DSP_BOOT_VERSION:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the interrupt source.

@param
    source

    The interrupt source to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      request is not valid

@note
    This function will consider DMA channel 0 and 1 interrupt sources as invalid
    as these sources are enabled by default.
 *******************************************************************************
 */
int validate_interrupt_source(dm75xx_int_source_t source)
{
	switch (source) {
	case DM75xx_INT_FIFO_WRITE:
	case DM75xx_INT_CGT_RESET:
	case DM75xx_INT_RESERVED:
	case DM75xx_INT_CGT_PAUSE:
	case DM75xx_INT_ABOUT:
	case DM75xx_INT_DELAY:
	case DM75xx_INT_SCNT_ADC:
	case DM75xx_INT_SCNT_DAC1:
	case DM75xx_INT_SCNT_DAC2:
	case DM75xx_INT_UTC1:
	case DM75xx_INT_UTC1_INV:
	case DM75xx_INT_UTC2:
	case DM75xx_INT_DIO:
	case DM75xx_INT_EXTERNAL:
	case DM75xx_INT_ETRIG_RISING:
	case DM75xx_INT_ETRIG_FALLING:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the DMA demand mode request source.

@param
    request

    The request source to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      request is not valid
 *******************************************************************************
 */
int validate_dma_request(dm75xx_dma_request_t request)
{
	switch (request) {
	case DM75xx_DMA_DEMAND_DISABLE:
	case DM75xx_DMA_DEMAND_UTC1:
	case DM75xx_DMA_DEMAND_FIFO_ADC:
	case DM75xx_DMA_DEMAND_FIFO_DAC1:
	case DM75xx_DMA_DEMAND_FIFO_DAC2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the DMA local source.

@param
    source

    The source to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      source is not valid
 *******************************************************************************
 */
int validate_dma_source(dm75xx_dma_source_t source)
{
	switch (source) {
	case DM75xx_DMA_FIFO_ADC:
	case DM75xx_DMA_FIFO_DAC1:
	case DM75xx_DMA_FIFO_DAC2:
	case DM75xx_DMA_FIFO_HSDIN:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the DMA channel.

@param
    channel

    The channel to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      channel is not valid
 *******************************************************************************
 */
int validate_dma_channel(dm75xx_dma_channel_t channel)
{
	switch (channel) {
	case DM75xx_DMA_CHANNEL_0:
	case DM75xx_DMA_CHANNEL_1:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the user output selection.

@param
    select

    The user output selection to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      selection is not valid
 *******************************************************************************
 */
int validate_uio_select(dm75xx_uio_source_t source)
{
	switch (source) {
	case DM75xx_UIO_ADC:
	case DM75xx_UIO_DAC1:
	case DM75xx_UIO_DAC2:
	case DM75xx_UIO_PRG:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the external trigger polarity.

@param
    polarity

    The polarity to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      polarity is not valid
 *******************************************************************************
 */
int validate_ext_polarity(dm75xx_ext_polarity_t polarity)
{
	switch (polarity) {
	case DM75xx_EXT_POLARITY_POS:
	case DM75xx_EXT_POLARITY_NEG:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the syncbus source.

@param
    src

    The syncbus source to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      source is not valid
 *******************************************************************************
 */
int validate_sbus_source(dm75xx_sbus_src_t src)
{
	switch (src) {
	case DM75xx_SBUS_SRC_SOFT_ADC:
	case DM75xx_SBUS_SRC_PCLK:
	case DM75xx_SBUS_SRC_BCLK:
	case DM75xx_SBUS_SRC_DIG_IT:
	case DM75xx_SBUS_SRC_ETRIG:
	case DM75xx_SBUS_SRC_DAC_UPDATE:
	case DM75xx_SBUS_SRC_DAC_CLK:
	case DM75xx_SBUS_SRC_UTC2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the highspeed digital input signal.

@param
    signal

    The signal to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      signal is not valid
 *******************************************************************************
 */
int validate_hsdin_signal(dm75xx_hsdin_signal_t signal)
{
	switch (signal) {
	case DM75xx_HSDIN_SIGNAL_SOFTWARE:
	case DM75xx_HSDIN_SIGNAL_ADC:
	case DM75xx_HSDIN_SIGNAL_UTC0:
	case DM75xx_HSDIN_SIGNAL_UTC1:
	case DM75xx_HSDIN_SIGNAL_UTC2:
	case DM75xx_HSDIN_SIGNAL_EPCLK:
	case DM75xx_HSDIN_SIGNAL_ETRIG:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the burst clock primary frequency.

@param
    freq

    The frequency to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      frequency is not valid
 *******************************************************************************
 */
int validate_bclk_freq(dm75xx_bclk_freq_t freq)
{
	switch (freq) {
	case DM75xx_BCLK_FREQ_8_MHZ:
	case DM75xx_BCLK_FREQ_20_MHZ:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the burst clock start trigger.

@param
    start

    The start trigger to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      start trigger is not valid
 *******************************************************************************
 */
int validate_bclk_start(dm75xx_bclk_start_t start)
{
	switch (start) {
	case DM75xx_BCLK_START_SOFTWARE:
	case DM75xx_BCLK_START_PACER:
	case DM75xx_BCLK_START_ETRIG:
	case DM75xx_BCLK_START_DIG_IT:
	case DM75xx_BCLK_START_SBUS0:
	case DM75xx_BCLK_START_SBUS1:
	case DM75xx_BCLK_START_SBUS2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog primary frequency.

@param
    freq

    The frequency to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      frequency is not valid
 *******************************************************************************
 */
int validate_dac_freq(dm75xx_dac_freq_t freq)
{
	switch (freq) {
	case DM75xx_DAC_FREQ_8_MHZ:
	case DM75xx_DAC_FREQ_20_MHZ:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog channel.

@param
    dac

    The dac to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      dac is not valid
 *******************************************************************************
 */
int validate_dac_channel(dm75xx_dac_channel_t dac)
{
	switch (dac) {
	case DM75xx_DAC1:
	case DM75xx_DAC2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog clock mode.

@param
    clk_mode

    The mode to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      clk_mode is not valid
 *******************************************************************************
 */
int validate_dac_clk_mode(dm75xx_dac_clk_mode_t clk_mode)
{
	switch (clk_mode) {
	case DM75xx_DAC_CLK_FREE_RUN:
	case DM75xx_DAC_CLK_START_STOP:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog clock stop trigger.

@param
    dac_clk_stop

    The digital to analog clock stop trigger to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      dac_clk_stop is not valid
 *******************************************************************************
 */
int validate_dac_clk_stop(dm75xx_dac_clk_stop_t dac_clk_stop)
{
	switch (dac_clk_stop) {
	case DM75xx_DAC_CLK_STOP_SOFTWARE_PACER:
	case DM75xx_DAC_CLK_STOP_ETRIG:
	case DM75xx_DAC_CLK_STOP_DIG_IT:
	case DM75xx_DAC_CLK_STOP_UTC2:
	case DM75xx_DAC_CLK_STOP_SBUS0:
	case DM75xx_DAC_CLK_STOP_SBUS1:
	case DM75xx_DAC_CLK_STOP_SBUS2:
	case DM75xx_DAC_CLK_STOP_SOFTWARE:
	case DM75xx_DAC_CLK_STOP_DAC1_UCNT:
	case DM75xx_DAC_CLK_STOP_DAC2_UCNT:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog clock start trigger.

@param
    dac_clk_start

    The digital to analog clock start trigger to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      dac_clk_start is not valid
 *******************************************************************************
 */
int validate_dac_clk_start(dm75xx_dac_clk_start_t dac_clk_start)
{
	switch (dac_clk_start) {
	case DM75xx_DAC_CLK_START_SOFTWARE_PACER:
	case DM75xx_DAC_CLK_START_ETRIG:
	case DM75xx_DAC_CLK_START_DIG_IT:
	case DM75xx_DAC_CLK_START_UTC2:
	case DM75xx_DAC_CLK_START_SBUS0:
	case DM75xx_DAC_CLK_START_SBUS1:
	case DM75xx_DAC_CLK_START_SBUS2:
	case DM75xx_DAC_CLK_START_SOFTWARE:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog mode.

@param
    dac_mode

    The mode to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      dac_mode is not valid
 *******************************************************************************
 */
int validate_dac_mode(dm75xx_dac_channel_t dac, dm75xx_dac_mode_t dac_mode)
{
	if (validate_dac_channel(dac)) {
		return -1;
	}
	switch (dac_mode) {
	case DM75xx_DAC_MODE_NOT_CYCLE:
	case DM75xx_DAC_MODE_CYCLE:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog update source.

@param
    dac_update_src

    The digital to analog update source to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      dac_update_src is not valid
 *******************************************************************************
 */
int validate_dac_update_source(dm75xx_dac_update_src_t dac_update_src)
{

	switch (dac_update_src) {
	case DM75xx_DAC_UPDATE_SOFTWARE:
	case DM75xx_DAC_UPDATE_CGT:
	case DM75xx_DAC_UPDATE_CLOCK:
	case DM75xx_DAC_UPDATE_EPCLK:
	case DM75xx_DAC_UPDATE_SBUS0:
	case DM75xx_DAC_UPDATE_SBUS1:
	case DM75xx_DAC_UPDATE_SBUS2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the digital to analog range.

@param
    dac_range

    The digital to analog range to validate.

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      dac_range is not valid
 *******************************************************************************
 */
int validate_dac_range(dm75xx_dac_range_t dac_range)
{
	switch (dac_range) {
	case DM75xx_DAC_RANGE_UNIPOLAR_5:
	case DM75xx_DAC_RANGE_UNIPOLAR_10:
	case DM75xx_DAC_RANGE_BIPOLAR_5:
	case DM75xx_DAC_RANGE_BIPOLAR_10:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the analog to digital sample counter source.

@param
    src

    The analog to digital sample counter source to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      src is not valid
 *******************************************************************************
 */
int validate_adc_scnt_src(dm75xx_adc_scnt_src_t src)
{
	switch (src) {
	case DM75xx_ADC_SCNT_SRC_CGT:
	case DM75xx_ADC_SCNT_SRC_FIFO:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the analog to digital conversion signal.

@param
    adc_conv_signal

    The analog to digital conversion signal to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      adc_conv_signal is not valid
 *******************************************************************************
 */
int validate_adc_conv_signal(dm75xx_adc_conv_signal_t adc_conv_signal)
{
	switch (adc_conv_signal) {
	case DM75xx_ADC_CONV_SIGNAL_SOFTWARE:
	case DM75xx_ADC_CONV_SIGNAL_PCLK:
	case DM75xx_ADC_CONV_SIGNAL_BCLK:
	case DM75xx_ADC_CONV_SIGNAL_DIG_IT:
	case DM75xx_ADC_CONV_SIGNAL_DAC1_MRKR1:
	case DM75xx_ADC_CONV_SIGNAL_DAC2_MRKR1:
	case DM75xx_ADC_CONV_SIGNAL_SBUS0:
	case DM75xx_ADC_CONV_SIGNAL_SBUS1:
	case DM75xx_ADC_CONV_SIGNAL_SBUS2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the pacer clock mode.

@param
    pclk_mode

    The pacer clock mode to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      pclk_mode is not valid
 *******************************************************************************
 */
int validate_pclk_mode(dm75xx_pclk_mode_t pclk_mode)
{
	switch (pclk_mode) {
	case DM75xx_PCLK_NO_REPEAT:
	case DM75xx_PCLK_REPEAT:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the pacer clock stop trigger.

@param
    pclk_stop

    The pacer clock stop trigger to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      pclk_stop is not valid
 *******************************************************************************
 */
int validate_pclk_stop(dm75xx_pclk_stop_t pclk_stop)
{
	switch (pclk_stop) {
	case DM75xx_PCLK_STOP_SOFTWARE:
	case DM75xx_PCLK_STOP_ETRIG:
	case DM75xx_PCLK_STOP_DIGITAL_IT:
	case DM75xx_PCLK_STOP_ACNT:
	case DM75xx_PCLK_STOP_UTC2:
	case DM75xx_PCLK_STOP_SBUS0:
	case DM75xx_PCLK_STOP_SBUS1:
	case DM75xx_PCLK_STOP_SBUS2:
	case DM75xx_PCLK_STOP_ASOFTWARE:
	case DM75xx_PCLK_STOP_AETRIG:
	case DM75xx_PCLK_STOP_ADIGITAL_IT:
	case DM75xx_PCLK_STOP_RES:
	case DM75xx_PCLK_STOP_AUTC2:
	case DM75xx_PCLK_STOP_ASBUS0:
	case DM75xx_PCLK_STOP_ASBUS1:
	case DM75xx_PCLK_STOP_ASBUS2:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the pacer clock start trigger.

@param
    pclk_start

    The pacer clock start trigger to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      pclk_start is not valid
 *******************************************************************************
 */
int validate_pclk_start(dm75xx_pclk_start_t pclk_start)
{
	switch (pclk_start) {
	case DM75xx_PCLK_START_SOFTWARE:
	case DM75xx_PCLK_START_ETRIG:
	case DM75xx_PCLK_START_DIGITAL_IT:
	case DM75xx_PCLK_START_UTC2:
	case DM75xx_PCLK_START_SBUS0:
	case DM75xx_PCLK_START_SBUS1:
	case DM75xx_PCLK_START_SBUS2:
	case DM75xx_PCLK_START_RES:
	case DM75xx_PCLK_START_DSOFTWARE:
	case DM75xx_PCLK_START_DETRIG:
	case DM75xx_PCLK_START_DDIGITAL_IT:
	case DM75xx_PCLK_START_DUTC2:
	case DM75xx_PCLK_START_DSBUS0:
	case DM75xx_PCLK_START_DSBUS1:
	case DM75xx_PCLK_START_DSBUS2:
	case DM75xx_PCLK_START_ETRIG_GATE:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the pacer clock select.

@param
    pclk_select

    The pacer clock selection to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      pclk_select is not valid
 *******************************************************************************
 */
int validate_pclk_select(dm75xx_pclk_select_t pclk_select)
{
	switch (pclk_select) {
	case DM75xx_PCLK_EXTERNAL:
	case DM75xx_PCLK_INTERNAL:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the pacer clock primary frequency.

@param
    freq_select

    The pacer clock primary frequency to validate

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      freq_select is not valid
 *******************************************************************************
 */
int validate_pclk_freq(dm75xx_pclk_freq_t freq_select)
{
	switch (freq_select) {
	case DM75xx_PCLK_FREQ_8_MHZ:
	case DM75xx_PCLK_FREQ_20_MHZ:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the user timer/counter clock source.

@param
    utc_select

    The user timer/counter selected.

@param
    source

    The user timer/counter clock source to validate.

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      utc_select is not valid

        @arg \c
            EINVAL      source is not valid
 *******************************************************************************
 */
int validate_utc_clock_source(dm75xx_utc_timer_t utc_select,
			      dm75xx_utc_clk_t source)
{
	switch (utc_select) {
	case DM75xx_UTC_0:
		switch (source) {
		case DM75xx_CUTC_8_MHZ:
		case DM75xx_CUTC_EXT_TC_CLOCK_1:
		case DM75xx_CUTC_EXT_TC_CLOCK_2:
		case DM75xx_CUTC_EXT_PCLK:
			break;
		default:
			errno = EINVAL;
			return -1;
			break;
		}
		break;
	case DM75xx_UTC_1:
		switch (source) {
		case DM75xx_CUTC_8_MHZ:
		case DM75xx_CUTC_EXT_TC_CLOCK_1:
		case DM75xx_CUTC_EXT_TC_CLOCK_2:
		case DM75xx_CUTC_EXT_PCLK:
		case DM75xx_CUTC_UTC_0_OUT:
		case DM75xx_CUTC_HSDIN_SIGNAL:
			break;
		default:
			errno = EINVAL;
			return -1;
			break;
		}
		break;
	case DM75xx_UTC_2:

		switch (source) {
		case DM75xx_CUTC_8_MHZ:
		case DM75xx_CUTC_EXT_TC_CLOCK_1:
		case DM75xx_CUTC_EXT_TC_CLOCK_2:
		case DM75xx_CUTC_EXT_PCLK:
		case DM75xx_CUTC_UTC_1_OUT:
			break;
		default:
			errno = EINVAL;
			return -1;
			break;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the user timer/counter gate value.

@param
    utc_select

    The user timer/counter selected.

@param
    gate

    The gate value to validate.

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      utc_select is not valid

        @arg \c
            EINVAL      gate is not valid
 *******************************************************************************
 */
int validate_utc_gate(dm75xx_utc_timer_t utc_select, dm75xx_utc_gate gate)
{
	switch (utc_select) {
	case DM75xx_UTC_0:
		switch (gate) {
		case DM75xx_GUTC_NOT_GATED:
		case DM75xx_GUTC_GATED:
		case DM75xx_GUTC_EXT_TC_CLK_1:
		case DM75xx_GUTC_EXT_TC_CLK_2:
			break;
		default:
			errno = EINVAL;
			return -1;
			break;
		}
		break;
	case DM75xx_UTC_1:
		switch (gate) {
		case DM75xx_GUTC_NOT_GATED:
		case DM75xx_GUTC_GATED:
		case DM75xx_GUTC_EXT_TC_CLK_1:
		case DM75xx_GUTC_EXT_TC_CLK_2:
		case DM75xx_GUTC_UTC_0_OUT:
			break;
		default:
			errno = EINVAL;
			return -1;
			break;
		}
		break;
	case DM75xx_UTC_2:
		switch (gate) {
		case DM75xx_GUTC_NOT_GATED:
		case DM75xx_GUTC_GATED:
		case DM75xx_GUTC_EXT_TC_CLK_1:
		case DM75xx_GUTC_EXT_TC_CLK_2:
		case DM75xx_GUTC_UTC_0_OUT:
			break;
		default:
			errno = EINVAL;
			return -1;
			break;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    Validate the user timer/counter mode.

@param
    utc_select

    The user timer/counter selected.

@param
    mode

    The user timer/counter mode to validate.

@return
    0

    Success

@return
    -1

    Failure.@n@n

    errno may be set as follows:
        @arg \c
            EINVAL      utc_select is not valid

        @arg \c
            EINVAL      mode is not valid
 *******************************************************************************
 */
int validate_utc_mode(dm75xx_utc_timer_t utc_select, dm75xx_utc_mode mode)
{
	switch (utc_select) {
	case DM75xx_UTC_0:
	case DM75xx_UTC_1:
	case DM75xx_UTC_2:
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	switch (mode) {
	case DM75xx_UTC_MODE_EVENT_COUNTER:
	case DM75xx_UTC_MODE_PROG_ONE_SHOT:
	case DM75xx_UTC_MODE_RATE_GENERATOR:
	case DM75xx_UTC_MODE_SQUARE_WAVE:
	case DM75xx_UTC_MODE_SOFTWARE_STROBE:
	case DM75xx_UTC_MODE_HARDWARE_STROBE:
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    32 bit write access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Data to write to the register

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Write_Register_32(DM75xx_Board_Descriptor * handle,
			 dm75xx_pci_region_num_t region,
			 uint16_t offset, uint32_t data)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.readwrite.access.region = region;
	ioctl_request.readwrite.access.offset = offset;
	ioctl_request.readwrite.access.size = DM75xx_PCI_REGION_ACCESS_32;
	ioctl_request.readwrite.access.data.data32 = data;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_WRITE, &ioctl_request)
	    == -1) {
		return -1;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    32 bit read access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Address where to store the read data.

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Read_Register_32(DM75xx_Board_Descriptor * handle,
			dm75xx_pci_region_num_t region,
			uint16_t offset, uint32_t * data)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.readwrite.access.region = region;
	ioctl_request.readwrite.access.offset = offset;
	ioctl_request.readwrite.access.size = DM75xx_PCI_REGION_ACCESS_32;
	ioctl_request.readwrite.access.data.data32 = 0x00000000;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_READ, &ioctl_request)
	    == -1) {
		return -1;
	}
	*data = ioctl_request.readwrite.access.data.data32;
	return 0;
}

/**
*******************************************************************************
@brief
    16 bit write access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Data to write to the register

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Write_Register_16(DM75xx_Board_Descriptor * handle,
			 dm75xx_pci_region_num_t region,
			 uint16_t offset, uint16_t data)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.readwrite.access.region = region;
	ioctl_request.readwrite.access.offset = offset;
	ioctl_request.readwrite.access.size = DM75xx_PCI_REGION_ACCESS_16;
	ioctl_request.readwrite.access.data.data16 = data;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_WRITE, &ioctl_request)
	    == -1) {
		return -1;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    16 bit read access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Address where to store the read data.

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Read_Register_16(DM75xx_Board_Descriptor * handle,
			dm75xx_pci_region_num_t region,
			uint16_t offset, uint16_t * data)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.readwrite.access.region = region;
	ioctl_request.readwrite.access.offset = offset;
	ioctl_request.readwrite.access.size = DM75xx_PCI_REGION_ACCESS_16;
	ioctl_request.readwrite.access.data.data16 = 0x0000;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_READ, &ioctl_request)
	    == -1) {
		return -1;
	}
	*data = ioctl_request.readwrite.access.data.data16;
	return 0;
}

/**
*******************************************************************************
@brief
    8 bit write access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Data to write to the register

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Write_Register_8(DM75xx_Board_Descriptor * handle,
			dm75xx_pci_region_num_t region,
			uint16_t offset, uint8_t data)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.readwrite.access.region = region;
	ioctl_request.readwrite.access.offset = offset;
	ioctl_request.readwrite.access.size = DM75xx_PCI_REGION_ACCESS_8;
	ioctl_request.readwrite.access.data.data8 = data;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_WRITE, &ioctl_request)
	    == -1) {
		return -1;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    8 bit read access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Address where to store the read value.

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Read_Register_8(DM75xx_Board_Descriptor * handle,
		       dm75xx_pci_region_num_t region,
		       uint16_t offset, uint8_t * data)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.readwrite.access.region = region;
	ioctl_request.readwrite.access.offset = offset;
	ioctl_request.readwrite.access.size = DM75xx_PCI_REGION_ACCESS_8;
	ioctl_request.readwrite.access.data.data8 = 0x0000;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_READ, &ioctl_request)
	    == -1) {
		return -1;
	}
	*data = ioctl_request.readwrite.access.data.data8;
	return 0;
}

/**
*******************************************************************************
@brief
    8 bit read-modify-write access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Data to write to the register

@param
    mask

    Mask of the bits to be modified.

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Modify_Register_8(DM75xx_Board_Descriptor * handle,
			 dm75xx_pci_region_num_t region,
			 uint16_t offset, uint8_t data, uint8_t mask)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.modify.access.region = region;
	ioctl_request.modify.access.offset = offset;
	ioctl_request.modify.access.size = DM75xx_PCI_REGION_ACCESS_8;
	ioctl_request.modify.access.data.data8 = data;
	ioctl_request.modify.mask.mask8 = mask;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_MODIFY,
	     &ioctl_request)
	    == -1) {
		return -1;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    16 bit read-modify-write access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Data to write to the register

@param
    mask

    Mask of the bits not to be modified.

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Modify_Register_16(DM75xx_Board_Descriptor * handle,
			  dm75xx_pci_region_num_t region,
			  uint16_t offset, uint16_t data, uint16_t mask)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.modify.access.region = region;
	ioctl_request.modify.access.offset = offset;
	ioctl_request.modify.access.size = DM75xx_PCI_REGION_ACCESS_16;
	ioctl_request.modify.access.data.data16 = data;
	ioctl_request.modify.mask.mask16 = mask;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_MODIFY,
	     &ioctl_request)
	    == -1) {
		return -1;
	}
	return 0;
}

/**
*******************************************************************************
@brief
    32 bit read-modify-write access to a DM75xx register.

@param
    handle

    Address of the device's library board descriptor.

@param
    region

    Local address space to access.

@param
    offset

    Register to access.

@param
    data

    Data to write to the register

@param
    mask

    Mask of the bits not to be modified.

@return
    0

    Success.

@return
    -1

    Failure.
 *******************************************************************************
 */
DM75xx_Error
DM75xx_Modify_Register_32(DM75xx_Board_Descriptor * handle,
			  dm75xx_pci_region_num_t region,
			  uint16_t offset, uint32_t data, uint32_t mask)
{
	dm75xx_ioctl_argument_t ioctl_request;
	ioctl_request.modify.access.region = region;
	ioctl_request.modify.access.offset = offset;
	ioctl_request.modify.access.size = DM75xx_PCI_REGION_ACCESS_32;
	ioctl_request.modify.access.data.data32 = data;
	ioctl_request.modify.mask.mask32 = mask;
	if (ioctl
	    (handle->file_descriptor, DM75xx_IOCTL_REGION_MODIFY,
	     &ioctl_request)
	    == -1) {
		return -1;
	}
	return 0;
}

/**
 * @} DM75xx_Library_Private_Functions
 */
/**
 * @} DM75xx_Library_Source
 */
/*=============================================================================
Public functions
 =============================================================================*/

/******************************************************************************
Determine device PCI master capable status
 ******************************************************************************/

DM75xx_Error
DM75xx_Board_PCI_Master(DM75xx_Board_Descriptor * handle, uint8_t * pci_master)
{
	uint32_t data;

	if (DM75xx_Read_Register_32
	    (handle, DM75xx_LAS0, DM75xx_LAS0_MT_MODE, &data)
	    != 0) {
		return -1;
	}

	*pci_master = data & 0x00000001;

	return 0;
}

/******************************************************************************
Reset a DM75xx device
 ******************************************************************************/

DM75xx_Error DM75xx_Board_Reset(DM75xx_Board_Descriptor * handle)
{
	dm75xx_ioctl_argument_t arg;

	return ioctl(handle->file_descriptor, DM75xx_IOCTL_RESET, arg);
}

/******************************************************************************
 Clear Interrupts via Mask
 ******************************************************************************/
DM75xx_Error
DM75xx_Clear_ITMask(DM75xx_Board_Descriptor * handle, uint16_t mask)
{
	uint32_t data;

	if (DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_CLEAR_IT,
				     mask)
	    != 0) {
		return -1;
	}
	return DM75xx_Read_Register_32(handle, DM75xx_LAS0,
				       DM75xx_LAS0_CLEAR_IT, &data);
}

/*******************************************************************************
 Clear Interrupt Overrun Register
 ******************************************************************************/
DM75xx_Error DM75xx_Clear_IT_Overrun(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_IT_OVERRUN, NO_ARG);
}

/*******************************************************************************
 Test Return Status and exit if error
 ******************************************************************************/
void DM75xx_Exit_On_Error(DM75xx_Board_Descriptor * handle,
			  DM75xx_Error status, char *str)
{

	if (status == 0) {
		return;
	}
	DM75xx_Board_Init(handle);
	DM75xx_Interrupt_Disable(handle, DM75xx_INT_DMA_0);
	DM75xx_Interrupt_Disable(handle, DM75xx_INT_DMA_1);
	DM75xx_DMA_Abort(handle, DM75xx_DMA_CHANNEL_0);
	DM75xx_DMA_Abort(handle, DM75xx_DMA_CHANNEL_1);
	DM75xx_RemoveISR(handle);
	DM75xx_Board_Close(handle);
	error(EXIT_FAILURE, errno, "ERROR: %s FAILED\n", str);
}

/******************************************************************************
Initialize a Board
 ******************************************************************************/
DM75xx_Error DM75xx_Board_Init(DM75xx_Board_Descriptor * handle)
{
	DM75xx_Error dm75xx_status = 0x00;

	dm75xx_status |= DM75xx_Board_Reset(handle);
	dm75xx_status |= DM75xx_ADC_Clear(handle);
	dm75xx_status |= DM75xx_DAC_Clear(handle, DM75xx_DAC1);
	dm75xx_status |= DM75xx_DAC_Clear(handle, DM75xx_DAC2);
	dm75xx_status |= DM75xx_DAC_Reset(handle, DM75xx_DAC1);
	dm75xx_status |= DM75xx_DAC_Reset(handle, DM75xx_DAC2);
	dm75xx_status |= DM75xx_HSDIN_Clear(handle);
	dm75xx_status |= DM75xx_CGT_Clear(handle);
	dm75xx_status |= DM75xx_DIO_Reset(handle);
	dm75xx_status |= DM75xx_DIO_Clear_IRQ(handle);
	dm75xx_status |= DM75xx_Clear_ITMask(handle, 0xFFFF);
	dm75xx_status |= DM75xx_Clear_IT_Overrun(handle);

	if (dm75xx_status != 0) {
		return -1;
	}
	return 0;
}

/*******************************************************************************
Enable Interrupt Sources
 ******************************************************************************/
DM75xx_Error DM75xx_Interrupt_Enable(DM75xx_Board_Descriptor * handle,
				     dm75xx_int_source_t int_source)
{
	dm75xx_ioctl_argument_t ioctl_request;

	ioctl_request.int_control.source = int_source;
	ioctl_request.int_control.function = DM75xx_INT_CONTROL_ENABLE;

	return ioctl(handle->file_descriptor,
		     DM75xx_IOCTL_INT_CONTROL, &ioctl_request);
}

/*******************************************************************************
Disable Interrupt Sources
 ******************************************************************************/
DM75xx_Error DM75xx_Interrupt_Disable(DM75xx_Board_Descriptor * handle,
				      dm75xx_int_source_t int_source)
{
	dm75xx_ioctl_argument_t ioctl_request;

	ioctl_request.int_control.source = int_source;
	ioctl_request.int_control.function = DM75xx_INT_CONTROL_DISABLE;

	return ioctl(handle->file_descriptor,
		     DM75xx_IOCTL_INT_CONTROL, &ioctl_request);
}

/*******************************************************************************
Check Interrupt Sources
 ******************************************************************************/
DM75xx_Error DM75xx_Interrupt_Check(DM75xx_Board_Descriptor * handle,
				    dm75xx_int_source_t * int_source)
{
	dm75xx_ioctl_argument_t ioctl_request;

	ioctl_request.int_control.function = DM75xx_INT_CONTROL_CHECK;

	if (ioctl(handle->file_descriptor,
		  DM75xx_IOCTL_INT_CONTROL, &ioctl_request) != 0) {
		return -1;
	}

	*int_source = ioctl_request.int_control.source;
	return 0;
}

/******************************************************************************
Copy User Space data into Kernel DMA Buffer
 ******************************************************************************/
DM75xx_Error DM75xx_DMA_Buffer_Write(DM75xx_Board_Descriptor * handle,
				     dm75xx_dma_channel_t channel,
				     unsigned long num_ints)
{
	unsigned long k_buf_siz, u_buf_siz;
	uint16_t *k_buf, *u_buf;
	uint8_t data;

	if (validate_dma_channel(channel)) {
		return -1;
	}

	k_buf = handle->k_buf[channel];
	u_buf = handle->u_buf[channel];
	k_buf_siz = handle->k_buf_siz[channel];
	u_buf_siz = handle->u_buf_siz[channel];

	memcpy(k_buf + (((num_ints - 1) % 2) * (k_buf_siz / 2)),
	       u_buf + (((num_ints - 1) * (k_buf_siz / 2)) % u_buf_siz),
	       k_buf_siz);

	data = ((num_ints - 1) % 2) << 4;

	data |= channel & DM75xx_DMA_RESET_SEL;

	return ioctl(handle->file_descriptor,
		     DM75xx_IOCTL_RESET_DMA_STATUS, &data);
}

/******************************************************************************
Copy Kernel DMA Buffer into User Space
 ******************************************************************************/
DM75xx_Error DM75xx_DMA_Buffer_Read(DM75xx_Board_Descriptor * handle,
				    dm75xx_dma_channel_t channel,
				    unsigned long num_ints)
{
	unsigned long k_buf_siz, u_buf_siz;
	uint16_t *k_buf, *u_buf;
	uint8_t data;

	if (validate_dma_channel(channel)) {
		return -1;
	}

	k_buf = handle->k_buf[channel];
	u_buf = handle->u_buf[channel];
	k_buf_siz = handle->k_buf_siz[channel];
	u_buf_siz = handle->u_buf_siz[channel];

	memcpy(u_buf + ((num_ints - 1) * (k_buf_siz / 2)) % u_buf_siz,
	       k_buf + (((num_ints - 1) % 2) * (k_buf_siz / 2)), k_buf_siz);

	data = ((num_ints - 1) % 2) << 4;

	data |= channel & DM75xx_DMA_RESET_SEL;

	return ioctl(handle->file_descriptor,
		     DM75xx_IOCTL_RESET_DMA_STATUS, &data);
}

/******************************************************************************
Allocate DMA buffer
 ******************************************************************************/
DM75xx_Error DM75xx_DMA_Buffer_Create(DM75xx_Board_Descriptor * handle,
				      uint16_t ** buffer,
				      dm75xx_dma_channel_t channel,
				      uint32_t samples)
{
	*buffer = (uint16_t *) malloc(samples * sizeof(uint16_t));
	if (*buffer == NULL) {
		return -1;
	}
	handle->u_buf[channel] = *buffer;
	handle->u_buf_siz[channel] = samples;
	return 0;
}

/******************************************************************************
Deallocate DMA buffer
 ******************************************************************************/
DM75xx_Error DM75xx_DMA_Buffer_Free(DM75xx_Board_Descriptor * handle,
				    uint16_t ** buffer,
				    dm75xx_dma_channel_t channel)
{
	if (validate_dma_channel(channel)) {
		return -1;
	}

	if (*buffer != NULL) {
		free(*buffer);
		*buffer = NULL;
	} else {
		return -1;
	}
	handle->u_buf[channel] = NULL;
	return 0;
}

/******************************************************************************
Initialize DMA channel for arbitrary PCI transfer
 ******************************************************************************/

DM75xx_Error
DM75xx_DMA_Init_Arb(DM75xx_Board_Descriptor * handle,
		    dm75xx_dma_channel_t channel,
		    dm75xx_dma_source_t source,
		    dm75xx_dma_request_t request,
		    uint32_t samples, uint32_t pci_address)
{
	dm75xx_ioctl_argument_t ioctl_request;
	DM75xx_Error dm75xx_status;

	if (samples == 0) {
		errno = EINVAL;
		return -1;
	}

	dm75xx_status = DM75xx_DMA_Request_Source(handle, channel, request);

	if (dm75xx_status != 0) {
		return -1;
	}

	if (validate_dma_channel(channel) || validate_dma_source(source)) {
		return -1;
	}

	ioctl_request.dma_function.channel = channel;
	ioctl_request.dma_function.source = source;
	ioctl_request.dma_function.request = request;
	ioctl_request.dma_function.size = samples * 2;
	ioctl_request.dma_function.pci_address = pci_address;
	ioctl_request.dma_function.arb = 0xFF;
	ioctl_request.dma_function.function = DM75xx_DMA_FUNCTION_INITIALIZE;

	return ioctl(handle->file_descriptor,
		     DM75xx_IOCTL_DMA_FUNCTION, &ioctl_request);
}

/******************************************************************************
Initialize DMA channel
 ******************************************************************************/

DM75xx_Error
DM75xx_DMA_Initialize(DM75xx_Board_Descriptor * handle,
		      dm75xx_dma_channel_t channel,
		      dm75xx_dma_source_t source,
		      dm75xx_dma_request_t request,
		      uint32_t samples, uint16_t ** buf)
{
	dm75xx_ioctl_argument_t ioctl_request;
	DM75xx_Error dm75xx_status;

	if (samples == 0) {
		errno = EINVAL;
		return -1;
	}

	dm75xx_status = DM75xx_DMA_Request_Source(handle, channel, request);

	if (dm75xx_status != 0) {
		printf("Could not request source.");
		return -1;
	}

	if (validate_dma_channel(channel) || validate_dma_source(source) ||
	    validate_dma_request(request)) {
		return -1;
	}

	ioctl_request.dma_function.channel = channel;
	ioctl_request.dma_function.source = source;
	ioctl_request.dma_function.request = request;
	ioctl_request.dma_function.size = samples * 2;
	ioctl_request.dma_function.arb = 0x00;
	ioctl_request.dma_function.function = DM75xx_DMA_FUNCTION_INITIALIZE;

	if (ioctl(handle->file_descriptor, DM75xx_IOCTL_DMA_FUNCTION,
		  &ioctl_request) == -1) {
		printf("failure requesting ioctl");
		return -1;
	}
	/*
	 * Map user space buffer to the kernel space DMA buffer
	 */
	*buf = (uint16_t *) mmap(NULL, samples * 2, (PROT_READ | PROT_WRITE),
				 (MAP_SHARED | MAP_LOCKED),
				 handle->file_descriptor, 0);
	/*
	 * Verify a successful mapping.
	 */
	if ((*buf == MAP_FAILED) || (*buf == NULL)) {
		printf("No mmap for you! %d\n", errno);
		return -1;
	}

	handle->k_buf[channel] = *buf;
	handle->k_buf_siz[channel] = samples;

	return dm75xx_status;
}

/******************************************************************************
 Abort DMA on a given channel
 ******************************************************************************/

DM75xx_Error
DM75xx_DMA_Abort(DM75xx_Board_Descriptor * handle, dm75xx_dma_channel_t channel)
{
	dm75xx_ioctl_argument_t ioctl_request;

	if (validate_dma_channel(channel)) {
		return -1;
	}

	ioctl_request.dma_function.channel = channel;
	ioctl_request.dma_function.function = DM75xx_DMA_FUNCTION_ABORT;

	handle->k_buf[channel] = NULL;

	return ioctl(handle->file_descriptor, DM75xx_IOCTL_DMA_FUNCTION,
		     &ioctl_request);
}

/******************************************************************************
 Enable/Disable DMA Channel
 ******************************************************************************/

DM75xx_Error
DM75xx_DMA_Enable(DM75xx_Board_Descriptor * handle,
		  dm75xx_dma_channel_t channel, uint8_t enable)
{
	uint16_t offset;
	uint8_t byte, mask;

	switch (channel) {
	case DM75xx_DMA_CHANNEL_0:
		offset = DM75xx_PLX_DMA_CSR0;
		break;
	case DM75xx_DMA_CHANNEL_1:
		offset = DM75xx_PLX_DMA_CSR1;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	if (enable) {
		byte = 0x01;
	} else {
		byte = 0x04;
	}

	mask = 0x05;

	return DM75xx_Modify_Register_8(handle, DM75xx_PLX_MEM, offset, byte,
					mask);

}

/******************************************************************************
 Set Demand Mode DMA Request Source
 ******************************************************************************/

DM75xx_Error
DM75xx_DMA_Request_Source(DM75xx_Board_Descriptor * handle,
			  dm75xx_dma_channel_t channel,
			  dm75xx_dma_request_t request)
{
	uint16_t offset;
	if (validate_dma_request(request)) {
		return -1;
	}

	switch (channel) {
	case DM75xx_DMA_CHANNEL_0:
		offset = DM75xx_LAS0_DMA0_SRC;
		break;
	case DM75xx_DMA_CHANNEL_1:
		offset = DM75xx_LAS0_DMA1_SRC;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, offset, request);
}

/******************************************************************************
 Start a DMA Channel
 ******************************************************************************/

DM75xx_Error
DM75xx_DMA_Start(DM75xx_Board_Descriptor * handle, dm75xx_dma_channel_t channel)
{
	uint16_t offset, reset, status;
	uint8_t byte, mask;
	switch (channel) {
	case DM75xx_DMA_CHANNEL_0:
		offset = DM75xx_PLX_DMA_CSR0;
		reset = DM75xx_LAS0_DMA_RSTRQST0;
		break;
	case DM75xx_DMA_CHANNEL_1:
		offset = DM75xx_PLX_DMA_CSR1;
		reset = DM75xx_LAS0_DMA_RSTRQST1;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	byte = 0x02;
	mask = 0x02;

	status = DM75xx_Write_Register_8(handle, DM75xx_LAS0, reset, NO_ARG);

	status |= DM75xx_Modify_Register_8(handle, DM75xx_PLX_MEM, offset, byte,
					   mask);

	return status;
}

/******************************************************************************
Close a DM75xx device
 ******************************************************************************/

DM75xx_Error DM75xx_Board_Close(DM75xx_Board_Descriptor * handle)
{
	if (handle == NULL) {
		errno = ENODATA;
		return -1;
	}

	if (close(handle->file_descriptor) == -1) {
		free(handle);
		return -1;
	}

	free(handle);
	return 0;
}

/******************************************************************************
Open a DM75xx device
 ******************************************************************************/

DM75xx_Error
DM75xx_Board_Open(uint8_t dev_num, DM75xx_Board_Descriptor ** handle)
{
	char device_name[25];
	int descriptor;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Form the device file name and attempt to open the file
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	(void)snprintf(&(device_name[0]), sizeof(device_name),
		       "/dev/rtd-dm75xx-%u", dev_num);
	descriptor = open(&(device_name[0]), O_RDWR);
	if (descriptor == -1) {
		if (errno != EBUSY) {
			*handle = NULL;
		}
		return -1;
	}

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Allocate and initialize memory for the library device descriptor
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	*handle =
	    (DM75xx_Board_Descriptor *) malloc(sizeof(DM75xx_Board_Descriptor)
	    );
	if (*handle == NULL) {
		return -1;
	}

	(void)memset(*handle, 0x00, sizeof(DM75xx_Board_Descriptor));

	(*handle)->file_descriptor = descriptor;
	(*handle)->isr = NULL;
	return 0;
}

/******************************************************************************
Get Device FIFO Size
 ******************************************************************************/
DM75xx_Error
DM75xx_FIFO_Size(DM75xx_Board_Descriptor * handle, unsigned int *data)
{
	return ioctl(handle->file_descriptor, DM75xx_IOCTL_GET_FIFO_SIZE, data);
}

/******************************************************************************
Get the board type (7520 vs. 7540)
 ******************************************************************************/
DM75xx_Error DM75xx_Board_Type(DM75xx_Board_Descriptor * handle,
			       dm75xx_board_t * data)
{
	return ioctl(handle->file_descriptor, DM75xx_IOCTL_GET_BOARD_TYPE,
		     data);
}

/******************************************************************************
Install userspace ISR
 ******************************************************************************/

DM75xx_Error
DM75xx_InstallISR(DM75xx_Board_Descriptor * handle,
		  void (*isr_fnct) (unsigned int status))
{
	int status;

	/*
	 * Check for ISR already installed
	 */

	if (handle->isr != NULL) {
		printf("ISR already installed.\n");
		return -EBUSY;
	}

	/*
	 * Set devices isr to the passed userspace isr
	 */

	handle->isr = isr_fnct;

	/*
	 * Start the thread to wait for the interrupt
	 */

	status = pthread_create(&(handle->pid),
				NULL, DM75xx_WaitForInterrupt, handle);

	if (status != 0) {
		printf("Could not create thread.\n");
		return -EFAULT;
	}

	return status;
}

/******************************************************************************
Uninstall userspace ISR
 ******************************************************************************/

DM75xx_Error DM75xx_RemoveISR(DM75xx_Board_Descriptor * handle)
{

	/*
	 * Check to make sure there exists an ISR to remove
	 */

	if (!handle->isr) {
		return -EFAULT;
	}

	/*
	 * Make ISR pointer NULL this will be seen by the thread and is a signal
	 * for it to quit.
	 */

	handle->isr = NULL;

	/*
	 * Join back up with ISR thread
	 */

	ioctl(handle->file_descriptor, DM75xx_IOCTL_WAKEUP);

	return pthread_join(handle->pid, NULL);
}

/******************************************************************************
Thread calls the board's poll method here and waits for interrupts
 ******************************************************************************/
void *DM75xx_WaitForInterrupt(void *ptr)
{
	fd_set exception_fds;
	fd_set read_fds;
	int status;
	sigset_t signal_mask;
	DM75xx_Board_Descriptor *handle;
	dm75xx_int_status_t interrupt_status;
	handle = (DM75xx_Board_Descriptor *) ptr;

	/*
	 * Set signal mask for thread
	 * This thread will ignore all signals except SIGKILL and SIGSTOP which
	 * are non-maskable anyway.
	 */

	sigfillset(&signal_mask);

	sigdelset(&signal_mask, SIGKILL);
	sigdelset(&signal_mask, SIGSTOP);

	status = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);

	if (status != 0) {
		handle->thread_status = -EFAULT;
		return handle;
	}

	while (1) {

		/*
		 * Set up the set of file descriptors that will be watched for input
		 * activity.  Only the DM75xx device file descriptor is of interest.
		 */

		FD_ZERO(&read_fds);
		FD_SET(handle->file_descriptor, &read_fds);

		/*
		 * Set up the set of file descriptors that will be watched for exception
		 * activity.  Only the DM75xx file descriptor is of interest.
		 */

		FD_ZERO(&exception_fds);
		FD_SET(handle->file_descriptor, &exception_fds);

		/*
		 * Wait for the interrupt to happen.  No timeout is given, which means
		 * the process will not be woken up until either an interrupt occurs or
		 * a signal is delivered.  This ends up calling the DM75xx's poll
		 * function within the driver.
		 */

		status = select((handle->file_descriptor) + 1,
				&read_fds, NULL, &exception_fds, NULL);

		/*
		 * The isr should be a null pointer if RemoveISR has been called
		 * This checks if the user has asked the thread to quit.
		 */

		if (handle->isr == NULL) {
			break;
		}

		/*
		 * Check select() error status
		 */

		if (status == -1) {

			/*
			 * Some error occurred.
			 */

			(*(handle->isr)) (status);
			break;
		}

		if (status == 0) {

			/*
			 * No file descriptors have data available.  Something is broken in the
			 * driver.
			 */

			(*(handle->isr)) (-ENODATA);
			break;
		}
		/*
		 * An exception occured, this means that no IRQ line was allocated to
		 * the device when the driver was loaded.
		 */

		if (FD_ISSET(handle->file_descriptor, &exception_fds)) {
			(*(handle->isr)) (-EIO);
			break;
		}

		/*
		 * At least one file descriptor has data available and no exception
		 * occured.  Check the device file descriptor to see if it is readable.
		 */

		if (!FD_ISSET(handle->file_descriptor, &read_fds)) {

			/*
			 * The device file is not readable.  This means something is broken.
			 */

			(*(handle->isr)) (-ENODATA);
			break;
		}

		/*
		 * An interrupt occured, get the first interrupt status
		 */

		status = ioctl(handle->file_descriptor, DM75xx_IOCTL_INT_STATUS,
			       &interrupt_status);

		if (status != 0) {
			handle->thread_status = -EFAULT;
			return handle;

		}

		/*
		 * Loop through all remaining interrupts
		 */

		while (interrupt_status.int_remaining != -1) {

			/*
			 * Call the Interrupt Service Routine and pass through the status
			 */

			(*(handle->isr)) (interrupt_status.status);

			/*
			 * Get Next Status
			 */

			status =
			    ioctl(handle->file_descriptor,
				  DM75xx_IOCTL_INT_STATUS, &interrupt_status);

			if (status != 0) {
				handle->thread_status = -EFAULT;
				return handle;

			}
		}
	}

	/*
	 * Terminate waiting thread
	 */
	handle->thread_status = 0;
	return handle;
}

/*##############################################################################
    User Timer/Counter 8254 Functions
##############################################################################*/
/******************************************************************************
Set User Timer/Counter Clock Source
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Set_Clock_Source(DM75xx_Board_Descriptor * handle,
			    dm75xx_utc_timer_t utc, dm75xx_utc_clk_t source)
{
	uint16_t utc_offset = 0x0000;
	if (validate_utc_clock_source(utc, source)) {
		return -1;
	}
	/*
	 * Determine the offset
	 */
	switch (utc) {
	case DM75xx_UTC_0:
		utc_offset = DM75xx_LAS0_UTC0_CLOCK;
		break;
	case DM75xx_UTC_1:
		utc_offset = DM75xx_LAS0_UTC1_CLOCK;
		break;
	case DM75xx_UTC_2:
		utc_offset = DM75xx_LAS0_UTC2_CLOCK;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, utc_offset,
					source);
}

/******************************************************************************
Set User Timer/Counter Gate
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Set_Gate(DM75xx_Board_Descriptor * handle,
		    dm75xx_utc_timer_t utc, dm75xx_utc_gate gate)
{
	uint16_t utc_offset = 0x0000;

	if (validate_utc_gate(utc, gate)) {
		return -1;
	}
	/*
	 * Determine the offset
	 */
	switch (utc) {
	case DM75xx_UTC_0:
		utc_offset = DM75xx_LAS0_UTC0_GATE;
		break;
	case DM75xx_UTC_1:
		utc_offset = DM75xx_LAS0_UTC1_GATE;
		break;
	case DM75xx_UTC_2:
		utc_offset = DM75xx_LAS0_UTC2_GATE;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, utc_offset, gate);
}

/******************************************************************************
Set User Timer/Counter Mode
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Set_Mode(DM75xx_Board_Descriptor * handle,
		    dm75xx_utc_timer_t utc, dm75xx_utc_mode mode)
{
	if (validate_utc_mode(utc, mode)) {
		return -1;
	}

	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_UTC_CTRL,
					((utc << 6) | (mode << 1) | 0x30));
}

/******************************************************************************
Get the User/Timer Counter Mode
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Get_Mode(DM75xx_Board_Descriptor * handle,
		    dm75xx_utc_timer_t utc, uint16_t * mode)
{
	uint16_t offset, cntr;
	uint32_t data;

	switch (utc) {
	case DM75xx_UTC_0:
		offset = DM75xx_LAS0_UTC0;
		cntr = 0x02;
		break;
	case DM75xx_UTC_1:
		offset = DM75xx_LAS0_UTC1;
		cntr = 0x04;
		break;
	case DM75xx_UTC_2:
		offset = DM75xx_LAS0_UTC2;
		cntr = 0x08;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	/*
	 * Write the control word
	 * This is using a Read-Back command to get the mode.
	 */
	if (DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_UTC_CTRL,
				     (0xE0 | cntr))
	    != 0) {
		return -1;
	}
	/*
	 * We need to read a random register to delay 165ns to allow
	 * multiple reads from this register.
	 */
	if (DM75xx_Read_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_MT_MODE,
				    &data)
	    != 0) {
		return -1;
	}
	/*
	 * Read LSB
	 */
	if (DM75xx_Read_Register_32(handle, DM75xx_LAS0, offset, &data)
	    != 0) {
		return -1;
	}
	/*
	 * Mask off everything but the 3 mode bits which are
	 * bits 1, 2, and 3
	 */
	*mode = (((data & 0x00FF) >> 1) & 0x7);

	return 0;
}

/******************************************************************************
Set User Timer/Counter Divisor
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Set_Divisor(DM75xx_Board_Descriptor * handle,
		       dm75xx_utc_timer_t utc, uint16_t divisor)
{
	uint16_t offset, lsb, msb;

	lsb = (divisor & 0x00FF);
	msb = (divisor & 0xFF00) >> 8;

	switch (utc) {
	case DM75xx_UTC_0:
		offset = DM75xx_LAS0_UTC0;
		break;
	case DM75xx_UTC_1:
		offset = DM75xx_LAS0_UTC1;
		break;
	case DM75xx_UTC_2:
		offset = DM75xx_LAS0_UTC2;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	/*
	 * Write least significant byte.
	 */
	if (DM75xx_Write_Register_32(handle, DM75xx_LAS0, offset, lsb)
	    != 0) {
		return -1;
	}
	/*
	 * Write most significant byte.
	 */
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, offset, msb);
}

/******************************************************************************
Return current User Timer/Counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Get_Count(DM75xx_Board_Descriptor * handle,
		     dm75xx_utc_timer_t utc, uint16_t * count)
{
	uint16_t offset;
	uint32_t lsb, msb;

	switch (utc) {
	case DM75xx_UTC_0:
		offset = DM75xx_LAS0_UTC0;
		break;
	case DM75xx_UTC_1:
		offset = DM75xx_LAS0_UTC1;
		break;
	case DM75xx_UTC_2:
		offset = DM75xx_LAS0_UTC2;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	/*
	 * Latch the counter
	 */
	if (DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_UTC_CTRL,
				     (utc << 6))
	    != 0) {
		return -1;
	}
	/*
	 * We need to read a random register to delay 165ns to allow
	 * multiple reads from this register.
	 */
	if (DM75xx_Read_Register_32
	    (handle, DM75xx_LAS0, DM75xx_LAS0_MT_MODE, &lsb)
	    != 0) {
		return -1;
	}
	/*
	 * Read least significant byte
	 */
	if (DM75xx_Read_Register_32(handle, DM75xx_LAS0, offset, &lsb)
	    != 0) {
		return -1;
	}
	/*
	 * Read most significant byte
	 */
	if (DM75xx_Read_Register_32(handle, DM75xx_LAS0, offset, &msb)
	    != 0) {
		return -1;
	}

	lsb &= 0x000000FF;
	msb &= 0x000000FF;

	*count = ((msb << 8) | lsb);

	return 0;
}

/******************************************************************************
Return current User Timer/Counter status
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Get_Status(DM75xx_Board_Descriptor * handle,
		      dm75xx_utc_timer_t utc_select, uint8_t * utc_status)
{
	uint16_t offset;
	uint32_t data;

	switch (utc_select) {
	case DM75xx_UTC_0:
		offset = DM75xx_LAS0_UTC0;
		break;
	case DM75xx_UTC_1:
		offset = DM75xx_LAS0_UTC1;
		break;
	case DM75xx_UTC_2:
		offset = DM75xx_LAS0_UTC2;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	/*
	 * Write the control word for the readback command.
	 */
	if (DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_UTC_CTRL,
				     0xE0 | (1 << (utc_select + 1)))
	    != 0) {
		return -1;
	}
	/*
	 * We need to read a random register to delay 165ns to allow
	 * multiple reads from this register.
	 */
	if (DM75xx_Read_Register_32
	    (handle, DM75xx_LAS0, DM75xx_LAS0_MT_MODE, &data)
	    != 0) {
		return -1;
	}
	/*
	 * Now perform the actual register read.
	 */
	if (DM75xx_Read_Register_32(handle, DM75xx_LAS0, offset, &data)
	    != 0) {
		return -1;
	}
	/*
	 * Mask off unecessary bits.
	 */
	*utc_status = data & 0xFF;
	return 0;
}

/******************************************************************************
Setup a User Timer/Counter
 ******************************************************************************/
DM75xx_Error
DM75xx_UTC_Setup(DM75xx_Board_Descriptor * handle,
		 dm75xx_utc_timer_t utc,
		 dm75xx_utc_clk_t source,
		 dm75xx_utc_gate gate, dm75xx_utc_mode mode, uint16_t divisor)
{
	DM75xx_Error dm75xx_status = 0x0000;
	dm75xx_status |= DM75xx_UTC_Set_Clock_Source(handle, utc, source);
	dm75xx_status |= DM75xx_UTC_Set_Gate(handle, utc, gate);
	dm75xx_status |= DM75xx_UTC_Set_Mode(handle, utc, mode);
	dm75xx_status |= DM75xx_UTC_Set_Divisor(handle, utc, divisor);
	if (dm75xx_status != 0) {
		return -1;
	}
	return 0;
}

/*##############################################################################
 Generic Status Functions
##############################################################################*/
/*******************************************************************************
 Get FIFO Status
 ******************************************************************************/
DM75xx_Error
DM75xx_FIFO_Get_Status(DM75xx_Board_Descriptor * handle, uint16_t * fifo_status)
{
	uint32_t data;

	if (DM75xx_Read_Register_32
	    (handle, DM75xx_LAS0, DM75xx_LAS0_FIFO_STATUS, &data) != 0) {
		return -1;
	}
	/*
	 * Mask off unecessary bits.
	 */
	*fifo_status = data & 0x7777;
	return 0;
}

/*******************************************************************************
 Get Pacer/Burst Status
 ******************************************************************************/
DM75xx_Error
DM75xx_CLK_Get_Status(DM75xx_Board_Descriptor * handle, uint16_t * status)
{
	return
	    DM75xx_Read_Register_16(handle, DM75xx_LAS0, DM75xx_LAS0_TIMER,
				    status);
}

/*##############################################################################
 Burst Clock Functions
##############################################################################*/
/*******************************************************************************
 Get Burst Clock counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_BCLK_Get_Count(DM75xx_Board_Descriptor * handle, uint16_t * data)
{

	return DM75xx_Read_Register_16(handle, DM75xx_LAS0,
				       DM75xx_LAS0_BCLK, data);
}

/*******************************************************************************
 Set Burst Clock counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_BCLK_Set_Count(DM75xx_Board_Descriptor * handle, uint16_t data)
{
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_BCLK, data);
}

/*******************************************************************************
 Set Burst Clock counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_BCLK_Set_Rate(DM75xx_Board_Descriptor * handle,
		     dm75xx_bclk_freq_t freq, float rate, float *actualRate)
{
	uint16_t divisor;
	uint32_t primary_clock;
	if (DM75xx_BCLK_Set_Frequency(handle, freq)) {
		return -1;
	}

	switch (freq) {
	case DM75xx_BCLK_FREQ_8_MHZ:
		primary_clock = 8000000L;
		break;
	case DM75xx_BCLK_FREQ_20_MHZ:
		primary_clock = 20000000L;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	if (rate > 1250000L) {
		errno = EINVAL;
		return -1;
	}

	if (rate <= (float)primary_clock / 0xFFFF) {
		return -1;
	}

	divisor = primary_clock / rate;
	*actualRate = (float)primary_clock / divisor;
	return DM75xx_BCLK_Set_Count(handle, divisor - 1);
}

/*******************************************************************************
 Set Burst Clock start trigger
 ******************************************************************************/
DM75xx_Error
DM75xx_BCLK_Set_Start(DM75xx_Board_Descriptor * handle,
		      dm75xx_bclk_start_t start)
{
	if (validate_bclk_start(start)) {
		return -1;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_BURST_START, start);
}

/*******************************************************************************
 Set Burst Clock primary frequency
 ******************************************************************************/
DM75xx_Error
DM75xx_BCLK_Set_Frequency(DM75xx_Board_Descriptor * handle,
			  dm75xx_bclk_freq_t freq)
{
	if (validate_bclk_freq(freq)) {
		return -1;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_BCLK_FREQ, freq);
}

/*******************************************************************************
 Get Burst Clock primary frequency
 ******************************************************************************/
DM75xx_Error
DM75xx_BCLK_Setup(DM75xx_Board_Descriptor * handle,
		  dm75xx_bclk_start_t start,
		  dm75xx_bclk_freq_t freq, float rate, float *actualRate)
{
	DM75xx_Error dm75xx_status = 0x0000;

	dm75xx_status = DM75xx_BCLK_Set_Start(handle, start);
	dm75xx_status = DM75xx_BCLK_Set_Rate(handle, freq, rate, actualRate);
	if (dm75xx_status != 0) {
		return -1;
	}
	return dm75xx_status;
}

/*##############################################################################
 Pacer Clock Functions
##############################################################################*/
/*******************************************************************************
 Set the Pacer Clock frequency
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Frequency(DM75xx_Board_Descriptor * handle,
			  dm75xx_pclk_freq_t pclk_freq)
{
	if (validate_pclk_freq(pclk_freq)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_PCLK_FREQ, pclk_freq);
}

/*******************************************************************************
 Set the Pacer Clock source
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Source(DM75xx_Board_Descriptor * handle,
		       dm75xx_pclk_select_t pclk_select)
{
	if (validate_pclk_select(pclk_select)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_PACER_SELECT, pclk_select);
}

/*******************************************************************************
 Set the Pacer Clock Start Trigger
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Start(DM75xx_Board_Descriptor * handle,
		      dm75xx_pclk_start_t pclk_start)
{
	if (validate_pclk_start(pclk_start)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_PACER_START, pclk_start);
}

/*******************************************************************************
 Set the Pacer Clock Stop Trigger
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Stop(DM75xx_Board_Descriptor * handle,
		     dm75xx_pclk_stop_t pclk_stop)
{
	if (validate_pclk_stop(pclk_stop)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_PACER_STOP, pclk_stop);
}

/*******************************************************************************
 Read Pacer Clock
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Read(DM75xx_Board_Descriptor * handle, uint32_t * pacer_value)
{
	uint32_t data = 0x00000000;

	DM75xx_Read_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_PCLK, &data);
	*pacer_value = data & 0xFFFFFF;
	return 0;
}

/*******************************************************************************
 Set Pacer Start Trigger Mode
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Trigger_Mode(DM75xx_Board_Descriptor * handle,
			     dm75xx_pclk_mode_t pclk_mode)
{
	if (validate_pclk_mode(pclk_mode)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_PACER_MODE, pclk_mode);
}

/*******************************************************************************
 Set Pacer Clock Count
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Count(DM75xx_Board_Descriptor * handle, uint32_t count)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_PCLK,
					count);
}

/*******************************************************************************
 Set Pacer Clock Rate
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Set_Rate(DM75xx_Board_Descriptor * handle,
		     dm75xx_pclk_freq_t freq, float rate, float *actualRate)
{
	uint32_t divisor;
	uint32_t primary_clock;
	if (DM75xx_PCLK_Set_Frequency(handle, freq)) {
		return -1;
	}

	switch (freq) {
	case DM75xx_PCLK_FREQ_8_MHZ:
		primary_clock = 8000000L;
		break;
	case DM75xx_PCLK_FREQ_20_MHZ:
		primary_clock = 20000000L;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	if (rate > 1250000L) {
		errno = EINVAL;
		return -1;
	}

	if (rate <= (float)primary_clock / 0x00FFFFFF) {
		return -1;
	}

	divisor = primary_clock / rate;
	*actualRate = (float)primary_clock / divisor;
	return DM75xx_PCLK_Set_Count(handle, divisor - 1);
}

/*******************************************************************************
 Setup Pacer Clock
 ******************************************************************************/
DM75xx_Error
DM75xx_PCLK_Setup(DM75xx_Board_Descriptor * handle,
		  dm75xx_pclk_select_t pclk_select,
		  dm75xx_pclk_freq_t pclk_freq,
		  dm75xx_pclk_mode_t pclk_mode,
		  dm75xx_pclk_start_t pclk_start,
		  dm75xx_pclk_stop_t pclk_stop, float rate, float *actualRate)
{
	DM75xx_Error status = 0x0000;

	status |= DM75xx_PCLK_Set_Source(handle, pclk_select);
	status |= DM75xx_PCLK_Set_Frequency(handle, pclk_freq);
	status |= DM75xx_PCLK_Set_Trigger_Mode(handle, pclk_mode);
	status |= DM75xx_PCLK_Set_Start(handle, pclk_start);
	status |= DM75xx_PCLK_Set_Stop(handle, pclk_stop);
	status |= DM75xx_PCLK_Set_Rate(handle, pclk_freq, rate, actualRate);

	if (status != 0) {
		return -1;
	}
	return 0;
}

/*******************************************************************************
 Pacer Start - Software
 ******************************************************************************/
DM75xx_Error DM75xx_PCLK_Start(DM75xx_Board_Descriptor * handle)
{
	uint32_t data;
	return DM75xx_Read_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_PACER,
				       &data);
}

/*******************************************************************************
 Pacer Stop - Software
 ******************************************************************************/
DM75xx_Error DM75xx_PCLK_Stop(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_PACER,
					0x00);
}

/*##############################################################################
  Channel Gain Table Functions
 #############################################################################*/
/*******************************************************************************
 Enable Channel Gain Table
 ******************************************************************************/
DM75xx_Error
DM75xx_CGT_Enable(DM75xx_Board_Descriptor * handle, uint16_t enable)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_CGT_ENABLE, enable);
}

/*******************************************************************************
 Enable Digital Table
 ******************************************************************************/
DM75xx_Error DM75xx_DT_Enable(DM75xx_Board_Descriptor * handle, uint16_t enable)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_DT_ENABLE, enable);
}

/*******************************************************************************
 CGT Latch (needed for single-channel operation)
 ******************************************************************************/
DM75xx_Error
DM75xx_CGT_Latch(DM75xx_Board_Descriptor * handle, dm75xx_cgt_entry_t cgt)
{
	uint16_t cgt_entry;
	DM75xx_CGT_Create_Entry(&cgt, &cgt_entry);
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_CGT_LATCH, cgt_entry);
}

/*******************************************************************************
 Write CGT entry
 ******************************************************************************/
DM75xx_Error
DM75xx_CGT_Write(DM75xx_Board_Descriptor * handle, dm75xx_cgt_entry_t cgt)
{
	uint16_t cgt_entry;
	DM75xx_CGT_Create_Entry(&cgt, &cgt_entry);
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_CGT_WRITE, cgt_entry);
}

/******************************************************************************
Reset CGT
 ******************************************************************************/
DM75xx_Error DM75xx_CGT_Reset(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_CGT_RESET, NO_ARG);
}

/******************************************************************************
Clear CGT
 ******************************************************************************/
DM75xx_Error DM75xx_CGT_Clear(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_CGT_CLEAR, NO_ARG);
}

/*******************************************************************************
 Create CGT Entry
 ******************************************************************************/
DM75xx_Error
DM75xx_CGT_Create_Entry(dm75xx_cgt_entry_t * cgt, uint16_t * cgt_entry)
{
	/*
	 * Here we have to build our 16 bit CGT entry from the struct
	 */
	*cgt_entry = 0x0000;
	*cgt_entry |= (cgt->channel & 0x0F);
	*cgt_entry |= (cgt->gain & 0x07) << 4;
	*cgt_entry |= (cgt->nrse & 0x01) << 7;
	*cgt_entry |= (cgt->range & 0x03) << 8;
	*cgt_entry |= (cgt->ground & 0x01) << 10;
	*cgt_entry |= (cgt->pause & 0x01) << 11;
	*cgt_entry |= (cgt->dac1 & 0x01) << 12;
	*cgt_entry |= (cgt->dac2 & 0x01) << 13;
	*cgt_entry |= (cgt->skip & 0x01) << 14;
	return 0;
}

/*******************************************************************************
 Write Digital Table Entry
 ******************************************************************************/
DM75xx_Error
DM75xx_DT_Write_Entry(DM75xx_Board_Descriptor * handle, uint8_t data)
{
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_DT_WRITE, data);
}

/*******************************************************************************
 Pause channel gain table.
 ******************************************************************************/
DM75xx_Error DM75xx_CGT_Pause(DM75xx_Board_Descriptor * handle, uint16_t pause)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_PAUSE_TABLE, pause);
}

/*##############################################################################
 Analog to Digital Functions.
 #############################################################################*/
/*******************************************************************************
 Read Analog to Digital FIFO
 ******************************************************************************/
DM75xx_Error
DM75xx_ADC_FIFO_Read(DM75xx_Board_Descriptor * handle, uint16_t * value)
{
	return DM75xx_Read_Register_16(handle, DM75xx_LAS1,
				       DM75xx_LAS1_ADC_FIFO, value);
}

/*******************************************************************************
 A/D Software Sample
 ******************************************************************************/
DM75xx_Error DM75xx_ADC_Software_Sample(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_FIFO_STATUS, NO_ARG);
}

/*******************************************************************************
 Set Analog to Digital Conversion Signal
 ******************************************************************************/
DM75xx_Error
DM75xx_ADC_Conv_Signal(DM75xx_Board_Descriptor * handle,
		       dm75xx_adc_conv_signal_t adc_conv_signal)
{
	if (validate_adc_conv_signal(adc_conv_signal)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_ADC_CONV, adc_conv_signal);
}

/*******************************************************************************
 A/D Sample Counter source select
 ******************************************************************************/
DM75xx_Error
DM75xx_ADC_SCNT_Source(DM75xx_Board_Descriptor * handle,
		       dm75xx_adc_scnt_src_t src)
{
	if (validate_adc_scnt_src(src)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_ADC_SCNT_SRC, src);
}

/*******************************************************************************
 About Counter Stop Enable
 ******************************************************************************/
DM75xx_Error
DM75xx_ADC_About_Enable(DM75xx_Board_Descriptor * handle, uint16_t enable)
{
	uint16_t data;
	if (enable > 0) {
		data = 0x0000;
	} else if (enable == 0x00) {
		data = 0xFFFF;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_ACNT_ENABLE, data);
}

/******************************************************************************
 Clear A/D Fifo
 ******************************************************************************/
DM75xx_Error DM75xx_ADC_Clear(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_ADC_FIFO_CLR, NO_ARG);
}

/******************************************************************************
 Read A/D Sample Counter
 ******************************************************************************/
DM75xx_Error
DM75xx_ADC_SCNT_Read(DM75xx_Board_Descriptor * handle, uint16_t * data)
{
	return DM75xx_Read_Register_16(handle, DM75xx_LAS0,
				       DM75xx_LAS0_ADC_SCNT, data);
}

/******************************************************************************
 Write A/D Sample Counter
 ******************************************************************************/
DM75xx_Error
DM75xx_ADC_SCNT_Load(DM75xx_Board_Descriptor * handle, uint16_t data)
{
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0,
					DM75xx_LAS0_ADC_SCNT, data);
}

/*##############################################################################
 Digital to Analog Functions
 #############################################################################*/
/*******************************************************************************
 Cause a DAC Software update
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Soft_Update(DM75xx_Board_Descriptor * handle, uint8_t dac)
{
	if (dac == DM75xx_DAC1) {
		DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_DAC1,
					 NO_ARG);
	} else if (dac == DM75xx_DAC2) {
		DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_DAC2,
					 NO_ARG);
	} else if (dac == (DM75xx_DAC1 & DM75xx_DAC2)) {
		DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_DAC,
					 NO_ARG);
	} else {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

/*******************************************************************************
 Get Digital to Analog Update Counter
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Get_Update_Counter(DM75xx_Board_Descriptor * handle,
			      dm75xx_dac_channel_t dac, uint16_t * data)
{
	uint16_t offset;
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_UCNT;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_UCNT;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Read_Register_16(handle, DM75xx_LAS0, offset, data);
}

/*******************************************************************************
 Set Digital to Analog Update Counter
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Update_Counter(DM75xx_Board_Descriptor * handle,
			      dm75xx_dac_channel_t dac, uint16_t data)
{
	uint16_t offset;
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_UCNT;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_UCNT;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0, offset, data);
}

/*******************************************************************************
 Set Digital to Analog Range
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Range(DM75xx_Board_Descriptor * handle,
		     dm75xx_dac_channel_t dac, dm75xx_dac_range_t range)
{
	uint16_t offset;
	if (validate_dac_channel(dac)) {
		return -1;
	}
	if (validate_dac_range(range)) {
		return -1;
	}
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_RANGE;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_RANGE;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0, offset, range);
}

/*******************************************************************************
 Set Digital to Analog Update Source
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Update_Source(DM75xx_Board_Descriptor * handle,
			     dm75xx_dac_channel_t dac,
			     dm75xx_dac_update_src_t src)
{
	uint16_t offset;
	if (validate_dac_channel(dac)) {
		return -1;
	}
	if (validate_dac_update_source(src)) {
		return -1;
	}
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_SRC;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_SRC;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0, offset, src);
}

/*******************************************************************************
 Set Digital to Analog Cycle Mode
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Mode(DM75xx_Board_Descriptor * handle,
		    dm75xx_dac_channel_t dac, dm75xx_dac_mode_t mode)
{
	uint16_t offset;
	if (validate_dac_mode(dac, mode)) {
		return -1;
	}
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_CYCLE;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_CYCLE;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0, offset, mode);
}

/*******************************************************************************
 Write a value to the DAC FIFO
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_FIFO_Write(DM75xx_Board_Descriptor * handle,
		      dm75xx_dac_channel_t dac, uint16_t data)
{
	uint16_t offset;
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS1_DAC1_FIFO;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS1_DAC2_FIFO;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS1, offset, data);
}

/*******************************************************************************
 Set Digital to Analog Clock
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Frequency(DM75xx_Board_Descriptor * handle,
			 dm75xx_dac_freq_t freq)
{
	if (validate_dac_freq(freq)) {
		return -1;
	}
	return DM75xx_Write_Register_16(handle,
					DM75xx_LAS0,
					DM75xx_LAS0_DAC_CLK_SOURCE, freq);
}

/*******************************************************************************
 Set Digital to Analog Clock Count
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Count(DM75xx_Board_Descriptor * handle, uint32_t count)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_DAC_CLK, count);
}

/*******************************************************************************
 Set Digital to Analog Clock Rate
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Rate(DM75xx_Board_Descriptor * handle,
		    dm75xx_dac_freq_t freq, uint32_t rate, float *actualRate)
{
	uint32_t primary_clock;
	if (DM75xx_DAC_Set_Frequency(handle, freq)) {
		return -1;
	}

	switch (freq) {
	case DM75xx_DAC_FREQ_8_MHZ:
		primary_clock = 8000000L;
		break;
	case DM75xx_DAC_FREQ_20_MHZ:
		primary_clock = 20000000L;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	if (rate > 20000000L || rate == 0) {
		errno = EINVAL;
		return -1;
	}

	rate = primary_clock / rate;
	*actualRate = (float)primary_clock / rate;
	return DM75xx_DAC_Set_Count(handle, rate - 1);
}

/*******************************************************************************
 Set Digital to Analog Clock Stop
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Clock_Stop(DM75xx_Board_Descriptor * handle,
			  dm75xx_dac_clk_stop_t stop)
{
	if (validate_dac_clk_stop(stop)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle,
					DM75xx_LAS0,
					DM75xx_LAS0_DAC_CLK_STOP, stop);
}

/*******************************************************************************
 Set Digital to Analog Clock Start
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_Clock_Start(DM75xx_Board_Descriptor * handle,
			   dm75xx_dac_clk_start_t start)
{
	if (validate_dac_clk_start(start)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle,
					DM75xx_LAS0,
					DM75xx_LAS0_DAC_CLK_START, start);
}

/*******************************************************************************
 DAC Start - Software
 ******************************************************************************/
DM75xx_Error DM75xx_DAC_Start(DM75xx_Board_Descriptor * handle)
{
	uint32_t arg;
	return DM75xx_Read_Register_32(handle, DM75xx_LAS0,
				       DM75xx_LAS0_DAC_CLK_ST, &arg);
}

/*******************************************************************************
 DAC Stop - Software
 ******************************************************************************/
DM75xx_Error DM75xx_DAC_Stop(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_DAC_CLK_ST, NO_ARG);
}

/*******************************************************************************
 Setup DAC
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Setup(DM75xx_Board_Descriptor * handle,
		 dm75xx_dac_channel_t dac,
		 dm75xx_dac_range_t range,
		 dm75xx_dac_update_src_t src, dm75xx_dac_mode_t mode)
{
	DM75xx_Error dm75xx_status = 0x0000;

	dm75xx_status |= DM75xx_DAC_Set_Range(handle, dac, range);
	dm75xx_status |= DM75xx_DAC_Set_Update_Source(handle, dac, src);
	dm75xx_status |= DM75xx_DAC_Set_Mode(handle, dac, mode);
	return dm75xx_status;
}

/*******************************************************************************
 Reset DAC FIFO
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Reset(DM75xx_Board_Descriptor * handle, dm75xx_dac_channel_t dac)
{
	uint16_t offset;
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_RESET;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_RESET;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, offset, NO_ARG);
}

/******************************************************************************
Clear DAC FIFO
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Clear(DM75xx_Board_Descriptor * handle, dm75xx_dac_channel_t dac)
{
	uint16_t offset;
	switch (dac) {
	case DM75xx_DAC1:
		offset = DM75xx_LAS0_DAC1_CLEAR;
		break;
	case DM75xx_DAC2:
		offset = DM75xx_LAS0_DAC2_CLEAR;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, offset, NO_ARG);
}

/*******************************************************************************
 Enable/Disable Start/Stop mode
 ******************************************************************************/
DM75xx_Error
DM75xx_DAC_Set_CLK_Mode(DM75xx_Board_Descriptor * handle,
			dm75xx_dac_clk_mode_t clk_mode)
{
	if (validate_dac_clk_mode(clk_mode)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_DAC_CLK_MODE, clk_mode);
}

/*##############################################################################
 HighSpeed Digital Functions
 #############################################################################*/
/*******************************************************************************
 HighSpeed Digital Software Sample
 ******************************************************************************/
DM75xx_Error DM75xx_HSDIN_Software_Sample(DM75xx_Board_Descriptor * handle)
{
	return
	    DM75xx_Write_Register_32(handle, DM75xx_LAS0, DM75xx_LAS0_TIMER,
				     NO_ARG);
}

/*******************************************************************************
 Set HighSpeed Digital Sampling Signal select
 ******************************************************************************/
DM75xx_Error
DM75xx_HSDIN_Sample_Signal(DM75xx_Board_Descriptor * handle,
			   dm75xx_hsdin_signal_t signal)
{
	if (validate_hsdin_signal(signal)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_HSDIN_START, signal);
}

/******************************************************************************
 Clear HSDIN FIFO
 ******************************************************************************/
DM75xx_Error DM75xx_HSDIN_Clear(DM75xx_Board_Descriptor * handle)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_HSDIN_FIFO_CLR, NO_ARG);
}

/*******************************************************************************
 Read HSDIN FIFO
 ******************************************************************************/
DM75xx_Error
DM75xx_HSDIN_FIFO_Read(DM75xx_Board_Descriptor * handle, uint16_t * data)
{
	return DM75xx_Read_Register_16(handle, DM75xx_LAS1,
				       DM75xx_LAS1_HSDIN_FIFO, data);
}

/*##############################################################################
 SyncBus Functions
 #############################################################################*/
/*******************************************************************************
 Set SyncBus Source Select
 ******************************************************************************/
DM75xx_Error
DM75xx_SBUS_Set_Source(DM75xx_Board_Descriptor * handle, dm75xx_sbus_t sbus,
		       dm75xx_sbus_src_t src)
{
	uint16_t offset;
	if (validate_sbus_source(src)) {
		return -1;
	}
	switch (sbus) {
	case DM75xx_SBUS0:
		offset = DM75xx_LAS0_SBUS0_SOURCE;
		break;
	case DM75xx_SBUS1:
		offset = DM75xx_LAS0_SBUS1_SOURCE;
		break;
	case DM75xx_SBUS2:
		offset = DM75xx_LAS0_SBUS2_SOURCE;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0, offset, src);
}

/*******************************************************************************
 Enable/Disable SyncBus
 ******************************************************************************/
DM75xx_Error
DM75xx_SBUS_Enable(DM75xx_Board_Descriptor * handle, dm75xx_sbus_t sbus,
		   uint16_t enable)
{
	uint16_t offset;
	switch (sbus) {
	case DM75xx_SBUS0:
		offset = DM75xx_LAS0_SBUS0_ENABLE;
		break;
	case DM75xx_SBUS1:
		offset = DM75xx_LAS0_SBUS1_ENABLE;
		break;
	case DM75xx_SBUS2:
		offset = DM75xx_LAS0_SBUS2_ENABLE;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0, offset, enable);
}

/*##############################################################################
 External Trigger/Interrupt Configuration
 #############################################################################*/
/*******************************************************************************
 External Trigger Polarity select
 ******************************************************************************/
DM75xx_Error
DM75xx_ETRIG_Polarity_Select(DM75xx_Board_Descriptor * handle,
			     dm75xx_ext_polarity_t polarity)
{
	if (validate_ext_polarity(polarity)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_ETRG_POLARITY, polarity);
}

/*******************************************************************************
 External Interrupt Polarity select
 ******************************************************************************/
DM75xx_Error
DM75xx_EINT_Polarity_Select(DM75xx_Board_Descriptor * handle,
			    dm75xx_ext_polarity_t polarity)
{
	if (validate_ext_polarity(polarity)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_EINT_POLARITY, polarity);
}

/*##############################################################################
 About Counter
 #############################################################################*/
/*******************************************************************************
 Get About Counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_ACNT_Get_Count(DM75xx_Board_Descriptor * handle, uint16_t * data)
{
	return DM75xx_Read_Register_16(handle, DM75xx_LAS0, DM75xx_LAS0_ACNT,
				       data);
}

/*******************************************************************************
 Set About Counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_ACNT_Set_Count(DM75xx_Board_Descriptor * handle, uint16_t data)
{
	return
	    DM75xx_Write_Register_16(handle, DM75xx_LAS0, DM75xx_LAS0_ACNT,
				     data);
}

/*##############################################################################
 Delay Counter
 #############################################################################*/
/*******************************************************************************
 Get Delay Counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_DCNT_Get_Count(DM75xx_Board_Descriptor * handle, uint16_t * data)
{
	return DM75xx_Read_Register_16(handle, DM75xx_LAS0, DM75xx_LAS0_DCNT,
				       data);
}

/*******************************************************************************
 Set Delay Counter value
 ******************************************************************************/
DM75xx_Error
DM75xx_DCNT_Set_Count(DM75xx_Board_Descriptor * handle, uint16_t data)
{
	return
	    DM75xx_Write_Register_16(handle, DM75xx_LAS0, DM75xx_LAS0_DCNT,
				     data);
}

/*##############################################################################
 Digital Input/Output
 #############################################################################*/
/*******************************************************************************
 Write Digital Port Output Line
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Set_Port(DM75xx_Board_Descriptor * handle, dm75xx_dio_port_t port,
		    uint8_t data)
{
	uint16_t offset;
	switch (port) {
	case DM75xx_DIO_PORT0:
		offset = DM75xx_LAS0_DIO0;
		break;
	case DM75xx_DIO_PORT1:
		offset = DM75xx_LAS0_DIO1;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_8(handle, DM75xx_LAS0, offset, data);
}

/*******************************************************************************
 Read Digital Ports Input Line
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Get_Port(DM75xx_Board_Descriptor * handle, dm75xx_dio_port_t port,
		    uint8_t * data)
{
	uint16_t offset;
	switch (port) {
	case DM75xx_DIO_PORT0:
		offset = DM75xx_LAS0_DIO0;
		break;
	case DM75xx_DIO_PORT1:
		offset = DM75xx_LAS0_DIO1;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Read_Register_8(handle, DM75xx_LAS0, offset, data);
}

/*******************************************************************************
 Read DIO IRQ Status
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Get_Status(DM75xx_Board_Descriptor * handle, uint8_t * data)
{
	return DM75xx_Read_Register_8(handle, DM75xx_LAS0,
				      DM75xx_LAS0_DIO_STATUS, data);
}

/*******************************************************************************
 Clear Digital IRQ Status
 ******************************************************************************/
DM75xx_Error DM75xx_DIO_Clear_IRQ(DM75xx_Board_Descriptor * handle)
{
	uint8_t data, mask;
	data = 0x00;
	mask = 0x03;

	/*
	 * Modify bits 0 and 1 to logic 0.
	 */

	if (DM75xx_Modify_Register_8
	    (handle, DM75xx_LAS0, DM75xx_LAS0_DIO_STATUS, data, mask) != 0) {
		return -1;
	}

	/*
	 * Once the status register is set, this read will actually clear the IRQ.
	 */

	return DM75xx_Read_Register_8(handle, DM75xx_LAS0,
				      DM75xx_LAS0_DIO_CTRL, &data);
}

/******************************************************************************
 Clear Digital Chip
 ******************************************************************************/
DM75xx_Error DM75xx_DIO_Reset(DM75xx_Board_Descriptor * handle)
{
	uint8_t data, mask;
	data = 0x00;
	mask = 0x03;

	/*
	 * Modify bits 0 and 1 to logic 0.
	 */

	if (DM75xx_Modify_Register_8
	    (handle, DM75xx_LAS0, DM75xx_LAS0_DIO_STATUS, data, mask) != 0) {
		return -1;
	}

	/*
	 * Once the status register is set, this write will reset the DIO.
	 */

	return DM75xx_Write_Register_8(handle, DM75xx_LAS0,
				       DM75xx_LAS0_DIO_CTRL, NO_ARG);
}

/*******************************************************************************
 Set Port 0 Direction
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Set_Direction(DM75xx_Board_Descriptor * handle,
			 dm75xx_dio_port_t port, uint8_t direction)
{
	uint8_t data, mask;
	if (port == DM75xx_DIO_PORT0) {
		mask = 0x03;
		data = 0x01;

		/*
		 * Port 0 is bit adjustable and you must first select the register
		 * that contains the Port 0 direction from the DIO Status register.
		 */

		if (DM75xx_Modify_Register_8
		    (handle, DM75xx_LAS0, DM75xx_LAS0_DIO_STATUS, data, mask)
		    != 0) {
			return -1;
		}

		/*
		 * Now write the direction to the Port 0 Direction register.
		 */

		return DM75xx_Write_Register_8(handle, DM75xx_LAS0,
					       DM75xx_LAS0_DIO_CTRL, direction);
	} else if (port == DM75xx_DIO_PORT1) {
		mask = 0x04;

		if (direction > 0) {
			data = 0x04;
		} else {
			data = 0x00;
		}

		/*
		 * Port 1 is byte adjustable and is controlled by a single bit
		 * in the DIO Status register.
		 */

		return DM75xx_Modify_Register_8(handle, DM75xx_LAS0,
						DM75xx_LAS0_DIO_STATUS, data,
						mask);
	} else {
		return -1;
	}
}

/*******************************************************************************
 Set Port 0 Mask
 ******************************************************************************/
DM75xx_Error DM75xx_DIO_Set_Mask(DM75xx_Board_Descriptor * handle, uint8_t mask)
{
	uint8_t data, bit_mask;
	bit_mask = 0x03;
	data = 0x02;
	/*
	 * Select the mask register by settings bits 0 and 1 of the status register.
	 */
	if (DM75xx_Modify_Register_8
	    (handle, DM75xx_LAS0, DM75xx_LAS0_DIO_STATUS, data, bit_mask)
	    != 0) {
		return -1;
	}
	/*
	 * Now write the mask.
	 */
	return DM75xx_Write_Register_8(handle, DM75xx_LAS0,
				       DM75xx_LAS0_DIO_CTRL, mask);
}

/*******************************************************************************
 Set Port 0 Compare
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Set_Compare(DM75xx_Board_Descriptor * handle, uint8_t compare)
{
	uint8_t data, mask;
	mask = 0x03;
	data = 0x03;
	/*
	 * Select the compare register from the status register
	 */
	if (DM75xx_Modify_Register_8
	    (handle, DM75xx_LAS0, DM75xx_LAS0_DIO_STATUS, data, mask)
	    != 0) {
		return -1;
	}
	/*
	 * Write the compare value.
	 */
	return DM75xx_Write_Register_8(handle, DM75xx_LAS0,
				       DM75xx_LAS0_DIO_CTRL, compare);
}

/*******************************************************************************
 Get Port 0 Compare
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Get_Compare(DM75xx_Board_Descriptor * handle, uint8_t * compare)
{
	uint8_t data, mask;
	mask = 0x03;
	data = 0x03;
	/*
	 * Select the compare register from the status register
	 */
	if (DM75xx_Modify_Register_8
	    (handle, DM75xx_LAS0, DM75xx_LAS0_DIO_STATUS, data, mask)
	    != 0) {
		return -1;
	}
	/*
	 * Read the compare value.
	 */
	return DM75xx_Read_Register_8(handle, DM75xx_LAS0,
				      DM75xx_LAS0_DIO_CTRL, compare);
}

/*******************************************************************************
 Set DIO IRQ Mode
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_IRQ_Mode(DM75xx_Board_Descriptor * handle, dm75xx_dio_mode_t mode)
{
	uint8_t data, mask;
	mask = 0x08;
	switch (mode) {
	case DM75xx_DIO_MODE_EVENT:
		data = 0x00;
		break;
	case DM75xx_DIO_MODE_MATCH:
		data = 0x08;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Modify_Register_8(handle, DM75xx_LAS0,
					DM75xx_LAS0_DIO_STATUS, data, mask);
}

/*******************************************************************************
 Set DIO Sample Clock
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Clock(DM75xx_Board_Descriptor * handle, dm75xx_dio_clk_t clock)
{
	uint8_t data, mask;
	mask = 0x20;
	switch (clock) {
	case DM75xx_DIO_CLK_8MHZ:
		data = 0x00;
		break;
	case DM75xx_DIO_CLK_UTC1:
		data = 0x20;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Modify_Register_8(handle, DM75xx_LAS0,
					DM75xx_LAS0_DIO_STATUS, data, mask);
}

/*******************************************************************************
 Enable/Disable DIO IRQ
 ******************************************************************************/
DM75xx_Error
DM75xx_DIO_Enable_IRQ(DM75xx_Board_Descriptor * handle, uint8_t enable)
{
	uint8_t data, mask;

	mask = 0x10;

	if (enable == 0x00) {
		data = 0x00;
	} else {
		data = 0x10;
	}
	return DM75xx_Modify_Register_8(handle, DM75xx_LAS0,
					DM75xx_LAS0_DIO_STATUS, data, mask);
}

/*##############################################################################
 User I/O Signal Functions
 #############################################################################*/
/*******************************************************************************
 Select User I/O Signal
 ******************************************************************************/
DM75xx_Error
DM75xx_UIO_Select(DM75xx_Board_Descriptor * handle,
		  dm75xx_uio_channel_t channel, dm75xx_uio_source_t source)
{
	uint16_t offset;
	if (validate_uio_select(source)) {
		return -1;
	}
	switch (channel) {
	case DM75xx_UIO0:
		offset = DM75xx_LAS0_UIO0_SELECT;
		break;
	case DM75xx_UIO1:
		offset = DM75xx_LAS0_UIO1_SELECT;
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}
	return DM75xx_Write_Register_16(handle, DM75xx_LAS0, offset, source);
}

/*******************************************************************************
 Read User Inputs
 ******************************************************************************/
DM75xx_Error DM75xx_UIO_Read(DM75xx_Board_Descriptor * handle, uint32_t * data)
{
	return DM75xx_Read_Register_32(handle, DM75xx_LAS0,
				       DM75xx_LAS0_USER_IO, data);
}

/*******************************************************************************
 Write User Outputs
 ******************************************************************************/
DM75xx_Error DM75xx_UIO_Write(DM75xx_Board_Descriptor * handle, uint32_t data)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_USER_IO, data);
}

/*##############################################################################
 McBSP Functions
 #############################################################################*/
/*******************************************************************************
 Enable A/D FIFO to DSP
 ******************************************************************************/
DM75xx_Error DM75xx_McBSP_ADC_FIFO(DM75xx_Board_Descriptor * handle,
				   uint8_t enable)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_MCBSP_AD_CTRL, enable);
}

/*******************************************************************************
 Enable D/A FIFO to DSP
 ******************************************************************************/
DM75xx_Error DM75xx_McBSP_DAC_FIFO(DM75xx_Board_Descriptor * handle,
				   uint8_t enable)
{
	return DM75xx_Write_Register_32(handle, DM75xx_LAS0,
					DM75xx_LAS0_MCBSP_DA_CTRL, enable);
}

/*##############################################################################
 SDM7540 Family specific functions
 #############################################################################*/
/*******************************************************************************
 Calibrate an SDM7540
 ******************************************************************************/
DM75xx_Error DM75xx_Calibrate(DM75xx_Board_Descriptor * handle,
			      uint16_t dac1_value,
			      uint16_t dac2_value,
			      dm75xx_dac_range_t dac1_range,
			      dm75xx_dac_range_t dac2_range)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	if (validate_dac_range(dac1_range) || validate_dac_range(dac2_range)) {
		return -1;
	}

	/*
	 * Make SRAM Available
	 */

	dm75xx_status =
	    DM75xx_Write_Register_32(handle,
				     DM75xx_PLX_MEM,
				     DM75xx_PLX_SRAM, 0x40400001);
	/*
	 * If failure occurs we need to make LAS1 available again
	 */

	if (dm75xx_status != 0) {
		DM75xx_Write_Register_32(handle,
					 DM75xx_PLX_MEM,
					 DM75xx_PLX_SRAM, 0x40000001);
	}

	/*
	 * Write DAC1 Value to SRAM
	 */

	dm75xx_status =
	    DM75xx_Write_Register_32(handle, DM75xx_LAS1, 0 * 4, dac1_value);
	if (dm75xx_status != 0) {
		DM75xx_Write_Register_32(handle,
					 DM75xx_PLX_MEM,
					 DM75xx_PLX_SRAM, 0x40000001);
	}

	/*
	 * Write DAC2 Value to SRAM
	 */

	dm75xx_status =
	    DM75xx_Write_Register_32(handle, DM75xx_LAS1, 1 * 4, dac2_value);
	if (dm75xx_status != 0) {
		DM75xx_Write_Register_32(handle,
					 DM75xx_PLX_MEM,
					 DM75xx_PLX_SRAM, 0x40000001);
	}
	/*
	 * Write DAC1 Range to SRAM
	 */

	dm75xx_status =
	    DM75xx_Write_Register_32(handle, DM75xx_LAS1, 2 * 4, dac1_range);
	if (dm75xx_status != 0) {
		DM75xx_Write_Register_32(handle,
					 DM75xx_PLX_MEM,
					 DM75xx_PLX_SRAM, 0x40000001);
	}
	/*
	 * Write DAC2 Range to SRAM
	 */

	dm75xx_status =
	    DM75xx_Write_Register_32(handle, DM75xx_LAS1, 3 * 4, dac2_range);
	if (dm75xx_status != 0) {
		DM75xx_Write_Register_32(handle,
					 DM75xx_PLX_MEM,
					 DM75xx_PLX_SRAM, 0x40000001);
	}

	/*
	 * Make LAS1 available again
	 */

	DM75xx_Write_Register_32(handle,
				 DM75xx_PLX_MEM, DM75xx_PLX_SRAM, 0x40000001);
	/*
	 * Set the device's calibration flag to be read by the DSP
	 */

	return DM75xx_Write_Register_32(handle,
					DM75xx_LAS0,
					DM75xx_LAS0_DSP, DM75xx_DSP_CAL_AUTO);
}

/*******************************************************************************
 Issue a DSP command
 ******************************************************************************/
DM75xx_Error
DM75xx_DSP_CMD_Send(DM75xx_Board_Descriptor * handle,
		    dm75xx_dsp_command_t command)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}
	if (validate_dsp_command(command)) {
		return -1;
	}
	return DM75xx_Write_Register_32(handle,
					DM75xx_LAS0, DM75xx_LAS0_DSP, command);
}

/*******************************************************************************
 Check for Compeletion of a DSP command
 ******************************************************************************/
DM75xx_Error
DM75xx_DSP_CMD_Complete(DM75xx_Board_Descriptor * handle, uint8_t * data)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}
	return DM75xx_Read_Register_8(handle, DM75xx_LAS0, DM75xx_LAS0_DSP,
				      data);
}

/*******************************************************************************
 Check Status of DSP Command
 ******************************************************************************/
DM75xx_Error
DM75xx_DSP_CMD_Status(DM75xx_Board_Descriptor * handle,
		      dm75xx_dsp_command_t command)
{
	DM75xx_Error dm75xx_status;
	uint32_t data;
	dm75xx_board_t check;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	if (validate_dsp_command(command)) {
		return -1;
	}

	dm75xx_status = DM75xx_Read_Register_32(handle,
						DM75xx_LAS0,
						DM75xx_LAS0_DSP, &data);

	if (dm75xx_status != 0) {
		return -1;
	}

	data = data >> 8;

	return !(~(data) && command);
}

/*******************************************************************************
 Read analog connection DIO mask
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Get_Mask(DM75xx_Board_Descriptor * handle,
		       dm75xx_algdio_mask_t * pin1, dm75xx_algdio_mask_t * pin2)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;
	uint8_t data;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	dm75xx_status = DM75xx_Read_Register_8(handle,
					       DM75xx_LAS0,
					       DM75xx_LAS0_ALGDIO_MASK, &data);

	if (dm75xx_status != 0) {
		return -1;
	}

	if (data & 0x01)
		*pin1 = DM75xx_ALGDIO_UNMASKED;
	else
		*pin1 = DM75xx_ALGDIO_MASKED;

	if (data & 0x02)
		*pin2 = DM75xx_ALGDIO_UNMASKED;
	else
		*pin2 = DM75xx_ALGDIO_MASKED;

	return 0;
}

/*******************************************************************************
 Write analog connection DIO mask
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Set_Mask(DM75xx_Board_Descriptor * handle,
		       dm75xx_algdio_mask_t pin1, dm75xx_algdio_mask_t pin2)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;
	uint8_t mask;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	dm75xx_status = DM75xx_Read_Register_8(handle,
					       DM75xx_LAS0,
					       DM75xx_LAS0_ALGDIO_MASK, &mask);

	if (dm75xx_status != 0) {
		return -1;
	}

	mask &= ~0x03;
	mask |= pin1 + (pin2 << 1);

	return DM75xx_Write_Register_8(handle,
				       DM75xx_LAS0,
				       DM75xx_LAS0_ALGDIO_MASK, mask);
}

/*******************************************************************************
 Read Analog connection DIO direction
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Get_Direction(DM75xx_Board_Descriptor * handle,
			    dm75xx_algdio_direction_t * pin1,
			    dm75xx_algdio_direction_t * pin2)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;
	uint8_t data = 0x00;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	dm75xx_status = DM75xx_Read_Register_8(handle,
					       DM75xx_LAS0,
					       DM75xx_LAS0_ALGDIO_DIR, &data);

	if (dm75xx_status != 0) {
		return -1;
	}

	if (data & 0x01) {
		*pin1 = DM75xx_ALGDIO_OUTPUT;
	} else {
		*pin1 = DM75xx_ALGDIO_INPUT;
	}

	if (data & 0x02) {
		*pin2 = DM75xx_ALGDIO_OUTPUT;
	} else {
		*pin2 = DM75xx_ALGDIO_INPUT;
	}

	return 0;
}

/*******************************************************************************
 Write analog conenction DIO direction
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Set_Direction(DM75xx_Board_Descriptor * handle,
			    dm75xx_algdio_direction_t pin1,
			    dm75xx_algdio_direction_t pin2)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;
	uint8_t data = 0x00;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	if (pin1) {
		data |= 0x01;
	} else {
		data &= ~0x01;
	}

	if (pin2) {
		data |= 0x02;
	} else {
		data &= ~0x02;
	}

	return DM75xx_Write_Register_8(handle,
				       DM75xx_LAS0,
				       DM75xx_LAS0_ALGDIO_DIR, data);
}

/*******************************************************************************
 Read analog connection DIO data from output pins
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Set_Data(DM75xx_Board_Descriptor * handle,
		       uint8_t pin1, uint8_t pin2)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;
	uint8_t data = 0x00;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	if (pin1) {
		data |= 0x01;
	} else {
		data &= ~0x01;
	}

	if (pin2) {
		data |= 0x02;
	} else {
		data &= ~0x02;
	}

	return DM75xx_Write_Register_8(handle,
				       DM75xx_LAS0,
				       DM75xx_LAS0_ALGDIO_DATA, data);
}

/*******************************************************************************
 Write analog connection DIO data to output pins
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Get_Data(DM75xx_Board_Descriptor * handle,
		       uint8_t * pin1, uint8_t * pin2)
{
	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;
	uint8_t data = 0x00;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	dm75xx_status = DM75xx_Read_Register_8(handle,
					       DM75xx_LAS0,
					       DM75xx_LAS0_ALGDIO_DATA, &data);

	if (data & 0x01) {
		*pin1 = 0xFF;
	}

	if (data & 0x02) {
		*pin2 = 0xFF;
	}

	return 0;
}

/*******************************************************************************
 Read analog connection DIO IRQ status
 ******************************************************************************/
DM75xx_Error
DM75xx_ALGDIO_Get_IRQ_Status(DM75xx_Board_Descriptor * handle, uint8_t * status)
{

	DM75xx_Error dm75xx_status;
	dm75xx_board_t check;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	return DM75xx_Read_Register_8(handle,
				      DM75xx_LAS0,
				      DM75xx_LAS0_ALGDIO_IRQ, status);
}

/*******************************************************************************
 Get temperature value from the board
 ******************************************************************************/
DM75xx_Error DM75xx_Get_Temp(DM75xx_Board_Descriptor * handle, uint8_t * temp)
{
	DM75xx_Error dm75xx_status = 0x0000;
	dm75xx_board_t check;
	uint32_t data = 0x00;

	dm75xx_status = DM75xx_Board_Type(handle, &check);
	if (dm75xx_status != 0) {
		return -1;
	}

	if (check != DM75xx_BOARD_SDM7540) {
		fprintf(stdout, "Board does not support this function!\n");
		return -1;
	}

	/*
	 * Check if sensor is busy
	 */
	do {
		dm75xx_status |= DM75xx_Write_Register_32(handle,
							  DM75xx_LAS0,
							  DM75xx_LAS0_I2C_ADDR,
							  0x98);

		dm75xx_status |= DM75xx_Write_Register_32(handle,
							  DM75xx_LAS0,
							  DM75xx_LAS0_I2C_PTR,
							  0x02);

		dm75xx_status |= DM75xx_Write_Register_32(handle,
							  DM75xx_LAS0,
							  DM75xx_LAS0_I2C_GO,
							  0x02);

		usleep(1000);

		dm75xx_status |= DM75xx_Read_Register_32(handle,
							 DM75xx_LAS0,
							 DM75xx_LAS0_I2C_READ,
							 &data);

		if (dm75xx_status != 0) {
			return -1;
		}

	} while ((data & 0x80) == 0x80);
	/*
	 * configure sensor for one-shot reading
	 */
	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_ADDR, 0x98);

	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_PTR, 0x0F);

	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_DATA, 0x0A);

	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_GO, 0x03);

	if (dm75xx_status != 0) {
		return -1;
	}

	usleep(1000);

	/*
	 * wait for the sensor to finish reading
	 */
	do {

		dm75xx_status |= DM75xx_Write_Register_32(handle,
							  DM75xx_LAS0,
							  DM75xx_LAS0_I2C_ADDR,
							  0x98);

		dm75xx_status |= DM75xx_Write_Register_32(handle,
							  DM75xx_LAS0,
							  DM75xx_LAS0_I2C_PTR,
							  0x02);

		dm75xx_status |= DM75xx_Write_Register_32(handle,
							  DM75xx_LAS0,
							  DM75xx_LAS0_I2C_GO,
							  0x02);

		usleep(1000);

		dm75xx_status |= DM75xx_Read_Register_32(handle,
							 DM75xx_LAS0,
							 DM75xx_LAS0_I2C_READ,
							 &data);

		if (dm75xx_status != 0) {
			return -1;
		}

	} while ((data & 0x80) == 0x80);
	/*
	 * read the temp
	 */
	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_ADDR, 0x98);

	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_PTR, 0x00);

	dm75xx_status |= DM75xx_Write_Register_32(handle,
						  DM75xx_LAS0,
						  DM75xx_LAS0_I2C_GO, 0x02);

	usleep(1000);

	dm75xx_status |= DM75xx_Read_Register_32(handle,
						 DM75xx_LAS0,
						 DM75xx_LAS0_I2C_READ, &data);

	if (dm75xx_status != 0) {
		return -1;
	}
	/*
	 * temp is offset by 64 when in extended temp range
	 */
	*temp = (data & 0xFF) - 64;

	return 0;
}
