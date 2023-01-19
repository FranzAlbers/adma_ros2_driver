#include "adma_ros2_driver/adma_data_server.hpp"
#include <rclcpp_components/register_node_macro.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
/**
 * @brief This helper class can "simulate" the ADMA to send its message stream. 
 * can be used to validate the correct parsing of the driver node
 */
namespace genesys
{
        DataServer::DataServer(const rclcpp::NodeOptions &options) : Node("data_server", options),
        _sendSocketfd(-1),
        _socketAdress(),
        _adressLength(4)
        {
                // read ros parameters
                _frequence = this->declare_parameter("frequence", 20);
                 std::string ip_adress = this->declare_parameter("ip_address", "127.0.0.1");
                _port = this->declare_parameter("port", 1040);
                _protocolversion = this->declare_parameter("protocol_version", "v3.3.4");

                RCLCPP_INFO(get_logger(), "Working with: %s", _protocolversion.c_str());
                
                // setup socket for sending data
                 _sendSocketfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
                 _adressLength = sizeof(_socketAdress);
                memset((char *) &_socketAdress, 0, _adressLength);
                _socketAdress.sin_family = AF_INET;
                _socketAdress.sin_port = htons(_port);
                inet_aton(ip_adress.c_str(), &(_socketAdress.sin_addr));

                updateLoop();
        }

        DataServer::~DataServer(){
                ::shutdown(_sendSocketfd, SHUT_RDWR);
        }

