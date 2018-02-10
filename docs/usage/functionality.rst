Functionality
=============


Control of the high voltage (HV)
--------------------------------

The CPU controls the HV sent to the PMTs via the Zynq board. 

**Important: always ensure that the instrument is in a dark, light-tight environment and that all LEDs on the instrument itself are covered or disabled before switching on the HV. Similarly, ensure the HV is switched off before allowing light on the instrument.**

* The voltage is automatically ramped up in steps of ~ 140 V from 0 V to the desired operating voltage
* The HV can be turned on in 2 different ways
  
  * ``mecontrol -hvps on -dv <X> -asicdac <Y>`` switches on the HV then exits the program, to allow for other tests
  * ``mecontrol -hv -dv <X> -asicdac <Y>`` starts an acquisition, turning the HV on and off automatically
     
* Explanation of the command flags

  * the flag ``-dv <X>`` sets the dynode voltage to X, where X is the DAC between 0 and 4096 
  * the conversion between this number ``<X>`` and a voltage is ``<X>/4096 * 2.44 * 466``, ie. ``DAC/maxDAC * maxDAC_voltage * 466``
  * the flag ``-asicdac <Y>`` sets the ASIC dac level, or the point in an S-curve that we want to use to acquire data (should be in the S-curve plateau)
  * the ``-asicdac`` flag was previously ``-hvdac`` but this was changed to avoid confusion, however, for the sake of compatibility, ``-hvdac`` can still be used with identical functionality
  * if these flags are not supplied, their default values are used from the configuration file in ``CPUsoftware/config``

* To check the current status, use ``mecontrol -check_status``
* If an acquisition with HV is interrupted using ``CTRL-C``, the HV will be switched off automatically

  
Control of the low voltage power supply (LVPS)
----------------------------------------------

The CPU controls the powering of the other sub-systems via the LVPS.
* The main data acquisition program will automatically turn on/off subsystems as they are needed
* There are also commands to switch on and off subsystems, then exit the program

  * ``mecontrol -lvps on -subsystem <S>`` will switch on ``<S>`` which can be ``zynq``, ``cam`` or ``hk``
  * ``mecontrol -lvps off -subsystem <S>`` will switch off the specified sub-system

    
Data acquisition
----------------

The CPU handles the data acquisition from all subsystems. 

* Summary of main command options

  * ``-scurve``: take a single S-curve and exit
  * ``-short``: take a single file (~ 2min) acquisition and exit
  * ``-zynq <MODE>``: use the Zynq acquisition mode (``<MODE>`` = ``0``, ``1``, ``periodic``, ``trigger``, default = ``periodic``)
  * ``-test_zynq <MODE>``: use the Zynq test mode (``<MODE>`` = ``0`` - ``6``, default = ``3``)
  * ``-keep_zynq_pkt``: keep the Zynq packets on FTP

* An example use case: ``mecontrol -log -test_zynq 3 -keep_zynq_pkt`` would start and acquisition in Zynq test mode 3 and keep the Zynq packets on the FTP server to check them

* Data from the PDM is collected as specified by the command line options, packets are sent from the Zynq every 5.24s with 3 levels of data (D1, D2 and D3) and information on timestamping and the HV status. 

  * The number of packets for D1 and D2 data, N1 and N2, can be set in the configuration file
  * Analog and housekeeping data are also gathered every 5.24 s and packaged together with the Zynq data into a CPU packet
  * One CPU packet is appended to the current CPU run file every 5.24s
  * Data from the cameras is collected independently with both cameras operating simultaneously

* if no USB storage is available, the output data from the CPU is in ``/home/minieusouser/DONE`` with filenames of the form ``CPU_RUN_<run_type>__<current_date>__<current_time>.dat``

  * S-curve files also have the configured dynode voltage appended to the filename, even if the HV is not switched on 
  * the data format of these files is documented in ``CPUsoftware/src/data_format/data_format.h`` 
  * log files are in ``CPUsoftware/log``, if log output is switched on with ``-log``

* the output data from the cameras is in ``cameras/multiplecam/<NIR/VIS>/<current_date>``

  * .raw for the photos from the cameras
  * log files are in ``cameras/multiplecam/log/``

* if USB storage is detected on the system, the output files will be automatically written there instead, and backed up if there is more than one device.

   
