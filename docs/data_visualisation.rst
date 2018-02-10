Data visualisation
==================

ETOT and ETOS
-------------

The standard EUSO tools for quick analysis of the data are developed and maintained by Lech Piotrowski (lech.piotrowski@riken.jp) and the documentation is provided at the following links:

* `Basic functionality <https://jemeuso.riken.jp/lwp/>`_
* `Further documentation <https://jemeuso.riken.jp/lwp/ETOS_manual/>`_

A quick guide for use on the Mini-EUSO data:

* ``ETOT``: to convert from the binary data format output by the CPU software to ``.root`` format, run ``etot -Mff <filename>.dat`` to produce ``<filename>.root``
* ``ETOS``: to open the file with ETOS, once it has been installed, run ``etos.py <filename>.root``


Simple python interface
-----------------------

Raw data files can also be read directly into python for quick plotting, dead time checks and debugging using the code provided `here <https://github.com/cescalara/euso_tools/tree/master/data_visualisation>`_ with some `examples <https://github.com/cescalara/euso_tools/blob/master/data_visualisation/cpu_data_visualisation.ipynb>`_ also included.
