Instrument
==========

The ``instrument`` folder contains code to handle the high-level operation of the instrument and the switching between different operational modes.

Description
-----------

The RunInstrument class handles the overall program flow. When the :cpp:func:`RunInstrument::Start()` function is called, the program checks for the request from the command line inputs to perform simple execute-and-exit commands such as switching on/off a subsystem with :cpp:func:`RunInstrument::LvpsSwitch()` or returning the instrument status with :cpp:func:`RunInstrument::CheckStatus()`. All member functions are documented in detail below. If no such command is required, the program continues to :cpp:func:`RunInstrument::StartUp()` which parses the configuration file and performs standard start up procedures and the to :cpp:func:`RunInstrument::CheckSystems()` which, as may guess, checks the status of the subsystems. RunInstrument then launches two background processes: instrument monitoring (checks for light level and stop signals) with :cpp:func:`RunInstrument::MonitorInstrument()` and data backup using :cpp:func:`UsbManager::RunDataBackup()`.

Once these background processes are launched, the program enters a loop which handles the switching between instrument modes. The initial mode is specified in :cpp:func:`RunInstrument::CheckSystems()` and is entered by calling :cpp:func:`RunInstrument::DayOperations()` or :cpp:func:`RunInstrument::NightOperations()`. These functions then call the relevant :cpp:func:`OperationMode::Start()` member function, which launches all the necessary processes in separate threads. These processes are documented in more detail in their respective class documentation (see DataAquisition and DataReduction). Upon receiving a signal from the :cpp:func:`RunInstrument::MonitorInstrument()` function, :cpp:func:`OperationMode::Notify()` is called and the threads are safely stopped and joined before the :cpp:func:`OperationMode::Start()` function can return. Upon return, the loop continues and the current instrument mode is checked in a switch() statement before the relevant :cpp:func:`OperationMode::Start()` function is called and the process begins again.

The RunInstrument class makes use of signal handling to react to the ``CTRL-C`` (SIGINT) signal. Upon receiving such a signal, all open threads are joined safely or killed in the case of detached threads. Any open data acquisition is stopped and the high voltage to the PMTs is also switched off before the program exits (it can take a few seconds).


RunInstrument
-------------

.. doxygenclass:: RunInstrument
   :path: ../CPU/CPUsoftware/doxygen/xml
   :members:
   :private-members:
