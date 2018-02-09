Operations
==========

Description
-----------

The OperationMode base class is defined in order to ease the addition and handling of instrument modes as the automated mode switching interface is implemented and the necessary mutex and condition variables and protected as private members. The DataAquisition and DataReduction classes both inherit from this class.

The DataAcquisition class describes the night-time operational mode of the instrument which is driven by data acquisition. The key function is ``DataAcquisition::CollectData()``, which spawns all the necessary data acquisition processes including ``DataAcquisiton::ProcessIncomingData()`` which watches the FTP directory for new files from the Zynq board and processes them. The main data acquisition is Synchronous. The PDM raw data is sent in a ``ZYNQ_PACKET`` (see the ``data_format.h`` for definition) every 5.24 s (corresponding to 128*128*128 GTU). When a new ``ZYNQ_PACKET`` is detected, the program also reads out the photodiodes and SiPM using AnalogManager and collects all this information into a ``CPU_PACKET`` which is written to the current CPU file. There is also so asynchronous acquisition from the thermistors via the ThermManager class, which pass a ``THERM_PACKET`` to the active CPU file once a minute. The cameras also operate asynchronously and their pictures are stored separately. There are also other operational modes, and the details of the acquisition are specified by command line inputs to the program.

The DataReduction class is designed to perform useful data reduction tasks during the day when data cannot be collected. Tasks involve data compression and production of small quick-look data samples that can be quickly sent down to Earth by the working astronauts to allow for a check of the instrument operating correctly. These tasks are yet to be implemented and currently the program just sleeps in day mode, however the framework is set up for the easy addition of this functionality.

OperationMode
-------------

.. doxygenclass:: OperationMode
   :members:
   :protected-members:

