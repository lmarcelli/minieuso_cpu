Cameras
=======

Description
-----------

Mini-EUSO is equipped with two ancillary cameras to provide complementary measurements in different wavelength bands (NIR and VIS). The main camera software, ``multiplecam``, is stored in ``CPU/cameras/multiplecam`` and is written and maintained by Sara Turriziani (sara.turriziani@gmail.com). Some documentation can be found in ``CPU/cameras/multiplecam/doc``.

The CamManager class is an interface to this software and handles the lauching of the camera software in a robust way, and the parallel operation of the cameras with respect to the other instrument subsystems.


CamManager
----------

.. doxygenclass:: CamManager
   :members:
   :private-members:
