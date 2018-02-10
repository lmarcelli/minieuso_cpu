Test procedure
==============

General guidelines for tests concerning the Mini-EUSO instrument.

Initial set-up
--------------

* Check all sub-systems are connected correctly and that the grounding is proper
* Document the state of the instrument (preferably with pictures)
* Keep a log of tests carried out in addition to the log files provided by the CPU, although these should also be saved (there is a section dedicated to `integration and testing <https://jemeuso.riken.jp/wiki/index.php?Mini-EUSO%20integration%20and%20testing>`_ on the Mini-EUSO wiki)
* Before starting tests run ``mecontrol -help`` and read the usage documentation pages to understand the operation of the instrument
* Start by running ``mecontrol -db`` to run a debug program of the main subsystems and check everything is working as expected

Tests with HV
-------------

**Always take care when using the HV that the instrument is in proper darkness (black box) and that all LEDs are disabled or removed**

* Once satisfied that the grounding is correct and the instrument is in proper darkness, start by switching the HV on to < 300 V using, for example, ``mecontrol -hvps on -dv 1000`` to ramp up the HV dynode voltage. The cathode voltage is always set to the 900 V mode (other modes are 0 V and 750 V).
* The status can be further checked using the ``mecontrol -check_status`` command
* The HVPS status returns 9 numbers, corresponding to each EC unit:

  * 0: the EC unit is off
  * 1: the EC unit is on, but status is undefined (can be switched down)
  * 2: the EC unit is on, but not at the correct voltage (switched down)
  * 3: the EC unit is switched on and at the correct voltage

* For the engineering model, only one EC is connected and thus the unconnected ECs will return undefined results
* When the HV is behaving as expected with simple status checks, it is suitable to proceed with raising the HV dynode voltage to the desired level
* To take an acquisition with the HV on use, for example::

    mecontrol -log -hv -dv 3000 -asicdac 500

  * The asic dac must be set when taking an acquisition, otherwise, the value in the configuration file will be used by default
  * This is the ASIC threshold setting, and should correspond to the single photoelectron plateau of the S-curve

* To take an S-curve with the HV on run::

    mecontrol -log -hv -dv 3000 -scurve

  * There is no need to specify the asicdac for an S-curve, all thresholds will be scanned

* Ensure the HV is switched off before exposing the instrument to high light levels by checking that the ``mecontrol -check_status`` returns ``HVPS status: 0 0 0 0 0 0 0 0 0``
  
    

