/*  nanowasp, an emulator for the microbee 128k
 *  Copyright (C) 2000-2003  David G. Churchill <froods@alphalink.com.au>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *
 *  Floppy Disk Support Module
 *
 *
 *  This modules implements functions to represent the actual physical
 *  disks.  It consists of support functions that manipulate the
 *  disk image files.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "disk.h"



int disk_open(const char *name, disk_t *disk)
{

   assert(disk != NULL);

   disk->fdisk = fopen(name, "rb");
   if (disk->fdisk == NULL)
      return -1;

   disk->wrprot = 1;

   return 0;
}



void disk_close(disk_t *disk)
{
   assert(disk != NULL);

   fclose(disk->fdisk);
   disk->fdisk = NULL;
}


int disk_create(const char *name, disk_t *disk)
{
   return -1;
}

 
int disk_read(disk_t *disk, char *buf, int max_size, 
              int side, int track, int sect)
{
   /* FIXME: maxsize */
   
   assert(disk != NULL);
   assert(disk->fdisk != NULL);

   fseek(disk->fdisk, ((side * 40 + track) * 10 + (sect - 1)) * 512, SEEK_SET);
   fread(buf, 512, 1, disk->fdisk);

   return 1;
}



int disk_write(disk_t *disk, char *buf, int size, int track, int sect)
{
   return -1;
}


/* int disk_getsecthdr(...) ?? */


int disk_iswrprot(disk_t *disk)
{
   if (disk == NULL)
      return 0;

   return disk->wrprot;
}

