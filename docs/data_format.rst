Data format
===========

PDM data is acquired, triggered and time-stamped in the Zynq board. This data is then passed to the CPU. The CPU also acquires data from the other subsystems and packages this together with the PDM data during nominal night-time observations. Data is acquired every 5.24 s (128 x 128 x 128 x 1 GTU, 1 GTU = 2.5 us). The CPU generates 3 types of files, ``CPU_RUN_MAIN`` containing the standard data acquistion, ``CPU_RUN_SC`` for S-curve data and ``CPU_RUN_HV`` for HV data. All data files have a matryoshka structure that is summarised below.

The CPU_RUN file
----------------

Files named ``CPU_RUN_....dat`` are the output files of the Mini-EUSO software. Information is what this file contains is stored in the :cpp:class:`CpuFileHeader` which is put together at runtime. This structure has the following fields:

* ``spacer``: a HEX ID tag 0xAA55AA55 for easy checking of the files in a hex viewer
* ``header``: a set of bytes defining the instrument, file type and file version (they types and versions are defined in ``minieuso_data_format.h`` and the header is put together in :cpp:func:`DataAcquisition::BuildCpuFileHeader`)
* ``run_info``: a text field containing the information on the command line options and configuration at runtime (put together in :cpp:func:`DataAcquisition::BuildCpuFileInfo`)
* ``run_size``: the number of :cpp:class:`CPU_PACKET` in the standard acquisition run 

The file is closed with a :cpp:class:`CpuFileTrailer`. This also contains the ``spacer`` and ``run_size`` fields as well as ``crc`` which stores a 32 bit CRC, calcluated over the whole file *excluding* the trailer and appended using :cpp:func:`SynchronisedFile::Checksum()` accessed through :cpp:func:`Access::GetChecksum()` within :cpp:func:`DataAcquisition::CloseCpuRun`.


1. The ``CPU_RUN_MAIN`` file format
   
.. image:: /images/cpu_format.png

At present the Zynq data format is fixed so that each :cpp:class:`ZYNQ_PACKET` contains:

* ``N1``: the number of D1 packets 
* ``N2``: the number of D2 packets  
* ``level1_data``: ``N1`` x 128 GTU packet of D1 data (1 byte/pixel)
* ``level2_data``: ``N2`` x 128 GTU_L2 packet of D2 data (2 bytes/pixel) 
* ``level3_data``: 1 x 128 GTU_L3 packet of D3 data (4 bytes/pixel)

The data format holds for both triggered and non-triggered readout.

2. The ``CPU_RUN_SC`` file format

.. image:: /images/sc_data_format.png

The ``CPU_RUN_SC`` has a fixed size which represents the maximum number of threshold steps (0 - 1023). For S-curves taken over a smaller threshold ranges, the file is simply padded with the value ``0xFFFFFFFF``. S-curve accumulation is calculated on-board the Zynq FPGA using the HLS scurve_adder (https://github.com/cescalara/zynq_ip_hls) allowing for S-curves to be taken with high statistics and stored in a small file size. 

3. The ``CPU_RUN_HV`` file format

This file also has a fixed size and is used to store information on the HV status at the end of a run. This information is additional and complementary to that stored inside the :cpp:class:`ZYNQ_PACKET`.

The format is described in detail by the two header files ``minieuso_pdmdata.h`` (the Zynq data format - depends on the firmware version) and ``minieuso_data_format.h`` (the CPU data format - depends on the CPU software version). The ``minieuso_data_format.h`` file is documented below.

A 32 bit CRC is calculated for each ``CPU_RUN`` file prior to adding the CpuFileTrailer (the last 10 bytes). This CRC is appended to each ``CPU_RUN`` file as part of the CpuFileTrailer. 


minieuso_data_format.h
----------------------

.. doxygenfile:: minieuso_data_format.h
   :path: ../CPU/CPUsoftware/doxygen/xml
