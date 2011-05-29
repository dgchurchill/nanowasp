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

#ifndef MICROBEE_H
#define MICROBEE_H

#include <map>
#include <vector>
#include <wx/filename.h>
#include "tinyxml/tinyxml.h"

class Device;
class Terminal;


/*! \brief Represents the emulated system
 *
 *  This class manages a thread which is responsible for emulating the system.  Based on the
 *  configuration specified, each Device require is instantiated, connections between them are
 *  established, and the list of devices requiring execution is created (most devices simply respond
 *  to port read/write requests, notable exceptions are the CRTC which must render the screen
 *  periodically, and the Z80 which must execute instructions).  The main loop of the thread simply
 *  calls the devices in the run list.
 */
class Microbee : public wxThread
{
public:
    /*! \brief Time type
     *
     *  All times are expressed in microseconds.  Using long long should allow the 
     *  emulation to run for around 300,000 years before it overflows.
     */
    typedef long long time_t;

    /*! \brief Construct the system based on XML file \p config_file, using \p scr_ for VDU 
     *         and keyboard support functions.
     *
     *  \throws ConfigError if a problem was found with the configuration
     */
    Microbee(Terminal &scr_, const char *config_file);
    ~Microbee();

    //! Main thread function, repeatedly executes the devices on the run list
    virtual ExitCode Entry();


    //! Resets the emulated system (only returns once the reset has completed).  Thread safe.
    void DoReset();

    //! Pauses the emulation (only returns once the emulation is paused).  Thread safe.
    void PauseEmulation();

    //! Resumes the emulation (returns immediately).  Thread safe.
    void ResumeEmulation();

    //! Seralizes the current emulation state into the specfied file.  Thread safe.
    void SaveState(const char *filename);
    
    //! Loads a disk in the specified drive.  TODO: Remove this and generalise Device specific functions
    void LoadDisk(unsigned int drive, const char *name);


    /*! \brief Returns a pointer to the Device identified by \p id.
     *
     *  \throws UnknownDevice if the device \p id doesn't exist
     *  \throws BadDeviceClass if the device is of the wrong type
     */
    template <typename T> T *GetDevice(std::string id)
    {
        Device *d = devices[id];

        if (d == NULL)
            throw UnknownDevice(id);

        T *t = dynamic_cast<T*>(d);
        if (t == NULL)
            throw BadDeviceClass(id);

        return t;
    }

    //! Returns a reference to the Terminal associated with this Microbee
    Terminal &GetTerminal() { return scr; }


    //! Returns the current emulation time in microseconds
    Microbee::time_t GetTime() const;
    
    //! Returns the absolute filename of the current configuration file
    const wxFileName& GetConfigFileName() const { return configFileName; }


private:
    //! Resets the emulated system
    void Reset();

    volatile bool paused;  //!< True if the emulation is currently paused.  Microbee thread does not write to this.  Only written by PauseEmulation() and ResumeEmulation().
    wxMutex pause_mutex;  //!< Mutex associated with pause_cond, must be constructed first
    wxCondition pause_cond;  //!< Used by the Microbee thread to signal to the main thread that emulation has paused


    Terminal &scr;  //!< For VDU and keyboard support functions

    /*! \brief Container for emulated devices
     *
     *  \note The devices stored in this map are owned by the map, and freed by ~Microbee()
     */
    std::map<std::string, Device*> devices;

    std::vector<Device*> run_list;  //!< List of devices to that require Device::Execute() to be called
    Device *current_dev;  //!< Currently executing device

    static const Microbee::time_t cMaxMicrosToRun = 200000;  //!< Maximum number of microseconds to run in an iteration of the main loop

    Microbee::time_t emu_time;  //!< Current emulation time (= 0 at reset)

    wxFileName configFileName;  //!< The absolute filename of the configuration file
    
    TiXmlDocument configuration;  //!< The XML configuration of the system

    // Private copy constuctor and assigment operator to prevent copies
    Microbee(const Microbee &);
    Microbee& operator= (const Microbee &);
};


#endif // MICROBEE_H