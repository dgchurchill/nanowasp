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
 *  FDC Module
 *
 *
 *  Emulates the 2793 floppy disk controller.
 *
 *  Todo:
 *     - write support
 *     - verify flag support (type I commands)
 *
 *
 *  notes:  if no disk in drive, index pulse is always on.
 */


#include <stdio.h>
#include <stdlib.h>

#include "fdc.h"
#include "disk.h"
#include "nanowasp.h"



#define FDC_RESTORE      0       /* type I */
#define FDC_SEEK         1
#define FDC_STEP         2
#define FDC_STEPIN       4
#define FDC_STEPOUT      6

#define FDC_READSECT     8       /* type II */
#define FDC_WRITESECT    10

#define FDC_READADDR     12      /* type III */
#define FDC_READTRACK    14
#define FDC_WRITETRACK   15

#define FDC_INTERRUPT    13      /* type IV */


#define FDC_NOTREADY     0x80    /* status port bits */
#define FDC_WRPROT       0x40
#define FDC_HEADLOADED   0x20
#define FDC_RECTYPE      0x20
#define FDC_SEEKERROR    0x10
#define FDC_RECNOTFOUND  0x10
#define FDC_CRCERROR     0x08
#define FDC_TRACK0       0x04
#define FDC_LOSTDATA     0x04
#define FDC_INDEXPULSE   0x02
#define FDC_DRQ          0x02
#define FDC_BUSY         0x01

#define FDC_STEPRATE     0x03    /* fdc command data bits */
#define FDC_VERIFY       0x04
#define FDC_LOADHEAD     0x08
#define FDC_UPDATETRACK  0x10
#define FDC_MULTISECT    0x10
#define FDC_SIDE         0x08
#define FDC_DELAY        0x04
#define FDC_CMPSIDE      0x02
#define FDC_DATAMARK     0x01
#define FDC_INTREADY     0x01
#define FDC_INTNOTREADY  0x02
#define FDC_INTINDEX     0x04
#define FDC_INTIMMED     0x08


#define FDC_MAXTRACK     39
#define FDC_BUFSIZE      10240


struct fdc_drive_s
{
   int track;     /* current track position of the head */
   disk_t *disk;
};

typedef struct fdc_drive_s fdc_drive_t;



static int ctrl_side, ctrl_drive, ctrl_ddense;
static int ctrl_rdata, ctrl_rtrack, ctrl_rsect,
           ctrl_intrq, ctrl_drq, ctrl_status,
           ctrl_stepdir;


static fdc_drive_t fdc_drive[FDC_NUMDRIVES];

static char buf[FDC_BUFSIZE];
static int bytes_left, buf_index;



int fdc_init(void)
{
   int i;


   for (i=0; i<FDC_NUMDRIVES; i++)
      fdc_drive[i].disk = NULL;

   fdc_loaddisk("bootdisk.img", 0);

   return 0;
}


int fdc_deinit(void)
{
   int i;


   for (i=0; i<FDC_NUMDRIVES; i++)
      fdc_unloaddisk(i);


   return 0;
}


int fdc_reset(void)
{
   ctrl_side = ctrl_drive = ctrl_ddense = 0;
   ctrl_rtrack = ctrl_intrq = ctrl_drq = 0;
   ctrl_stepdir = +1;

   fdc_drive[ctrl_drive].track = 0;
   ctrl_status = FDC_TRACK0;
   if (fdc_drive[ctrl_drive].disk == NULL)
      ctrl_status |= FDC_INDEXPULSE;  /* set index pulse if no disk */
   if (disk_iswrprot(fdc_drive[ctrl_drive].disk))
      ctrl_status |= FDC_WRPROT;


   bytes_left = 0;

   return 0;
}



int fdc_loaddisk(const char *name, int drive)
{
   if (fdc_drive[drive].disk != NULL)
      fdc_unloaddisk(drive);

   fdc_drive[drive].disk = malloc(sizeof(disk_t));
   if (fdc_drive[drive].disk == NULL)
   {
      perror("loaddisk");
      exit(1);
   }

   if (disk_open(name, fdc_drive[drive].disk) != 0)
   {
      fprintf(stderr, "fdc_loaddisk: unable to load %s\n", name);
      return -1;
   }

   return 0;
}


