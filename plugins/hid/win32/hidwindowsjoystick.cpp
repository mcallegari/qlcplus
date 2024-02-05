/*
  Q Light Controller Plus
  hidwindowsjoystick.cpp

  Copyright (c) Massimo Callegari

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

#include "hidwindowsjoystick.h"

#define JOY_BUTTON_MASK(n) (1 << n)

HIDWindowsJoystick::HIDWindowsJoystick(HIDPlugin *parent, quint32 line, hid_device_info *info)
    : HIDJsDevice(parent, line, info)
{
    init();
}

bool HIDWindowsJoystick::isJoystick(unsigned short vid, unsigned short pid)
{
    JOYCAPS caps;
    JOYINFO joyInfo;

    for (UINT i = 0; i < joyGetNumDevs(); i++)
    {
        memset(&caps, 0, sizeof(JOYCAPS));

        MMRESULT error = joyGetDevCapsW(i, &caps, sizeof(JOYCAPS));
        if (error == JOYERR_NOERROR && vid == caps.wMid && pid == caps.wPid)
        {
            if (joyGetPos(i, & joyInfo) == JOYERR_NOERROR)
                return true;
        }
    }
    return false;
}

void HIDWindowsJoystick::init()
{
    m_info.dwFlags = JOY_RETURNALL;
    m_info.dwSize  = sizeof(m_info);

    QString devPath = path();
    bool ok;
    unsigned short VID = devPath.mid(devPath.indexOf("vid_") + 4, 4).toUShort(&ok, 16);
    unsigned short PID = devPath.mid(devPath.indexOf("pid_") + 4, 4).toUShort(&ok, 16);

    for (UINT i = 0; i < joyGetNumDevs(); i++)
    {
        memset(&m_caps, 0, sizeof(m_caps));

        MMRESULT error = joyGetDevCapsW(i, &m_caps, sizeof(JOYCAPS));

        if (error == JOYERR_NOERROR && VID == m_caps.wMid && PID == m_caps.wPid)
        {
            /* Windows joystick drivers may provide any combination of
             * X,Y,Z,R,U,V,POV - not necessarily the first n of these.
             */
            if (m_caps.wCaps & JOYCAPS_HASV)
            {
                m_axesNumber = 6;
                //joy->min[ 7 ] = -1.0; joy->max[ 7 ] = 1.0;  /* POV Y */
                //joy->min[ 6 ] = -1.0; joy->max[ 6 ] = 1.0;  /* POV X */
            }
            else
                m_axesNumber = m_caps.wNumAxes;

            m_buttonsNumber = m_caps.wNumButtons;
            m_axesValues.fill(0, m_axesNumber);
            m_windId = i;
            break;
        }
        else
        {
            m_axesNumber = 0;
            m_buttonsNumber = 0;
            m_windId = -1;
        }
    }
}

bool HIDWindowsJoystick::readEvent()
{
    MMRESULT status = joyGetPosEx(m_windId, &m_info);

    if (status != JOYERR_NOERROR)
        return false;

    if (m_buttonsNumber)
    {
        for (int i = 0; i < m_buttonsNumber; ++i)
        {
            if ((m_info.dwButtons & JOY_BUTTON_MASK(i)) !=
                (m_buttonsMask & JOY_BUTTON_MASK(i)))
            {
                if (m_info.dwButtons & JOY_BUTTON_MASK(i))
                    emit valueChanged(UINT_MAX, m_line, m_axesNumber + i, 255);
                else
                    emit valueChanged(UINT_MAX, m_line, m_axesNumber + i, 0);
            }
        }
        m_buttonsMask = m_info.dwButtons;
    }

    if (m_axesNumber)
    {
        QList<DWORD> cmpVals;
        cmpVals.append(m_info.dwXpos);
        cmpVals.append(m_info.dwYpos);
        cmpVals.append(m_info.dwZpos);
        cmpVals.append(m_info.dwRpos);
        cmpVals.append(m_info.dwUpos);
        cmpVals.append(m_info.dwVpos);

        for (int i = 0; i < m_axesNumber; i++)
        {
            uchar val = SCALE(double(cmpVals.at(i)), double(0), double(USHRT_MAX),
                        double(0), double(UCHAR_MAX));
            if (val != (uchar)m_axesValues.at(i))
                emit valueChanged(UINT_MAX, m_line, i, val);
            m_axesValues[i] = val;
        }
    }
    return true;
}
