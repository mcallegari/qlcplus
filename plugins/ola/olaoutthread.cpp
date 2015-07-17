/*
  Q Light Controller
  olaoutthread.cpp

  Copyright (c) Simon Newton

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

#include <QDebug>
#include <ola/Callback.h>
#include "olaoutthread.h"

OlaOutThread::OlaOutThread()
    : QThread()
    , m_init_run(false)
    , m_ss(NULL)
    , m_pipe(NULL)
    , m_client(NULL)
{
}

/*
 * Clean up.
 */
OlaOutThread::~OlaOutThread()
{
    wait();
    if (m_client)
    {
        m_client->Stop();
        delete m_client;
    }

    if (m_pipe)
        delete m_pipe;

    cleanup();
}


/*
 * Start the OLA thread
 *
 * @return true if sucessfull, false otherwise
 */
bool OlaOutThread::start(Priority priority)
{
    if (!init())
        return false;

    if (!m_pipe)
    {
        // setup the pipe to recv dmx data on
        m_pipe = new ola::io::LoopbackDescriptor();
        m_pipe->Init();

        m_pipe->SetOnData(ola::NewCallback(this, &OlaOutThread::new_pipe_data));
        m_pipe->SetOnClose(ola::NewSingleCallback(this, &OlaOutThread::pipe_closed));
        m_ss->AddReadDescriptor(m_pipe);
    }

    QThread::start(priority);
    return true;
}


/*
 * Close the socket which stops the thread.
 */
void OlaOutThread::stop()
{
    if (m_pipe)
        m_pipe->CloseClient();
    return;
}


/*
 * Run the select server.
 */
void OlaOutThread::run()
{
    m_ss->Run();
    return;
}


/*
 * Send the new data over the socket so that the other thread picks it up.
 * @param universe the universe nmuber this data is for
 * @param data a pointer to the data
 * @param channels the number of channels
 */
int OlaOutThread::write_dmx(unsigned int universe, const QByteArray& data)
{
    if (m_pipe)
    {
        Q_ASSERT(data.size() <= (int)sizeof(m_data.data));

        m_data.universe = universe;
        memset(m_data.data, 0, sizeof(m_data.data));
        memcpy(m_data.data, data.data(), data.size());

        m_pipe->Send((uint8_t*) &m_data, sizeof(m_data));
    }
    return 0;
}


/*
 * Called when the pipe used to communicate between QLC and OLA is closed
 */
void OlaOutThread::pipe_closed()
{
    // We don't need to delete the socket here because that gets done in the
    // Destructor.
    m_ss->Terminate();
}


/*
 * Called when there is data to be read on the pipe socket.
 */
void OlaOutThread::new_pipe_data()
{
    dmx_data data;
    unsigned int data_read;
    int ret = m_pipe->Receive((uint8_t*) &data, sizeof(data), data_read);
    if (ret < 0)
    {
        qCritical() << "olaout: socket receive failed";
        return;
    }

    m_buffer.Set(data.data, data_read - sizeof(data.universe));
    if (!m_client->SendDmx(data.universe, m_buffer))
        qWarning() << "olaout:: SendDmx() failed";
}


/*
 * Setup the OlaCallbackClient to communicate with the server.
 * @return true if the setup worked corectly.
 */
bool OlaOutThread::setup_client(ola::io::ConnectedDescriptor *descriptor)
{
    if (!m_client)
    {
        m_client = new ola::OlaCallbackClient(descriptor);
        if (!m_client->Setup())
        {
            qWarning() << "olaout: client setup failed";
            delete m_client;
            m_client = NULL;
            return false;
        }
        m_ss->AddReadDescriptor(descriptor);
    }
    return true;
}


/*
 * Cleanup after the main destructor has run
 */
void OlaStandaloneClient::cleanup()
{
    if (m_tcp_socket)
    {
        if (m_ss)
            m_ss->RemoveReadDescriptor(m_tcp_socket);
        delete m_tcp_socket;
        m_tcp_socket = NULL;
    }

    if (m_ss)
        delete m_ss;
}


/*
 * Setup the standalone client.
 * @return true is successful.
 */
bool OlaStandaloneClient::init()
{
    if (m_init_run)
        return true;

    if (!m_ss)
        m_ss = new ola::io::SelectServer();

    if (!m_tcp_socket)
    {
        ola::network::IPV4SocketAddress server_address(
            ola::network::IPV4Address::Loopback(), ola::OLA_DEFAULT_PORT);
        m_tcp_socket = ola::network::TCPSocket::Connect(server_address);
        if (!m_tcp_socket)
        {
            qWarning() << "olaout: Connect failed, is OLAD running?";
            delete m_tcp_socket;
            m_tcp_socket = NULL;
            delete m_ss;
            m_ss = NULL;
            return false;
        }
    }

    if (!setup_client(m_tcp_socket))
    {
        m_tcp_socket->Close();
        delete m_tcp_socket;
        m_tcp_socket = NULL;
        delete m_ss;
        m_ss = NULL;
        return false;
    }
    m_init_run = true;
    return true;
}


/*
 * Clean up the embedded server.
 */
void OlaEmbeddedServer::cleanup()
{
    if (m_daemon)
        delete m_daemon;

    if (m_pipe_socket)
        delete m_pipe_socket;
}


/*
 * Setup the embedded server.
 * @return true is successful.
 */
bool OlaEmbeddedServer::init()
{
    if (m_init_run)
        return true;

    ola::OlaServer::Options options;
    options.http_enable = true;
    options.http_port = ola::OlaServer::DEFAULT_HTTP_PORT;
    m_daemon = new ola::OlaDaemon(options);
    if (!m_daemon->Init())
    {
        qWarning() << "OLA Server failed init";
        delete m_daemon;
        m_daemon = NULL;
        return false;
    }
    m_ss = m_daemon->GetSelectServer();

    // setup the pipe socket used to communicate with the OlaServer
    if (!m_pipe_socket)
    {
        m_pipe_socket = new ola::io::PipeDescriptor();
        if (!m_pipe_socket->Init())
        {
            qWarning() << "olaout: pipe failed";
            delete m_pipe_socket;
            m_pipe_socket = NULL;
            delete m_daemon;
            m_daemon = NULL;
            return false;
        }
    }

    if (!setup_client(m_pipe_socket))
    {
        delete m_pipe_socket;
        m_pipe_socket = NULL;
        delete m_daemon;
        m_daemon = NULL;
        return false;
    }

    m_daemon->GetOlaServer()->NewConnection(m_pipe_socket->OppositeEnd());
    m_init_run = true;
    return true;
}
