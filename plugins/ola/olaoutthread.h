/*
  Q Light Controller
  olaoutthread.h

  Copyright (c) Simon Newton
                Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef OLAOUTTHREAD_H
#define OLAOUTTHREAD_H

#include <QThread>

#include <ola/DmxBuffer.h>
#include <ola/OlaClient.h>
#include <ola/io/Descriptor.h>
#include <ola/io/SelectServer.h>
#include <ola/network/Socket.h>
#include <olad/OlaDaemon.h>

// This should really be in qlcmacros.h!
enum { K_UNIVERSE_SIZE = 512 };

// Used to pass data between the threads
typedef struct
{
    unsigned int universe;
    uchar data[K_UNIVERSE_SIZE];
} dmx_data;

/*
 * The OLA thread.
 *
 * Basic design: qlc plugins aren't allowed to block in calls, so we start a
 * new thread which runs a select server. Calls to write_dmx in the plugin send
 * data over a pipe which the OLA thread listens on. It then uses the OlaClient
 * api to send the data to the OLA Server.
 *
 * The thread can either run as a OLA Client or embed the OLA server. As a
 * client, we connect to the OLA server using a TCP socket.
 *
 *   OlaOut --pipe-> OlaOutThread --tcp socket-> olad (separate process)
 *
 * When embedded the server, we still use the OlaClient class and setup a pipe
 * to send the rpcs over. Yes, this results in copying the data twice over a
 * pipe but we can't use a single pipe because the OlaClient needs to
 * response to events.
 *
 * OlaOut --pipe-> OlaOutThread --pipe-> OlaServer
 */
class OlaOutThread : public QThread
{
public:
    OlaOutThread();
    virtual ~OlaOutThread();

    void run();
    bool start(Priority priority=InheritPriority);
    void stop();
    int write_dmx(unsigned int universe, const QByteArray& data);
    void new_pipe_data();
    void pipe_closed();

protected:
    bool setup_client(ola::io::ConnectedDescriptor *descriptor);
    bool m_init_run;
    ola::io::SelectServer *m_ss; // the select server

private:
    virtual bool init() = 0;
    virtual void cleanup() {};
    ola::io::LoopbackDescriptor *m_pipe; // the pipe to get new dmx data on
    ola::OlaClient *m_client;
    dmx_data m_data;
    ola::DmxBuffer m_buffer;
};


/*
 * Use this to run as a standalone client.
 */
class OlaStandaloneClient : public OlaOutThread
{
public:
    OlaStandaloneClient():
            OlaOutThread(),
            m_tcp_socket(NULL) {}

private:
    bool init();
    void cleanup();
    ola::network::TCPSocket *m_tcp_socket;
};


/*
 * Use this to run with an embedded server.
 */
class OlaEmbeddedServer : public OlaOutThread
{
public:
    OlaEmbeddedServer():
            OlaOutThread(),
            m_daemon(NULL),
            m_pipe_socket(NULL) {}

private:
    bool init();
    void cleanup();
    ola::OlaDaemon *m_daemon;
    ola::io::PipeDescriptor *m_pipe_socket;
};

#endif
