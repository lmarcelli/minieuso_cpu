Thermistors
===========

Description
-----------

The acquisition of the thermistors is controlled via the ``digitemp`` software (https://github.com/bcl/digitemp). The command is called every ``THERM_ACQ_SLEEP`` seconds and the resulting output is parsed into an array of temperature data. This data is then written to the CPU run file opened by the main acquisition (``DataAcquisition::CreateCpuRun``) asynchronously in the form of a ``THERM_PACKET``.

ThermManager
------------

.. doxygenclass:: ThermManager
   :members:
   :private-members:
