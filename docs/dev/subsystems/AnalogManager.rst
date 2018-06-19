Analog board
============

Description
-----------

The :cpp:class:`AnalogManager` handles the acquisition of data from the photodiodes and SiPMs. Data is acquired using :cpp:func:`AnalogManager::GetLightLevel()` member function. This function uses the dm75xx library to handle the analog inputs on a separate board that is directly connected to the CPU board via the PCIe bus. Due to the nature of the dm75xx library this function can ONLY be run in a joinable thread, and in one thread at a time (without addition of mutex). In this way, this function is called in a separate joinable thread via the AnalogManager by both OperationMode daughter classes when they are operating to update the photodiode measurements periodically. Functions like :cpp:func:`RunInstrument::PollLightLevel()` on make use of the :cpp:func:`AnalogManager::ReadLightLevel()` to read out the current stored LightLevel in the AnalogManager object in a thread safe way. The :cpp:func:`AnalogManager::CompareLightLevel()` handles the comparison of the average photodiode reading with a set threshold in order to set the mode to day/night. This threshold is stored in ``AnalogManager.h`` as ``LIGHT_THRESHOLD`` and must be updated as the instrument calibration is carried out. The pinout of the analog board connected is described in the hardware interfaces section of the documentation. 

Reading photodiodes/SiPMs
-------------------------

The analog channels are always read out, it is not necessary to add a flag to the main ``mecontrol`` executable. If no sensors are connected, the values on all channels should be ``-5``. If sensors are connected, some positive value should be read out. The exact value and its units will depend on the eventual calibration and light level. In the past, a small level of cross talk has been observed, with neighbouring channels showing ``-X``, where -5 < ``X`` < 0. This should be no problem as we can add a threshold for negative values. 

Connection the photodiodes/SiPMs
--------------------------------

Please refer to the `hardware interfaces/DM55xx ports <http://minieuso-software.readthedocs.io/en/latest/hardware/dm75xx_ports.html>`_ section for the connector pinout. 


AnalogManager
-------------

.. doxygenclass:: AnalogManager
   :path: ../CPU/CPUsoftware/doxygen/xml
   :members:
   :private-members:
