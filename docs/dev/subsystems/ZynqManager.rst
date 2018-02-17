Zynq board
==========

Description
-----------

The :cpp:class:`ZynqManager` class holds information on the current status of the Zynq in its public member variables. The majority of the member functions make use of socket programming to communicate with the Zynq board on the ``ZYNQ_IP`` and ``TELNET_PORT`` defined in the header file. The Zynq has several different operational modes, the key modes will be described here and for further details the reader is directed to the Zynq board documentation written and maintained by Alexander Belov (aabcad@gmail.com).

The Zynq data acquisition modes are documented `here <http://minieuso-software.readthedocs.io/en/latest/usage/functionality.html#zynq-acquisition-modes>`_

In addition to the standard data acquisition, the Zynq can also provide S-curves. An S-curve is made by sweeping the ASIC thresholds whilst collecting data and can be used to fully characterise the PMTs, making it a powerful diagnostic tool. The :cpp:func:`ZynqManager::Scurve()` takes an S-curve with the desired parameters which are passed from the configuration file by RunInstrument and DataAcquisition. S-curves can be requested from the main program by using the ``mecontrol -scurve`` command line argument.

As well as data acquisition the Zynq also handles the interface to the high voltage (HV) which is needed by the PMTs of Mini-EUSO. :cpp:func:`ZynqManager::HvTurnOn()` is used to ramp-up the high voltage in safe steps to the desired operational level. Whenever the program is interrupted with ``CTRL-C`` (SIGINT), the :cpp:func:`ZynqManager::HvTurnOff()` is called to ensure the HV is switched off before the program exits.

ZynqManager
-----------

.. doxygenclass:: ZynqManager
   :path: ../CPU/CPUsoftware/doxygen/xml
   :members:
   :private-members:

