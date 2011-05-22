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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <sstream>

#include "tinyxml/tinyxml.h"


/*! \brief Thrown if a file is unable to be opened
 */
class FileNotFound : public std::runtime_error
{
public:
    explicit FileNotFound(const std::string &name) : std::runtime_error(name) {}
};


/*! \brief Thrown when a reference to an unknown Device id is made
 */
class UnknownDevice : public std::runtime_error
{
public:
    explicit UnknownDevice(const std::string &id) : std::runtime_error(id) {}
};


/*! \brief Thrown when a reference to an incorrect/unknown Device class is made 
 */
class BadDeviceClass : public std::runtime_error
{
public:
    explicit BadDeviceClass(const std::string &device) : std::runtime_error(device) {}
};




/*! \brief Thrown when a Disk fails to construct
 */
class DiskImageError : public std::runtime_error
{
public:
    DiskImageError() : std::runtime_error("DiskImageError") {}
    explicit DiskImageError(const std::string &detail) : std::runtime_error(detail) {}
};


/*! \brief Thrown when a parameter is out of range
 */
class OutOfRange : public std::runtime_error
{
public:
    OutOfRange() : std::runtime_error("OutOfRange") {}
};


/*! \brief Thrown when a configuration error is detected
 *
 *  This exception should only be thrown if when an error in user modifiable
 *  configuration data is found. 
 */
class ConfigError : public std::runtime_error
{
public:
    ConfigError(const TiXmlNode *node, const std::string &detail) : 
        std::runtime_error(""), msg(detail)
    {
        if (node)
        {
            std::ostringstream ss;
            ss << "\nRow: " << node->Row() << " Col: " << node->Column();
            msg = detail + ss.str();
        }
    }

    virtual const char *what() const throw () { return msg.c_str(); }

    virtual ~ConfigError() throw() {}
    
private:
    std::string msg;
};

#endif // EXCEPTIONS_H