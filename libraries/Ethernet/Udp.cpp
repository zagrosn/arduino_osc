/*
 *  Udp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  Drop Udp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/ 
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/29/2008
 */

extern "C" {
#include "types.h"
#include "w5100.h"
#include "socket.h"
}

#include "Ethernet.h"
#include "Udp.h"

/* Start UDP socket, listening at local port PORT */
void UdpClass::begin(uint16_t port) {
	_port = port;
	_sock = 0; //TODO: should not be hardcoded
	_index =0;
	socket(_sock,Sn_MR_UDP,_port,0);
}


/* Send packet contained in buf of length len to peer at specified ip, and port */
/* Use this function to transmit binary data that might contain 0x00 bytes*/
/* This function returns sent data size for success else -1. */
uint16_t UdpClass::sendPacket(uint8_t * buf, uint16_t len,  uint8_t * ip, uint16_t port){
	return sendto(_sock,(const uint8_t *)buf,len,ip,port);
}

/* Send  zero-terminated string str as packet to peer at specified ip, and port */
/* This function returns sent data size for success else -1. */
uint16_t UdpClass::sendPacket(const char str[], uint8_t * ip, uint16_t port){	
	// compute strlen
	const char *s;
	for(s = str; *s; ++s);
	uint16_t len = (s-str);
	// send packet
	return sendto(_sock,(const uint8_t *)str,len,ip,port);
}
/* Is data available in rx buffer? Returns 0 if no, number of available bytes if yes. */
int UdpClass::available() {
	return getSn_RX_RSR(_sock);
}


/* Read a received packet into buffer buf (whis is of maximum length len); */
/* store calling ip and port as well. Call available() to make sure data is ready first. */
/* NOTE: I don't believe len is ever checked in implementation of recvfrom(),*/
/*       so it's easy to overflow buf. */
uint16_t UdpClass::readPacket(uint8_t * buf, uint16_t len, uint8_t *ip, uint16_t *port) {
	return recvfrom(_sock,buf,len,ip,port);
}

/* Read a received packet, throw away peer's ip and port.  See note above. */
uint16_t UdpClass::readPacket(uint8_t * buf, uint16_t len) {
	uint8_t ip[4];
	uint16_t port[1];
	return recvfrom(_sock,buf,len,ip,port);
}



uint8_t UdpClass::beginPacket(uint8_t *ip, uint16_t port) { // returns 1 on success, 0 if we already started a packet
	if(_index==0) {
		_remoteIp[0]=ip[0];
		_remoteIp[1]=ip[1];
		_remoteIp[2]=ip[2];
		_remoteIp[3]=ip[3];
		_remotePort = port;
		return 1;
	}
	else {
		//we already started a packet
		return 0;
	}
}



// TODO: how do we indicate that we can't add to full buffer?
// or do we just send a full packet and start the next?
void UdpClass::write(uint8_t b) {// add a byte to the currently assembled packet if there is space
	if(_index>= UDP_TX_PACKET_MAX_SIZE)
		return;		
	_buffer[_index++] = b;
}

uint16_t UdpClass::endPacket(){ // returns # of bytes sent on success, 0 if there's nothing to send
	// send the packet
	uint16_t result = sendPacket(_buffer,_index,_remoteIp,_remotePort);
	// reset buffer index
	_index=0;
	// return sent bytes
	return result;
}

/* Create one global object */
UdpClass Udp;
