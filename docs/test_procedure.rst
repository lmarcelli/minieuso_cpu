Test procedure
==============

General guidelines for tests concerning the Mini-EUSO instrument.

Initial set-up
--------------

* Check all sub-systems are connected correctly and that the grounding is proper
* Document the state of the instrument (preferably with pictures)
* Keep a log of tests carried out in addition to the log files provided by the CPU, although these should also be saved (there is a section dedicated to `integration and testing <https://jemeuso.riken.jp/wiki/index.php?Mini-EUSO%20integration%20and%20testing>`_ on the Mini-EUSO wiki)
* Before starting tests run ``mecontrol -help``, this calls :cpp:func:`InputParser::PrintHelpMsg()` function to see all available commands
* Read the `usage documentation <http://minieuso-software.readthedocs.io/en/latest/usage.html>`_ pages to understand the operation of the instrument and what these commands do
* Start by running ``mecontrol -db`` to run a debug program of the main subsystems and check everything is working as expected, this calls the :cpp:func:`RunInstrument::DebugMode()` function
* Make sure to use the ``mecontrol -comment "..."`` command to add a comment to both the filename and the :cpp:class:`CpuFileHeader`

  
Connecting to Zynq
------------------
* The ``mecontrol`` software automatically switches on the Zynq and waits for it to boot when performing an acquisition. However, in the past there have been some issues which require a Zynq reboot, for which the commands ``mecontrol -lvps off -subsystem zynq`` and ``mecontrol -lvps on -subsystem zynq`` can be used
* If there are problems with the automatic connection of the Zynq by the ``mecontrol`` software, try rebooting the Zynq and checking for ``ping 192.168.7.10`` to return with 0% packet loss before trying the ``mecontrol`` software again.
  
  
Tests with HV
-------------

**Always take care when using the HV that the instrument is in proper darkness (black box) and that all LEDs are disabled or removed**

* Once satisfied that the grounding is correct and the instrument is in proper darkness, start by switching the HV on to < 300 V using, for example, ``mecontrol -hvswitch on -hv all -dv 1000`` to ramp up the HV dynode voltage. The cathode voltage is always set to the 900 V mode (other modes are 0 V and 750 V). This is done with :cpp:func:`ZynqManager::HvpsTurnOn()`.
* The status can be further checked using the ``mecontrol -check_status`` command (:cpp:func:`RunInstrument::CheckStatus()`)
* The HVPS status returns 9 numbers, corresponding to each EC unit:

  * 0: the EC unit is off
  * 1: the EC unit is on, but status is undefined (can be switched down)
  * 2: the EC unit is on, but not at the correct voltage (switched down)
  * 3: the EC unit is switched on and at the correct voltage

* For the engineering model, only one EC is connected and thus the unconnected ECs will return undefined results
* When the HV is behaving as expected with simple status checks, it is suitable to proceed with raising the HV dynode voltage to the desired level
* To take an acquisition with the HV on use, for example::

    mecontrol -log -hv all -dv 3000 -asicdac 500 -zynq periodic

  * The asic dac must be set when taking an acquisition, otherwise, the value in the configuration file will be used by default
  * This is the ASIC threshold setting, and should correspond to the single photoelectron plateau of the S-curve

* To take an S-curve with the HV on run::

    mecontrol -log -hv all -dv 3000 -scurve

  * There is no need to specify the asicdac for an S-curve, all thresholds will be scanned

* Ensure the HV is switched off before exposing the instrument to high light levels by checking that the ``mecontrol -check_status`` returns::

    HVPS status: 0 0 0 0 0 0 0 0 0
  
NB: If EC units are not connected, their status will be undefined and can return non-zero values. In this case use the EC by EC commands described `here <http://minieuso-software.readthedocs.io/en/latest/usage/functionality.html#control-of-the-high-voltage-hv>`_.

