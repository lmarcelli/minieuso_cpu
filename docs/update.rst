Update
======

To update the software following installation: 

1. **Login as root** using ``su -l`` and the same password as for minieusouser
   
2. Connect to the internet 

3. Run ``git pull`` from the command line within the ``/home/software`` directory

4. Run ``make`` inside ``/home/software/CPU/CPUsoftware``

5. The executable ``mecontrol`` will now be available for use

If there are any conflicts when running ``git pull``, resolve these conflicts as desired by editing the files and then use ``git commit`` to store the changes before trying again.


Problems?
---------

If there are any conflicts when running ``git pull``, resolve these conflicts as desired by editing the files and then use ``git commit`` to store the changes before trying again.

Local files can also be completely overwritten by the remote using ``git reset --hard origin/master``.



If, for whatever reason, the ``/home/software`` git repository is deleted, it is necessary to clone again::

  git clone https://github.com/cescalara/minieuso_cpu /home/software

Following this, the camera software must be compiled, the necessary libraries built and the log directories created. There is a script to do this in ``/home/
software/CPU/CPUsetup`` called ``reinstall.sh``.

**NB: when running reinstall.sh, it is important that both cameras are connected and powered so that the cameras.ini file can be created for the multiplecam software**