void fdc_unloaddisk(int drive)
{
   if (fdc_drive[drive].disk == NULL)
      return;

   disk_close(fdc_drive[drive].disk);
   free(fdc_drive[drive].disk);
   fdc_drive[drive].disk = NULL;
}



void fdc_cmd(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   int cmd;
   fdc_drive_t *drive = &fdc_drive[ctrl_drive];


   ctrl_intrq = 0;   /* new command clears interrupt, FIXME for
                        immediate interrupt case */

   cmd = (data >> 4) & 0x0F;  /* get command bits */
   if ( ((cmd >> 1) >= 1) && ((cmd >> 1) <= 5) )
      cmd &= 0x0E;  /* get rid of data bits in the command field */

   switch (cmd)
   {
      case FDC_RESTORE:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: restore\n");

         drive->track = ctrl_rtrack = 0;

         ctrl_intrq = 1;
         ctrl_status = FDC_TRACK0;
         if (data & (FDC_LOADHEAD | FDC_VERIFY))
            ctrl_status |= FDC_HEADLOADED;
         if (disk_iswrprot(drive->disk))
            ctrl_status |= FDC_WRPROT;
         if (drive->disk == NULL)
            ctrl_status |= FDC_INDEXPULSE;
         break;

      case FDC_SEEK:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: seek\n");

         drive->track += ctrl_rdata - ctrl_rtrack;
         if (drive->track < 0)
            drive->track = 0;
         else if (drive->track > FDC_MAXTRACK)
            drive->track = FDC_MAXTRACK;

         if (ctrl_rdata < ctrl_rtrack)
            ctrl_stepdir = -1;
         else
            ctrl_stepdir = +1;
         ctrl_rtrack = ctrl_rdata;

         ctrl_intrq = 1;
         ctrl_status = 0;
         if (drive->track == 0)
            ctrl_status |= FDC_TRACK0;
         if (data & (FDC_LOADHEAD | FDC_VERIFY))
            ctrl_status |= FDC_HEADLOADED;
         if (disk_iswrprot(drive->disk))
            ctrl_status |= FDC_WRPROT;
         if (drive->disk == NULL)
            ctrl_status |= FDC_INDEXPULSE;
         break;

      case FDC_STEP:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: step\n");

         drive->track += ctrl_stepdir;
         if (drive->track < 0)
            drive->track = 0;
         else if (drive->track > FDC_MAXTRACK)
            drive->track = FDC_MAXTRACK;

         if (data & FDC_UPDATETRACK)
            ctrl_rtrack += ctrl_stepdir;

         ctrl_intrq = 1;
         ctrl_status = 0;
         if (drive->track == 0)
            ctrl_status |= FDC_TRACK0;
         if (data & (FDC_LOADHEAD | FDC_VERIFY))
            ctrl_status |= FDC_HEADLOADED;
         if (disk_iswrprot(drive->disk))
            ctrl_status |= FDC_WRPROT;
         if (drive->disk == NULL)
            ctrl_status |= FDC_INDEXPULSE;
         break;

      case FDC_STEPIN:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: stepin\n");

         drive->track++;
         if (drive->track > FDC_MAXTRACK)
            drive->track = FDC_MAXTRACK;

         if (data & FDC_UPDATETRACK)
            ctrl_rtrack++;

         ctrl_stepdir = +1;

         ctrl_intrq = 1;
         ctrl_status = 0;
         if (drive->track == 0)
            ctrl_status |= FDC_TRACK0;
         if (data & (FDC_LOADHEAD | FDC_VERIFY))
            ctrl_status |= FDC_HEADLOADED;
         if (disk_iswrprot(drive->disk))
            ctrl_status |= FDC_WRPROT;
         if (drive->disk == NULL)
            ctrl_status |= FDC_INDEXPULSE;
         break;

      case FDC_STEPOUT:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: stepout\n");

         drive->track--;
         if (drive->track < 0)
            drive->track = 0;

         if (data & FDC_UPDATETRACK)
            ctrl_rtrack--;

         ctrl_stepdir = -1;

         ctrl_intrq = 1;
         ctrl_status = 0;
         if (drive->track == 0)
            ctrl_status |= FDC_TRACK0;
         if (data & (FDC_LOADHEAD | FDC_VERIFY))
            ctrl_status |= FDC_HEADLOADED;
         if (disk_iswrprot(drive->disk))
            ctrl_status |= FDC_WRPROT;
         if (drive->disk == NULL)
            ctrl_status |= FDC_INDEXPULSE;
         break;



      case FDC_READSECT:
         /* FIXME: non-standard disk layout, re-design multisector reads */

         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: readsect\n");

         if ((drive->disk == NULL) || (ctrl_rsect < 1) || (ctrl_rsect > 10))
         {
            ctrl_status = FDC_RECNOTFOUND;
            break;
         }

         disk_read(drive->disk, buf, 512, ctrl_side, drive->track, ctrl_rsect);
         buf_index = 0;
         bytes_left = 512;

         if (data & FDC_MULTISECT)
            while (++ctrl_rsect <= 10)
            {
               disk_read(drive->disk, &buf[bytes_left], 512, 
                         ctrl_side, drive->track, ctrl_rsect);
               bytes_left += 512;
            }

         ctrl_status = FDC_BUSY | FDC_DRQ;
         ctrl_drq = 1;
         break;

      case FDC_WRITESECT:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: writesect\n");

         break;



      case FDC_READADDR:
         /* FIXME for non-standard disks */

         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: readaddr\n");

         bytes_left = 6;
         buf_index = 0;
         buf[0] = ctrl_rsect = drive->track;
         buf[1] = ctrl_side;
         buf[2] = 1;
         buf[3] = 2;
         buf[4] = buf[5] = 0xFF; /* crc, FIXME! */
         ctrl_status = FDC_BUSY | FDC_DRQ;
         ctrl_drq = 1;
         break;

      case FDC_READTRACK:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: readtrack\n");

         break;

      case FDC_WRITETRACK:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: writetrack\n");

         break;



      case FDC_INTERRUPT:
         if (debuglvl > 0)
            fprintf(debuglog, "fdc_cmd: interrupt\n");

         ctrl_status = FDC_WRPROT; /* FIXME: check this */
         bytes_left = 0;
         ctrl_drq = 0;
         ctrl_intrq = 1;
         break;
   }

}


