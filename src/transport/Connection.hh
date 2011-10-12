/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef CONNECTION_HH
#define CONNECTION_HH

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <iomanip>
#include <deque>

#include <google/protobuf/message.h>

#include "common/Event.hh"
#include "common/Console.hh"
#include "common/Exception.hh"

#define HEADER_LENGTH 8

namespace gazebo
{
  namespace transport
  {
    class IOManager;
    class Connection;
    typedef boost::shared_ptr<Connection> ConnectionPtr;

    /// \addtogroup gazebo_transport
    /// \{

    /// \brief TCP/IP Connection
    class Connection : public boost::enable_shared_from_this<Connection>
    {
      /// \brief Constructor
      public: Connection();

      /// \brief Destructor
      public: virtual ~Connection();

      /// \brief Connect to a remote host
      public: void Connect(const std::string &host,  unsigned short port);

      typedef boost::function<void(const ConnectionPtr&)> AcceptCallback;

      /// \brief Start a server that listens on a port
      public: void Listen(unsigned short port, const AcceptCallback &accept_cb);

      typedef boost::function<void(const std::string &data)> ReadCallback;
      /// \brief Start a thread that reads from the connection, and passes
      ///        new message to the ReadCallback
      public: void StartRead(const ReadCallback &cb);
             
      /// \brief Stop the read loop 
      public: void StopRead();

      /// \brief Shutdown the socket
      public: void Shutdown();

      /// \brief Return true if the connection is open
      public: bool IsOpen() const;

      /// \brief Close a connection
      private: void Close();

      /// \brief Cancel all async operations on an open socket
      private: void Cancel();

      /// \brief Read data from the socket
      public: bool Read(std::string &data);

      /// \brief Write data to the socket
      public: void EnqueueMsg(const std::string &_buffer, bool _force = false);

      /// \brief Get the local URI
      public: std::string GetLocalURI() const;
              
      /// \brief Get the remote URI
      public: std::string GetRemoteURI() const;

      /// \brief Get the address of this connection
      public: std::string GetLocalAddress() const;

      /// \brief Get the port of this connection
      public: unsigned short GetLocalPort() const;

      /// \brief Get the remote address
      public: std::string GetRemoteAddress() const;

      /// \brief Get the remote port number
      public: unsigned short GetRemotePort() const;

      /// \brief Get the remote hostname
      public: std::string GetRemoteHostname() const;

      /// \brief Get the local hostname
      public: std::string GetLocalHostname() const;

      /// \brief Peform and asyncronous read
      public: template<typename Handler>
              void AsyncRead(Handler handler)
              {
                void (Connection::*f)(const boost::system::error_code &,
                    boost::tuple<Handler>) = &Connection::OnReadHeader<Handler>;

                this->inbound_header.resize(HEADER_LENGTH);
                boost::asio::async_read(*this->socket,
                    boost::asio::buffer(this->inbound_header),
                    boost::bind(f, shared_from_this(), 
                                boost::asio::placeholders::error,
                                boost::make_tuple(handler)) );
              }

      // Handle a completed read of a message header. The handler is passed
      // using a tuple since boost::bind seems to have trouble binding
      // a function object created using boost::bind as a parameter
      private: template<typename Handler>
               void OnReadHeader(const boost::system::error_code &e_,
                                 boost::tuple<Handler> handler_)
              {
                if (e_)
                {
                  if (e_.message() != "End of File")
                  {
                    this->Close();
                    // This will occur when the other side closes the
                    // connection
                  }
                }
                else
                {
                  std::size_t inbound_data_size = 0;
                  std::string header(&this->inbound_header[0], 
                    this->inbound_header.size());
                  this->inbound_header.clear();

                  inbound_data_size = this->ParseHeader(header);


                  /* DEBUG
                  if (this->GetLocalPort() == 11345)
                    printf("Master: receive header from[%d] [%d]\n", this->GetRemotePort(), (int)inbound_data_size);

                  if (this->GetRemotePort() == 11345)
                    printf("Slave[%d]: receive header [%d]\n", this->GetLocalPort(), (int)inbound_data_size);
                    */

                 if (inbound_data_size > 0)
                  {
                    // Start the asynchronous call to receive data
                    this->inbound_data.resize(inbound_data_size);

                    void (Connection::*f)(const boost::system::error_code &e,
                        boost::tuple<Handler>) = &Connection::OnReadData<Handler>;

                    //gzdbg << "Connection::async_read[" << inbound_data_size << "][" << this->GetRemotePort() << "]\n";
                    boost::asio::async_read( *this->socket, 
                        boost::asio::buffer(this->inbound_data), 
                        boost::bind(f, shared_from_this(), 
                                    boost::asio::placeholders::error, 
                                    handler_) );
                  }
                  else
                  {
                    gzerr << "Header is empty\n";

                   /* void (Connection::*f)(const boost::system::error_code &,
                        boost::tuple<Handler>) = &Connection::OnReadHeader<Handler>;


                    boost::asio::async_read(*this->socket,
                        boost::asio::buffer(this->inbound_header),
                        boost::bind(f, shared_from_this(), 
                          boost::asio::placeholders::error,
                          handler_) );
                          */
                   boost::get<0>(handler_)("");
                  }
                }
              }

