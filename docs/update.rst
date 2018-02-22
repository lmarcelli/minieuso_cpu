Update
======

To update the software following installation: 

1. **Login as root** using ``su -l`` and the same password as for minieusouser
   
2. Connect to the internet 

3. Run ``git pull`` from the command line within the ``/home/software`` directory

4. Run ``make`` inside ``/home/software/CPU/CPUsoftware``

5. The executable ``mecontrol`` will now be available for use

If there are any conflicts when running ``git pull``, resolve these conflicts as desired by editing the files and then use ``git commit`` to store the changes before trying again.
