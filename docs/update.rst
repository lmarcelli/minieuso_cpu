Update
======

To update the software following installation: 

1. **Login as root** using ``su -l`` and the same password as for minieusouser
   
2. Connect to the internet 

3. Run ``git pull`` from the command line within the ``/home/software`` directory

4. Run ``make clean`` and then ``make`` inside ``/home/software/CPU/CPUsoftware``

5. The executable ``mecontrol`` will now be available for use

If there are any conflicts when running ``git pull``, resolve these conflicts as desired by editing the files and then use ``git commit`` to store the changes before trying again.

Note: The full software takes some time to compile on the CPU, but the makefile contains dependency checks and will only recompile what is needed

To switch over to a new branch (testing new features)

1. **Login as root** using ``su -l`` and the same password as for minieusouser
   
2. Connect to the internet 

3. Run ``git pull`` from the command line within the ``/home/software`` directory

4. Run ``make clean`` inside ``/home/software/CPU/CPUsoftware`` and ``/home/software/CPU/CPUsoftware/lib``

5. Run ``git checkout <branch_name>`` (after, you can check which branch you are on using ``git branch``)

6. Recompile by running ``make`` inside *first* ``/home/software/CPU/CPUsoftware/lib`` and *then* ``/home/software/CPU/CPUsoftware/``
   
7. The correct executable ``mecontrol`` will now be available for use
 
In both cases, the command ``mecontrol -ver`` can be used to check the currently compiled version of the software.


Problems?
---------

If there are any conflicts when running ``git pull``, resolve these conflicts as desired by editing the files and then use ``git commit`` to store the changes before trying again.

Local files can also be completely overwritten by the remote using ``git reset --hard origin/master``.



If, for whatever reason, the ``/home/software`` git repository is deleted, it is necessary to clone again::

  git clone https://github.com/cescalara/minieuso_cpu /home/software

Following this, the camera software must be compiled, the necessary libraries built and the log directories created. There is a script to do this in ``/home/
software/CPU/CPUsetup`` called ``reinstall.sh``.

**NB: when running reinstall.sh, it is important that both cameras are connected and powered so that the cameras.ini file can be created for the multiplecam software**
