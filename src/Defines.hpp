/*
 * Define.hpp
 *
 *  Created on: Apr 30, 2014
 *      Author: Pimenta
 */

#ifndef DEFINE_HPP_
#define DEFINE_HPP_

// network addresses
#define IP_MULTICAST            "224.69.69.69"
#define TCPUDP_MAIN             "6969"
#define TCP_HTTPSERVER          "8080"

// sizes
#define SIZE_UDPSOCKET_MAXLEN   0x1000
#define SIZE_HTTPSERVER_MAXLEN  0x1000
#define SIZE_FILEBUFFER_MAXLEN  0x4000

// times
#define MS_SLEEP                20
#define MS_DETECTFAILURE        15000
#define MS_WAITRECOVERY         5000
#define MS_SPEAK                1000
#define MS_LOGINSYNC            3000
#define MS_WAIT                 200

#endif /* DEFINE_HPP_ */
