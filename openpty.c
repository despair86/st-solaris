
/* Open a pseudo-terminal.
   Copyright (C) 2010-2011 Free Software Foundation, Inc.
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

// A simplfied openpty for Solaris 2.10+. -rick
// This module was imported from Gnulib: as such, it remains under the LGPL

// Naturally, _GNU_SOURCE doesn't exist here.
#define __EXTENSIONS__

# include <fcntl.h>
# include <string.h>
# include <sys/ioctl.h>
# include <termios.h>
# include <unistd.h>
# include <stropts.h>

int
openpty (int *amaster, int *aslave, char *name,
         struct termios const *termp, struct winsize const *winp)
{
  int master;
  char *slave_name;
  int slave;

  master = posix_openpt (O_RDWR | O_NOCTTY);
  if (master < 0)
    return -1;

  if (grantpt (master))
    goto fail;

  if (unlockpt (master))
    goto fail;
  
  slave_name = ptsname (master);
  if (slave_name == NULL)
      goto fail;
  
  slave = open (slave_name, O_RDWR | O_NOCTTY);
  if (slave == -1)
    goto fail;

  if (ioctl (slave, I_PUSH, "ptem") < 0
      || ioctl (slave, I_PUSH, "ldterm") < 0
      || ioctl (slave, I_PUSH, "ttcompat") < 0
     )
    {
      close (slave);
      goto fail;
    }

  /* XXX Should we ignore errors here?  */
  if (termp)
    tcsetattr (slave, TCSAFLUSH, termp);
  if (winp)
    ioctl (slave, TIOCSWINSZ, winp);

  *amaster = master;
  *aslave = slave;
  if (name != NULL)
    strcpy (name, slave_name);

  return 0;

 fail:
  close (master);
  return -1;
}