        void DataServer::updateLoop(){
                unsigned char data_msg_v333[] = {
                        //static header (0-67)
                        0x47,0x42,0x49,0x4e, //genesys id
                        0x00,0x01,0x00,0x01, //header version
                        0x00,0x30,0x00,0x10, //format  id
                        0x00,0x04,0x03,0x03, //format version
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //reserved
                        0xe8,0x75,0x00,0x00, //serial number
                        0x41,0x44,0x4d,0x41,0x20,0x53,0x4e,0x3a,0x33,0x30,0x31,0x38,0x34,0x00,0x00,0x00, //alias
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //alias
                        //dynamic header (68-95)
                        0x0a,0x00,0x00,0x00,
                        0x02,0x00,0x00,0x00,
                        0x00,0x00,0x03,0x01,
                        0x0b,0x43,0x00,0x00,
                        0x44,0x17,0x00,0x00,
                        0x04,0x00,0x00,0x00,
                        0x49,0x36,0x20,0x45,
                        //status / error / warning (96-107)
                        0x11,0x20,0x00,0x81,0x00,0x02,0x00,0x00, //status
                        0x00,0x00,0x09,0x00,0x00,0x00,0x00,0x00, //error + warning
                        //acc + rate (108-295)
                        0x64,0x00,0x00,0x00,0x2d,0xff,0xff,0xff, //sensor body x
                        0x38,0x00,0x00,0x00,0xd1,0xfe,0xff,0xff, //sensor body y
                        0x0f,0x27,0x00,0x00,0x96,0xff,0xff,0xff, //sensor body z
                        0xfe,0xff,0xfd,0xff,0xff,0xff,0x00,0x00, //rates body
                        0xfe,0xff,0xfd,0xff,0xff,0xff,0x00,0x00, //rate horizontal
                        0x19,0x00,0x0e,0x00,0xc4,0x09,0x00,0x00, //acc body
                        0x00,0x00,0x03,0x00,0xc4,0x09,0x00,0x00, //acc hor
                        0x1c,0x00,0x0f,0x00,0xd4,0x09,0x00,0x00, //acc body poi1
                        0x1b,0x00,0x08,0x00,0xb9,0x09,0x00,0x00,
                        0x1d,0x00,0x08,0x00,0xc4,0x09,0x00,0x00,
                        0x19,0x00,0x0e,0x00,0xc4,0x09,0x00,0x00,
                        0x1b,0x00,0x10,0x00,0xd0,0x09,0x00,0x00,
                        0x1b,0x00,0x0c,0x00,0xc3,0x09,0x00,0x00,
                        0x19,0x00,0x0e,0x00,0xc4,0x09,0x00,0x00,
                        0x19,0x00,0x0e,0x00,0xc4,0x09,0x00,0x00,
                        0x03,0x00,0x05,0x00,0xd4,0x09,0x00,0x00, //acc hor poi 1
                        0x02,0x00,0xfd,0xff,0xb9,0x09,0x00,0x00,
                        0x04,0x00,0xfd,0xff,0xc4,0x09,0x00,0x00,
                        0x00,0x00,0x03,0x00,0xc4,0x09,0x00,0x00,
                        0x01,0x00,0x06,0x00,0xd0,0x09,0x00,0x00,
                        0x01,0x00,0x01,0x00,0xc3,0x09,0x00,0x00,
                        0x00,0x00,0x03,0x00,0xc4,0x09,0x00,0x00,
                        0x00,0x00,0x03,0x00,0xc4,0x09,0x00,0x00,
                        //velocity
                        0xfa,0xff,0xf9,0xff,0x00,0x00,0x00,0x00, //ext vel ana
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //ext vel dig
                        0xfa,0xff,0xf9,0xff,0x00,0x00,0x00,0x00, //ext vel corrected
                        //reserved
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        //miscellanous
                        0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00, //misc poi 1
                        0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
                        //triggers
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        //system data
                        0x10,0x27,0x59,0x01,0x28,0x00,0x42,0x00,
                        //GNSS
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS pos abs
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS pos rel
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS EPE
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS vel frame
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS EVE
                        0x3a,0x07,0x04,0x00,0x00,0x00,0x10,0x00, //GNSS time
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS aux data 1
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS aux data 2
                        0x18,0x00,0xc6,0xff,0xa0,0x8c,0x00,0x00, //INS angle and GNSS cog
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS height msl
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS dualant time
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS dual ant angle
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //GNSS Dualant ETE
                        //INS position height
                        0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0xbf,0xff,0xff,0xff,0xbb,0xff,0xff,0xff,
                        0xba,0xff,0xff,0xff,0x01,0x00,0x00,0x00,
                        0xe0,0xff,0xff,0xff,0xdd,0xff,0xff,0xff,
                        0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
                        //INS time
                        0x3a,0x08,0x04,0x00,0x00,0x00,0x12,0x00,
                        //INS position (pos+rel)
                        0x7d,0xc5,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0x00,0x7c,0x36,0x20,0x20,0x16,0x79,0x03,
                        //ins position POI (abs + rel)
                        0xcc,0xc6,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0x40,0x7d,0x36,0x20,0x18,0x16,0x79,0x03,
                        0x10,0xc5,0xf7,0x1c,0x35,0x71,0xb5,0x04,
                        0x40,0x7b,0x36,0x20,0xc0,0x15,0x79,0x03,
                        0x10,0xc5,0xf7,0x1c,0x30,0x72,0xb5,0x04,
                        0x40,0x7b,0x36,0x20,0x78,0x16,0x79,0x03,
                        0x7d,0xc5,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0x00,0x7c,0x36,0x20,0x20,0x16,0x79,0x03,
                        0x81,0xc6,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0xc0,0x7c,0x36,0x20,0x18,0x16,0x79,0x03,
                        0x75,0xc5,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0xc0,0x7b,0x36,0x20,0x18,0x16,0x79,0x03,
                        0x7d,0xc5,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0x00,0x7c,0x36,0x20,0x20,0x16,0x79,0x03,
                        0x7d,0xc5,0xf7,0x1c,0xb3,0x71,0xb5,0x04,
                        0x00,0x7c,0x36,0x20,0x20,0x16,0x79,0x03,
                        //ins vel hor + frame
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        //INS vel POIs
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        //INS EPE
                        0x2d,0x01,0x2d,0x01,0x2d,0x01,0x00,0x00,
                        //INS EVE + ETE
                        0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
                        //Analog In
                        0xfe,0xff,0xfa,0xff,0x00,0x00,0xfb,0xff,
                        //KF status
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        //GNSS receiver error
                        0x00,0x00,0x00,0x00,
                        //GNSS receiver status
                        0x00,0x00,0x00,0x00
                };

                unsigned char data_msg_v32[] = {
                        0x47,0x42,0x49,0x4e,0x00,0x01,
                        0x00,0x01,0x00,0x30,0x00,0x10,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xca,0x75,0x00,0x00,0x41,0x44,
                        0x4d,0x41,0x20,0x53,0x4e,0x3a,0x33,0x30,0x31,0x35,0x34,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0a,0x00,
                        0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x03,0x01,0xf3,0x42,0x00,0x00,0x08,0x31,
                        0x00,0x00,0x04,0x00,0x00,0x00,0x50,0x6f,0x73,0x69,0x12,0xa0,0x00,0x19,0x00,0x02,
                        0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x29,0x00,0x00,0x00,0x5b,0xff,
                        0xff,0xff,0xff,0xff,0xff,0xff,0x8d,0xff,0xff,0xff,0x13,0x27,0x00,0x00,0x57,0xfd,
                        0xff,0xff,0xfe,0xff,0xff,0xff,0xf9,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x0a,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0x09,
                        0x00,0x00,0xfe,0xff,0xff,0xff,0xc5,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0xfe,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x10,0x27,0x27,0x01,0x0d,0x00,0x4f,0x00,0x05,0x43,0xe5,0x1c,0xf4,0xaf,
                        0xb8,0x04,0x04,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0xfd,0x06,0xd5,0x05,0x18,0x0f,
                        0x4a,0x00,0x01,0x00,0xff,0xff,0xfb,0xff,0x00,0x00,0x87,0x00,0x71,0x00,0x24,0x01,
                        0x00,0x00,0xf2,0x9c,0x19,0x11,0xb4,0x08,0x00,0x00,0x00,0x00,0x0e,0x0f,0x00,0x00,
                        0x00,0x00,0x01,0x3c,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xe9,0xff,0xa0,0x8c,
                        0x00,0x00,0xc5,0x3d,0x00,0x00,0x06,0x13,0x00,0x00,0x64,0x3d,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x64,0x3d,0x00,0x00,0x64,0x3d,0x00,0x00,0x64,0x3d,0x00,0x00,0x64,0x3d,
                        0x00,0x00,0x64,0x3d,0x00,0x00,0x64,0x3d,0x00,0x00,0x64,0x3d,0x00,0x00,0x64,0x3d,
                        0x00,0x00,0x06,0x9d,0x19,0x11,0xb4,0x08,0x12,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x43,0xe5,0x1c,0xf2,0xaf,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0xb8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0xb3,0x00,0x95,0x00,0x83,0x01,0x00,0x00,0x01,0x01,0x03,0x02,0x02,0x01,
                        0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xfd,0xff
                };

                while(rclcpp::ok()){
                        // select desired datat packet to send.. 
                        if(_protocolversion == "v3.2")
                        {
                                ::sendto(_sendSocketfd, (void*)(&data_msg_v32), sizeof(data_msg_v32), 0, (struct sockaddr *) &_socketAdress, _adressLength);
                                // RCLCPP_INFO(get_logger(), "sended payload: %ld", sizeof(data_msg_v32));
                        }else if (_protocolversion == "v3.3.3" || _protocolversion == "v3.3.4")
                        {
                                ::sendto(_sendSocketfd, (void*)(&data_msg_v333), sizeof(data_msg_v333), 0, (struct sockaddr *) &_socketAdress, _adressLength);
                                // RCLCPP_INFO(get_logger(), "sended payload: %ld", sizeof(data_msg_v333));
                        }else{
                                RCLCPP_WARN(get_logger(), "Unsupported protocol version: %s", _protocolversion.c_str());
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / _frequence));
                }
                
        }
} // end namespace genesys
RCLCPP_COMPONENTS_REGISTER_NODE(genesys::DataServer)