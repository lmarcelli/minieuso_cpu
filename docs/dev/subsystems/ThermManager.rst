Thermistors
===========

Description
-----------

The acquisition of the thermistors is controlled via the ``digitemp`` software (https://github.com/bcl/digitemp). The command is called every ``THERM_ACQ_SLEEP`` seconds and the resulting output is parsed into an array of temperature data. This data is then written to the CPU run file opened by the main acquisition (:cpp:func:`DataAcquisition::CreateCpuRun`) asynchronously in the form of a ``THERM_PACKET``.

Reading out the thermistors
---------------------------

The thermistors will be read out asynchronously if the ``-therm`` flag is passed to the main ``mecontrol`` executable. If no thermistors are connected, a temperature of ``99`` will be read out for debugging purposes. If thermistors are connected, some sensible temperature should be read out. The unit of the measurements is degrees celcius and the range of the sensors is from -50 to +70. See the `digitemp <https://github.com/bcl/digitemp>`_ software for more information. 


ThermManager
------------

.. doxygenclass:: ThermManager
   :path: ../CPU/CPUsoftware/doxygen/xml
   :members:
   :private-members:
