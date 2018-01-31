#include "AnalogManager.h"

/* default constructor */
AnalogManager::AnalogManager() {
  this->light_level = std::make_shared<LightLevel>();
  this->analog_acq = std::make_shared<AnalogAcq>();

  this->inst_mode_switch = false;
}


/* analog board read out */
int AnalogManager::AnalogDataCollect() {
#ifndef __APPLE__
  
  DM75xx_Board_Descriptor * brd;
  DM75xx_Error dm75xx_status;
  dm75xx_cgt_entry_t cgt[CHANNELS];
  int i, j;
  float actR;
  uint16_t data = 0x0000;  
  unsigned long int minor_number = 0;

  clog << "info: " << logstream::info << "starting analog acquistion" << std::endl;

  /* Device initialisation */
  dm75xx_status = DM75xx_Board_Open(minor_number, &brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Open");
  dm75xx_status = DM75xx_Board_Init(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Init");
  
  /* Clear the FIFO */
  dm75xx_status = DM75xx_ADC_Clear(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Clear_AD_FIFO");
  dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_FIFO_Get_Status");
  
  /* enable the channel gain table */
  dm75xx_status = DM75xx_CGT_Enable(brd, 0xFF);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_CGT_Enable");
  
  /* set the channel gain table for all channels */
  for (i = 0; i < CHANNELS; i++) {
    cgt[i].channel = i;
    cgt[i].gain = 0;
    cgt[i].nrse = 0;
    cgt[i].range = 0;
    cgt[i].ground = 0;
    cgt[i].pause = 0;
    cgt[i].dac1 = 0;
    cgt[i].dac2 = 0;
    cgt[i].skip = 0;
    dm75xx_status = DM75xx_CGT_Write(brd, cgt[i]);
    DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_CGT_Write");
  }
  
  /* set up clocks */
  dm75xx_status = DM75xx_BCLK_Setup(brd,
				    DM75xx_BCLK_START_PACER,
				    DM75xx_BCLK_FREQ_8_MHZ,
				    BURST_RATE, &actR);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Setup");
  dm75xx_status = DM75xx_PCLK_Setup(brd,
				    DM75xx_PCLK_INTERNAL,
				    DM75xx_PCLK_FREQ_8_MHZ,
				    DM75xx_PCLK_NO_REPEAT,
				    DM75xx_PCLK_START_SOFTWARE,
				    DM75xx_PCLK_STOP_SOFTWARE,
				    PACER_RATE, &actR);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Setup");
  
  /* Set ADC Conversion Signal Select */
  dm75xx_status =
    DM75xx_ADC_Conv_Signal(brd, DM75xx_ADC_CONV_SIGNAL_BCLK);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_ADC_Conv_Signal");
  
  /* Start the pacer clock */
  dm75xx_status = DM75xx_PCLK_Start(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Start");
  
  /* Read data into the FIFO */
  do {
    dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
    DM75xx_Exit_On_Error(brd, dm75xx_status,
			 (char *)"DM75xx_FIFO_Get_Status");
  }
  while (data & DM75xx_FIFO_ADC_NOT_FULL);
  
  /* Stop the pacer clock */
  dm75xx_status = DM75xx_PCLK_Stop(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Stop");
  
  /* Read out data from the FIFO */
  do {
    
    /* Reading the FIFO */
    for (i = 0; i < FIFO_DEPTH; i++) {
      for (j = 0; j < CHANNELS; j++) {
	dm75xx_status = DM75xx_ADC_FIFO_Read(brd, &data);
	DM75xx_Exit_On_Error(brd, dm75xx_status,
			     (char *)"DM75xx_ADC_FIFO_Read");
	this->analog_acq->val[i][j] = ((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10);

	/* Check the FIFO status each time */
	dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
	DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_FIFO_Get_Status");
      }
    }
  }
  while (data & DM75xx_FIFO_ADC_NOT_EMPTY);
  
  /* Print how many samples were received */
  clog << "info: " << logstream::info << "received " << (unsigned)(i * j) << " analog samples" << std::endl;

  /* Reset the board and close the device */
  dm75xx_status = DM75xx_Board_Reset(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Reset");
  dm75xx_status = DM75xx_Board_Close(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Close");

#endif
  return 0;
}



/* get the current light level */
int AnalogManager::GetLightLevel() {

  int i, k;
  float sum_ph[N_CHANNELS_PHOTODIODE];
  float sum_sipm1 = 0;
  auto current_light_level = std::make_shared<LightLevel>();
 
  /* read out the data */
  AnalogDataCollect();
  
  /* interpret the analog acquisition struct */
  /* initialise */
  for(k = 0; k < N_CHANNELS_PHOTODIODE; k++) {
    sum_ph[k] = 0;
  }

  /* read out multiplexed sipm 64 values and averages of sipm 1 and photodiodes */
  for(i = 0; i < FIFO_DEPTH; i++) {

    /* sum the four photodiode channels */
    sum_ph[0] += this->analog_acq->val[i][0];
    sum_ph[1] += this->analog_acq->val[i][1];
    sum_ph[2] += this->analog_acq->val[i][2];
    sum_ph[3] += this->analog_acq->val[i][3];

    /* sum the one channel SiPM values */
    sum_sipm1 += this->analog_acq->val[i][4];
    
    /* read out the multiplexed 64 channel SiPM values */
    {
      std::unique_lock<std::mutex> lock(this->m_light_level);
      this->light_level->sipm_data[i] = this->analog_acq->val[i][5];
    } /* release mutex */
 }

  /* average the photodiode values */
  for (k = 0; k < N_CHANNELS_PHOTODIODE; k++) {
     {
       std::unique_lock<std::mutex> lock(this->m_light_level);
       this->light_level->photodiode_data[k] = sum_ph[k]/FIFO_DEPTH;
     } /* release mutex */
  }
  /* average the one channel SiPM values */
   {
     std::unique_lock<std::mutex> lock(this->m_light_level);
     this->light_level->sipm_single = sum_sipm1/FIFO_DEPTH;
   } /* release mutex */
   
   return 0;
}

/* read light level from object, making an acquisition */
std::shared_ptr<LightLevel> AnalogManager::ReadLightLevel() {
  
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    auto light_level = this->light_level;
  } /* release mutex */
  
 
  return light_level; 
}

/* compare light level to threshold value */
bool AnalogManager::CompareLightLevel() {

  bool above_light_threshold = false;
  float ph_avg = 0;
  int i;
  
  clog << "info: " << logstream::info << "comparing light level to threshold" << std::endl;
  
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    auto light_level = this->light_level;
  } /* release mutex */
  
  
  /* read the light level */
  /* average the 4 photodiode values */
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    for (i = 0; i < N_CHANNELS_PHOTODIODE; i++) {
      ph_avg += light_level->photodiode_data[i];
    }
  } /* release mutex */
  ph_avg = ph_avg/(float)N_CHANNELS_PHOTODIODE;
  
  std::cout << "photodiode average = " << ph_avg << std::endl;
  clog << "info: " << logstream::info << "average photodiode reading is: " << ph_avg << std::endl;
     
  /* compare the result to threshold */
  if (ph_avg > LIGHT_THRESHOLD) {
    above_light_threshold = true;
    clog << "info: " << logstream::info << "light level is ABOVE threshold" << std::endl;
  }
  else { 
    clog << "info: " << logstream::info << "light level is BELOW threshold" << std::endl;
  }
  
  return above_light_threshold;
}

int AnalogManager::ProcessAnalogData() {


  std::unique_lock<std::mutex> lock(this->m_mode_switch);
  /* enter loop while instrument mode switching not requested */
  while(!this->cv_mode_switch.wait_for(lock,
				       std::chrono::milliseconds(WAIT_PERIOD),
				       [this] { return this->inst_mode_switch; })) { 

    this->GetLightLevel();

    sleep(LIGHT_ACQ_TIME);
  }
  return 0;
}


/* reset the mode switching */
int AnalogManager::ResetSwitch() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = false;
  } /* release mutex */
  
  return 0;
}

/* notify the object of an instrument mode switch */
int AnalogManager::NotifySwitch() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = true;
  } /* release mutex */
  
  return 0;
}
