DM75xx ports
============

The DM75xx series board is used in addition to the main CPU board to handle the analog acquisition. An external 68 pin I/O connector (CN3) is used to interface to the analog signals, but only the utlised channels are shown here. These channels collect data from the SiPMs and the photodiodes. The table is laid out on the same way as the pins so the left and right halves are independent.

+----------+----------------+-----------+----------+----------------+-----------+
| Pin      | Analog channel | Function  | Pin      | Analog channel | Function  | 
+==========+================+===========+==========+================+===========+
| **1**    | 1              | PH 1.1    | **3**    | 2              | PH 2.1    |
+----------+----------------+-----------+----------+----------------+-----------+
| **5**    | 3              | PH 1.2    | **7**    | 4              | PH 2.2    |
+----------+----------------+-----------+----------+----------------+-----------+
| **11**   | 5              | SiPM 1    | **13**   | 6              | SiPM 64.1 |
+----------+----------------+-----------+----------+----------------+-----------+
| **15**   | 7              | SiPM 64.2 | **17**   | 8              | SiPM 64.3 |
+----------+----------------+-----------+----------+----------------+-----------+
| **9**    |                | AINSENSE  | **10**   |                | AGND      |
+----------+----------------+-----------+----------+----------------+-----------+


*NB: The software is currently set up to read out only SiPM 64.1 64 times (through multiplexer)*
