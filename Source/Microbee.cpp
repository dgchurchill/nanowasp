/*  Nanowasp - A Microbee emulator
 *  Copyright (C) 2000-2007 David G. Churchill
 *
 *  This file is part of Nanowasp.
 *
 *  Nanowasp is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nanowasp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "stdafx.h"
#include "Microbee.h"

#include <sstream>
#include <limits>

#ifdef __WXOSX__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "Terminal.h"
#include "Device.h"
#include "DeviceFactory.h"
#include "Z80/Z80CPU.h"

#include "Drives.h" // TODO: Remove (remove LoadDisk() func from this class)


Microbee::Microbee(Terminal &scr_, const char *config_file) :
    paused(false),
    pause_cond(pause_mutex),
    scr(scr_),
    current_dev(NULL),
    emu_time(0),
    configFileName(config_file)
{
    pause_mutex.Lock();
    
    configFileName.MakeAbsolute();

    TiXmlDocument config(config_file);
    if (!config.LoadFile())
        throw ConfigError(NULL, std::string("Unable to load configuration file ") + config_file);

    TiXmlElement *mbee_tag = config.FirstChildElement("microbee");
    if (mbee_tag == NULL)
        throw ConfigError(&config, "Config is missing <microbee> element");


    DeviceFactory dev_factory;
    Z80CPU *z80 = NULL;

    // Create devices
    for (const TiXmlElement *el = mbee_tag->FirstChildElement("device"); el != NULL; el = el->NextSiblingElement("device"))
    {
        const char *id = el->Attribute("id");
        const char *cls = el->Attribute("class");
        if (id == NULL)
            throw ConfigError(el, "<device> missing id attribute");
        if (cls == NULL)
            throw ConfigError(el, "<device> missing class attribute");

        Device *dev;
        try
        {
            dev = dev_factory.createDevice(cls, *this, *el);
        }
        catch (BadDeviceClass &c)
        {
            throw ConfigError(el, std::string("<device> specifies unknown class") + c.what());
        }
        devices[id] = dev;

        if (std::string(cls) == "Z80CPU")
            z80 = GetDevice<Z80CPU>(id);

        if (dev->Executable())
            run_list.push_back(dev);
    }

    if (z80 == NULL)
        throw ConfigError(mbee_tag, "No Z80CPU device specified in configuration");


    // Register ports (second loop because we need to find the Z80CPU first)
    for (const TiXmlElement *el = mbee_tag->FirstChildElement("device"); el != NULL; el = el->NextSiblingElement("device"))
    {
        const char *port = el->Attribute("port");
        const char *id = el->Attribute("id");  // This must be valid here, because an exception would've be thrown above if not

        if (port != NULL)
        {
            std::istringstream port_ss(port);

            port_ss >> std::hex >> std::skipws;

            int p;
            while (port_ss >> p)
            {
                try
                {
                    z80->RegPortDevice(p, GetDevice<PortDevice>(id));
                    port_ss.ignore(std::numeric_limits<std::streamsize>::max(), ',');
                }
                catch (OutOfRange &)
                {
                    throw ConfigError(el, "Port out of range");
                }
            }
        }
    }


    // Final initialisation
    std::map<std::string, Device*>::iterator it = devices.begin();
    for (; it != devices.end(); it++)
    {
        try
        {
            it->second->LateInit();
        }
        catch (UnknownDevice &d)
        {
            throw ConfigError(mbee_tag, std::string("<connect> specifies unknown device ") + d.what());  // TODO: Improve detail of error message
        }
        catch (BadDeviceClass &d)
        {
            throw ConfigError(mbee_tag, std::string("<connect> specifies device of incorrect class ") + d.what());  // TODO: Improve detail of error message
        }
    }


    Reset();
}


Microbee::~Microbee()
{
    std::map<std::string, Device*>::iterator it = devices.begin();
    for (; it != devices.end(); it++)
        delete it->second;
}


Microbee::ExitCode Microbee::Entry()
{
    std::vector<Device*>::iterator it;
    wxStopWatch sw;
    time_t elapsed;
    time_t micros_to_run = cMaxMicrosToRun;
    time_t next_micros, tmp;

    while (!TestDestroy())
    {
        if (paused)
        {
            {
                wxMutexLocker lock(pause_mutex);
                pause_cond.Signal();
            }

            // Copy the front buffer into the back buffer so we can just flip continuously to repaint
            glReadBuffer(GL_FRONT);
            glRasterPos2i(0, 0);
            glCopyPixels(0, 0, Terminal::width, Terminal::height, GL_COLOR);
            glFlush();

            while (paused)
            {
                Sleep(100);
                scr.SwapBuffers();
                if (TestDestroy())
                    return 0;
            }
        }


        sw.Start();

        next_micros = cMaxMicrosToRun;

        for (it = run_list.begin(); it != run_list.end(); it++)
        {
            current_dev = *it;
            tmp = current_dev->Execute(emu_time, micros_to_run);
            if (tmp != 0 && tmp < next_micros)
                next_micros = tmp;
        }

        emu_time += micros_to_run;

        elapsed = sw.Time() * 1000;
        if (elapsed < micros_to_run)
            Sleep((micros_to_run - elapsed) / 1000);  // TODO: Using a running average to smooth out the speed?

        micros_to_run = next_micros;
    }

    return 0;
}



/*! \note While paused, the thread will still refresh the the display using OpenGL calls */
// MUST BE THREAD SAFE
void Microbee::DoReset()
{
    PauseEmulation();
    Reset();
    ResumeEmulation();
}


// MUST BE THREAD SAFE
void Microbee::PauseEmulation()
{
    if (!paused)
    {
        paused = true;
        pause_cond.Wait();
    }
}


// MUST BE THREAD SAFE
void Microbee::ResumeEmulation()
{
    paused = false;
}


// MUST BE THREAD SAFE
// TODO: Generalise as a Device member function which registers its own menu / panel
void Microbee::LoadDisk(unsigned int drive, const char *name)
{
    PauseEmulation();
    GetDevice<Drives>("drives")->LoadDisk(drive, name);
    ResumeEmulation();
}


void Microbee::Reset()
{
    std::map<std::string, Device*>::iterator it;

    emu_time = 0;
    current_dev = NULL;

    for (it = devices.begin(); it != devices.end(); it++)
        it->second->Reset();
}


/*! This function is provided for devices to retrieve the current emulation time.  Particularly
    useful when one device calls another, and the second needs to know the current time as viewed
    by the first device.  Because we're running in a single thread, we have to simulate the simulataneous
    execution of devices by running them each for a short time in rapid succession.  As a result, multiple
    sequential calls to this function (in real-world time) may result in time appearing to go backwards (e.g. if the
    main loop of the emulator is executing a different device than during the previous call, but for the
    same emulated period of time).
 */
Microbee::time_t Microbee::GetTime() const
{
    if (current_dev)
        return current_dev->GetTime();
    else
        return emu_time;
}