UINT16 fdc_status(UINT16 port, struct z80PortRead *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_status: called\n");

   ctrl_intrq = 0;   /* read status clears interrupt, FIXME for
                        immediate interrupt case */

   if (bytes_left != 0) /* FIXME: timing */
   {
      buf_index++;
      ctrl_status |= FDC_LOSTDATA;
      if (--bytes_left == 0)
      {
         ctrl_status = FDC_WRPROT | FDC_LOSTDATA;  /* FIXME! */
         ctrl_drq = 0;
         ctrl_intrq = 1;
      }
   }


   return ctrl_status;
}



void fdc_data_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_data_w: data = 0x%02X\n", data);
   ctrl_rdata = data;
}


UINT16 fdc_data_r(UINT16 port, struct z80PortRead *port_s)
{
   if (bytes_left != 0)
   {
      ctrl_rdata = buf[buf_index++];
      if (--bytes_left == 0)
      {
         ctrl_drq = 0;
         ctrl_intrq = 1;
         ctrl_status = FDC_WRPROT; /* FIXME! */
      }
   }

   if (debuglvl > 0)
      fprintf(debuglog, "fdc_data_r: data = 0x%02X\n", ctrl_rdata & 0xFF);

   return ctrl_rdata & 0xFF;
}



void fdc_track_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_track_w: called\n");
   ctrl_rtrack = data;
}

UINT16 fdc_track_r(UINT16 port, struct z80PortRead *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_track_r: called\n");
   return ctrl_rtrack;
}



void fdc_sect_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_sect_w: called\n");
   ctrl_rsect = data;
}

UINT16 fdc_sect_r(UINT16 port, struct z80PortRead *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_sect_r: called\n");
   return ctrl_rsect;
}



void fdc_ext_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_ext_w: called\n");
   ctrl_drive = data & 0x03;
   ctrl_side = (data & 0x04) >> 2;
   ctrl_ddense = (data & 0x08) >> 3;
}


UINT16 fdc_ext_r(UINT16 port, struct z80PortRead *port_s)
{
   if (debuglvl > 0)
      fprintf(debuglog, "fdc_ext_r: called\n");
   if (ctrl_intrq || ctrl_drq)
      return 0x80;
   else
      return 0x00;
}




