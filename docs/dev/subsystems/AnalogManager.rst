Analog board
============

Description
-----------

The AnalogManager handles the acquisition of data from the photodiodes and SiPMs. Data is acquired using ``AnalogManager::GetLightLevel()`` member function. This function uses the dm75xx library to handle the analog inputs on a separate board that is directly connected to the CPU board via the PCIe bus. Due to the nature of the dm75xx library this function can ONLY be run in a joinable thread, and in one thread at a time (without addition of mutex). In this way, this function is called in a separate joinable thread via the AnalogManager by both OperationMode daughter classes when they are operating to update the photodiode measurements periodically. Functions like ``RunInstrument::PollLightLevel()`` on make use of the ``AnalogManager::ReadLightLevel()`` to read out the current stored LightLevel in the AnalogManager object in a thread safe way. The ``AnalogManager::CompareLightLevel()`` handles the comparison of the average photodiode reading with a set threshold in order to set the mode to day/night. This threshold is stored in ``AnalogManager.h`` as ``LIGHT_THRESHOLD`` and must be updated as the instrument calibration is carried out. The pinout of the analog board connected is described in the hardware interfaces section of the documentation. 


AnalogManager
-------------

.. doxygenclass:: AnalogManager
   :members:
   :private-members:
