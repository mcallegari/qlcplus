/*
  Q Light Controller
  olaoutthread.h

  Copyright (c) Simon Newton
                Heikki Junnila

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

#ifndef OLAOUTTHREAD_H
#define OLAOUTTHREAD_H

#include <QThread>

#include <ola/DmxBuffer.h>
#include <ola/OlaCallbackClient.h>
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
 * data over a pipe which the OLA thread listens on. It then uses the
 * OlaCallbackClient api to send the data to the OLA Server.
 *
 * The thread can either run as a OLA Client or embed the OLA server. As a
 * client, we connect to the OLA server using a TCP socket.
 *
 *   OlaOut --pipe-> OlaOutThread --tcp socket-> olad (separate process)
 *
 * When embedded the server, we still use the OlaCallbackClient class and setup
 * a pipe to send the rpcs over. Yes, this results in copying the data twice
 * over a pipe but we can't use a single pipe because the OlaClient needs to
 * respond to events.
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
    ola::OlaCallbackClient *m_client;
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
