USB
===

Description
-----------

Mini-EUSO will store data on USB flash drives which will be sent to Earth periodically during its mission. USB storage will also be used to pass configuration files to Mini-EUSO and quick-look diagnostic data back to Earth via the ISS internet connection.

UsbManager makes use of the ``libusb`` package to identify USB devices and handle automated data backups using ``rsync`` whenever the instrument is switched on. 

UsbManager
----------

.. doxygenclass:: UsbManager
   :path: ../CPU/CPUsoftware/doxygen/xml
   :members:
   :private-members:
