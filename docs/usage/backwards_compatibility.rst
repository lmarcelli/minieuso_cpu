Backwards compatibility
=======================

Stable versions of the software used in previous integration tests of the Mini-EUSO instrument are stored in the following branches named after the integration date. The current stable version of the software is in the ``master`` branch.


1. ``aug_06_2017``: August integration in Tor Vergata, Rome
2. ``oct_16_2017``: October integration in Tor Vergata, Rome
3. ``dec_17_2017``: December integration in Tor Vergata, Rome

Some key differences of the previous software with the current version:

* ``aug_06_2017``

  * The CPU generates only one run file, with the S-curve packet stored first, followed by RUN_SIZE CPU packets. 
  * S-curves are gathered from DAC 0 - 1000 (inclusive), with a step size of 1 and an accumulation of 1.
  * S-curve accumulation is not calculated, the frames are simply stored for post-processing

* ``oct_16_2017``

  * no LVPS control
  * no variable N1 and N2

* ``dec_16_2017`` 

  * no automated instrument mode switching
  * no LVPS status checking
  * less robust checks on subsystems
  * different class structure
