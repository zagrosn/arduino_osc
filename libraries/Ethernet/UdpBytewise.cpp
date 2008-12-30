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
#include "UdpBytewise.h"

/* Start UDP socket, listening at local port PORT */
void UdpBytewiseClass::begin(uint16_t port) {
	_port = port;
	_sock = 0; //TODO: should not be hardcoded
	_txIndex =0;
	_rxIndex =0;
	_rxSize = 0;
	socket(_sock,Sn_MR_UDP,_port,0);
}


/* Is data available in rx buffer? Returns 0 if no, number of available bytes if yes. */
int UdpBytewiseClass::available() {
	if(_rxSize==0 || _rxSize-_rxIndex==0) { 
		//if local buffer is empty or depleted
		//check wiz5100 buffer for new packet
		_rxSize = getSn_RX_RSR(_sock); //this size is inflated by 8 byte header
		if(_rxSize){
			//if we have a new packet there
			//copy it into our local buffer
			_rxIndex=0;
			_rxSize = recvfrom(_sock,_rxBuffer,_rxSize-8,_rxIp,&_rxPort);
		} else {
			//else do nothing and rxsize is still 0
			;
		}
		return _rxSize; //return the new number of bytes in our buffer
	} else{
		//if buffer is not empty, return remaining # of bytes
		return (_rxSize-_rxIndex);
	}
	
}



int UdpBytewiseClass::beginPacket(uint8_t *ip, unsigned int port) { // returns 1 on success, 0 if we already started a packet
	if(_txIndex==0) {
		_txIp[0]=ip[0];
		_txIp[1]=ip[1];
		_txIp[2]=ip[2];
		_txIp[3]=ip[3];
		_txPort = port;
		return 1;
	}
	else {
		//we already started a packet
		return 0;
	}
}



// TODO: how do we indicate that we can't add to full buffer?
// or do we just send a full packet and start the next?
void UdpBytewiseClass::write(uint8_t b) {// add a byte to the currently assembled packet if there is space
	if(_txIndex>= UDP_TX_PACKET_MAX_SIZE)
		return;		
	_txBuffer[_txIndex++] = b;
}

int UdpBytewiseClass::endPacket(){ // returns # of bytes sent on success, 0 if there's nothing to send
	// send the packet
	uint16_t result = sendto(_sock,(const uint8_t *)_txBuffer,_txIndex,_txIp,_txPort);
	// reset buffer index
	_txIndex=0;
	// return sent bytes
	return (int)result;
}

int UdpBytewiseClass::read() {
	if(_rxIndex!=_rxSize) {
		//if there is something to be read
		//return the next byte
		return _rxBuffer[_rxIndex++];
		
	} else {
		//we already sent the last byte
		//reset our buffer
		_rxIndex=0;
		_rxSize=0;
		return -1;
	}
}
	
void UdpBytewiseClass::getSenderIp(uint8_t*ip) {
	ip[0]=_rxIp[0];
	ip[1]=_rxIp[1];
	ip[2]=_rxIp[2];
	ip[3]=_rxIp[3];
}
	
unsigned int  UdpBytewiseClass::getSenderPort() {
	return _rxPort;
}

/* Create one global object */
UdpBytewiseClass UdpBytewise;