     private: template<typename Handler>
              void OnReadData(const boost::system::error_code &e,
                              boost::tuple<Handler> handler)
              {

                if (e)
                  gzerr << "Error Reading data!\n";

                //gzdbg << "  Connection::async_READ[" 
                  //<< this->inbound_data.size() << "][" 
                  //<< this->GetRemotePort() << "]\n";

                // Inform caller that data has been received
                std::string data(&this->inbound_data[0], 
                    this->inbound_data.size());
                this->inbound_data.clear();


                /* DEBUG
                if (this->GetLocalPort() == 11345)
                  printf("Master: receive data from[%d] [%d]\n", this->GetRemotePort(), (int)data.size());

                  if (this->GetRemotePort() == 11345)
                    printf("Slave[%d]: receive data [%d]\n", this->GetLocalPort(), (int)data.size());
                    */

                if (data.empty())
                  gzerr << "OnReadData got empty data!!!\n";
                
                if (!e)
                  boost::get<0>(handler)(data);
              }

     public: event::ConnectionPtr ConnectToShutdown( boost::function<void()> subscriber_ ) 
             { return this->shutdown.Connect(subscriber_); }

     public: void DisconnectShutdown( event::ConnectionPtr subscriber_)
             {this->shutdown.Disconnect(subscriber_);}


     /// \brief Handle on write callbacks
     public: void ProcessWriteQueue();

     private: void OnWrite(const boost::system::error_code &e,
                  boost::asio::streambuf *_b);
           //std::list<boost::asio::const_buffer> *_buffer);

     /// \brief Handle new connections, if this is a server
     private: void OnAccept(const boost::system::error_code &e);

     /// \brief Parse a header to get the size of a packet
     private: std::size_t ParseHeader( const std::string &header );

     /// \brief the read thread
     private: void ReadLoop(const ReadCallback &cb);

     /// \brief Get the local endpoint
     private: boost::asio::ip::tcp::endpoint GetLocalEndpoint() const;

     /// \brief Get the remote endpoint
     private: boost::asio::ip::tcp::endpoint GetRemoteEndpoint() const;

     private: std::string GetHostname(boost::asio::ip::tcp::endpoint ep) const;

      private: boost::asio::ip::tcp::socket *socket;
      private: boost::asio::ip::tcp::acceptor *acceptor;

      private: std::deque<std::string> writeQueue;
      private: std::deque<unsigned int> writeCounts;
      private: boost::recursive_mutex *writeMutex;
      private: boost::recursive_mutex *readMutex;

      // Called when a new connection is received
      private: AcceptCallback acceptCB;

      //private: char inbound_header[HEADER_LENGTH];
      private: std::vector<char> inbound_header;
      private: std::vector<char> inbound_data;

      private: boost::thread *readThread;
      private: bool readQuit;

      public: unsigned int id;
      private: static unsigned int idCounter;
      private: ConnectionPtr acceptConn;

      private: event::EventT<void()> shutdown;
      private: static IOManager *iomanager;

      public: unsigned int writeCount;

      private: std::string localURI;
      private: std::string localAddress;
      private: std::string remoteURI;
      private: std::string remoteAddress;
    };
    /// \}
  }
}

#endif
