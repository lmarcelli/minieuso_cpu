
Automated acquisition
=====================

In order to run ``mecontrol`` automatically on boot, simply edit the ``~/.profile`` (make sure you are logged in as **root**) and add the command that you need.

An example ``~/.profile``::

  # ~/.profile: executed by Bourne-compatible login shells.

  if [ "$BASH" ]; then
      if [ -f ~/.bashrc ]; then
          . ~/.bashrc
      fi
  fi

  mesg n

  # mecontrol acquisition options, uncomment desired command
  #mecontrol -log -zynq periodic -short 10
  #mecontorl -log -zynq self


See the `usage <http://minieuso-software.readthedocs.io/en/latest/usage.html>`_ section of this documentation for all the different ``mecontrol`` options.

