/*
  Q Light Controller
  vinceusbdmx512.h

  Copyright (C) Jérôme Lebleu

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef VINCEUSBDMX512_H
#define VINCEUSBDMX512_H

#include <QByteArray>
#include <QObject>

#include "dmxusbwidget.h"

#define VINCE_CMD_START_DMX   char(0x01) //! CMD_START_DMX
#define VINCE_CMD_STOP_DMX    char(0x02) //! CMD_STOP_DMX
#define VINCE_CMD_RESET_DMX   char(0x10) //! CMD_RAZ_DMX
#define VINCE_CMD_UPDATE_DMX  char(0x11) //! CMD_MAJ_DMX
#define VINCE_START_OF_MSG    char(0x0f) //! STX
#define VINCE_END_OF_MSG      char(0x04) //! ETX

#define VINCE_RESP_OK         char(0x00) //! CMD_OK
#define VINCE_RESP_KO         char(0x01) //! CMD_KO
#define VINCE_RESP_WARNING    char(0x02) //! CMD_WARNING
#define VINCE_RESP_UNKNOWN    char(0x02) //! CMD_UNKNOWN
#define VINCE_RESP_IO_ERR     char(0x10) //! CMD_IO_ERR
#define VINCE_RESP_PARAM_ERR  char(0x11) //! CMD_PARAM_ERR

/**
 * This is the base interface class for Vince USB-DMX512 widgets.
 */
class VinceUSBDMX512 : public DMXUSBWidget
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VinceUSBDMX512(DMXInterface *interface, quint32 outputLine);
    virtual ~VinceUSBDMX512();

    /** @reimp */
    Type type() const;

protected:
    /** Requests commands */
    enum Command
    {
        StartDMX  = VINCE_CMD_START_DMX,
        StopDMX   = VINCE_CMD_STOP_DMX,
        ResetDMX  = VINCE_CMD_RESET_DMX,
        UpdateDMX = VINCE_CMD_UPDATE_DMX
    };

    /****************************************************************************
     * Name & Serial
     ****************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /****************************************************************************
     * Open & Close
     ****************************************************************************/
public:
    /** @reimp */
    bool open(quint32 line = 0, bool input = false);

    /** @reimp */
    virtual bool close(quint32 line = 0, bool input = false);

    /************************************************************************
     * Write & Read
     ************************************************************************/
protected:
    /**
     * Format and write data to the opened interface.
     *
     * @param command The request type
     * @param data The data to write
     * @return true if the values were sent successfully, otherwise false
     */
    bool writeData(enum Command command, const QByteArray& data = QByteArray());

    /**
     * Read and extract data from the opened interface.
     *
     * @param ok A pointer which tells if data was read or not
     * @return The available data
     */
    QByteArray readData(bool* ok = NULL);

    /************************************************************************
     * Write universe
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

private:
    QByteArray m_universe;
};

#endif
