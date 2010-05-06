/* 
 * File:   tcp_driver.h
 * Author: thek
 *
 * Created on den 24 februari 2010, 21:18
 */

#ifndef _TCP_DRIVER_H
#define	_TCP_DRIVER_H
#include "binlog_driver.h"
#include "bounded_buffer.h"
#include "protocol.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>


#define MAX_PACKAGE_SIZE 0xffff

#define GET_NEXT_PACKET_HEADER   \
   boost::asio::async_read(*m_socket, boost::asio::buffer(m_net_header, 4), \
     boost::bind(&Binlog_tcp_driver::handle_net_packet_header, this, \
     boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)) \

using boost::asio::ip::tcp;

namespace MySQL { namespace system {

class Binlog_tcp_driver : public Binary_log_driver
{
public:

    Binlog_tcp_driver() {
        m_socket = NULL;
        m_binlog_offset = 4;
        m_waiting_event= 0;
        m_event_queue= new bounded_buffer<Binary_log_event_ptr >(50);
        m_event_loop= 0;
        m_total_bytes_transferred= 0;
        m_shutdown= false;
    }

    ~Binlog_tcp_driver()
    {
        delete m_event_queue;
        delete m_socket;
    }

    /**
     * Connects to a mysql server, authenticates and initiates the event
     * request loop.
     *
     * @param user The user account on the server side
     * @param passwd The password used to authenticate the user
     * @param host The DNS host name or IP of the server
     * @param port The service port number to connect to
     *
     *
     * @return Success or failure code
     *   @retval 0 Successfully established a connection
     *   @retval >1 An error occurred.
     */
    int connect(const std::string user, const std::string passwd, const std::string host, long port, std::string binlog_filename="", size_t offset=4);


    /**
     * Blocking wait for the next binary log event to reach the client
     */
    int wait_for_next_event(MySQL::Binary_log_event_ptr &event);

    /**
     * Non-blocking wait for the next binary log event to reach the client
     */
    int poll_next_event(MySQL::Binary_log_event_ptr &event);

    /**
     * Reconnects to the master with a new binlog dump request.
     */
    bool set_position(const std::string &str, unsigned long position);

    bool get_position(std::string &str, unsigned long &position);


private:

    /**
     * Request a binlog dump and starts the event loop in a new thread
     * @param binlog_file_name The base name of the binlog files to query
     *
     */
    void start_binlog_dump(std::string &binlog_file_name, size_t offset);

    /**
     * Handles a completed mysql server package header and put a
     * request for the body in the job queue.
     */
    void handle_net_packet_header(const boost::system::error_code& err, std::size_t bytes_transferred);

    /**
     * Handles a completed network package with the assumption that it contains
     * a binlog event.
     *
     * TODO rename to handle_event_log_packet?
     */
    void handle_net_packet(const boost::system::error_code& err, std::size_t bytes_transferred);

    /**
     * Called from handle_net_packet(). The function handle a stream of bytes
     * representing event packets which may or may not be complete.
     * It uses m_waiting_event and the size of the stream as parameters
     * in a state machine. If there is no m_waiting_event then the event
     * header must be parsed for the event packet length. This can only
     * be done if the accumulated stream of bytes are more than 19.
     * Next, if there is a m_waiting_event, it can only be completed if
     * event_length bytes are waiting on the stream.
     *
     * If none of these conditions are fullfilled, the function exits without
     * any action.
     *
     * @param err Not used
     * @param bytes_transferred The number of bytes waiting in the event stream
     * 
     */
    void handle_event_packet(const boost::system::error_code& err, std::size_t bytes_transferred);
    
    /**
     * Executes io_service in a loop.
     * TODO Checks for connection errors and reconnects to the server
     * if necessary.
     */
    void start_event_loop(void);

    /**
     * Reconnect to the server by first calling disconnect and then connect.
     */
    void reconnect(void);

    /**
     * Disconnet from the server. The io service must have been stopped before
     * this function is called.
     * The event queue is emptied.
     */
    void disconnect(void);

    /**
     * Terminates the io service and sets the shudown flag.
     * this causes the event loop to terminate.
     */
    void shutdown(void);

    void parse_event(boost::asio::streambuf &sbuff, Binary_log_event_ptr &ev);

    boost::thread *m_event_loop;
    boost::asio::io_service m_io_service;
    tcp::socket *m_socket;
    bool m_shutdown;

    /**
     * Temporary storage for a handshake package
     */
    struct st_handshake_package m_handshake_package;

    /**
     * Temporary storage for an OK package
     */
    struct st_ok_package m_ok_package;

    /**
     * Temporary storage for an error package
     */
    struct st_error_package m_error_package;
    
    /**
     * each bin log event starts with a 19 byte long header
     * We use this sturcture every time we initiate an async
     * read.
     */
    boost::uint8_t m_event_header[19];

    /**
     *
     */
    boost::uint8_t m_net_header[4];

    /**
     *
     */
    boost::uint8_t m_net_packet[MAX_PACKAGE_SIZE];
    boost::asio::streambuf m_event_stream_buffer;
    char * m_event_packet;
    
    /**
     * Used each time the client reconnects to the server to specify an
     * offset position.
     */
    unsigned long m_binlog_offset;

    std::string m_binlog_file_name;

    /**
     * This pointer points to an object constructed from event
     * stream during async communication with
     * server. If it is 0 it means that no event has been
     * constructed yet.
     */
    Binary_log_event *m_waiting_event;

    /**
     * A ring buffer used to dispatch aggregated events to the user application
     */
    bounded_buffer<Binary_log_event *> *m_event_queue;

    std::string m_user;
    std::string m_host;
    std::string m_passwd;
    long m_port;

    boost::uint64_t m_total_bytes_transferred;


};

/**
 * Sends a SHOW MASTER STATUS command to the server and retrieve the
 * current binlog position.
 *
 * @return False if the operation succeeded, true if it failed.
 */
bool fetch_master_status(tcp::socket *socket, std::string &filename, unsigned long &position);
/**
 * Sends a SHOW BINARY LOGS command to the server and stores the file
 * names and sizes in a map.
 */
bool fetch_binlogs_name_and_size(tcp::socket *socket, std::map<std::string, unsigned long> &binlog_map);

int authenticate(tcp::socket *socket, std::string user, std::string passwd, struct st_handshake_package &handshake_package);

tcp::socket *sync_connect_and_authenticate(boost::asio::io_service &io_service, const std::string &user, const std::string &passwd, const std::string &host, long port);


} }

#endif	/* _TCP_DRIVER_H */
