/*
 * This file is part of QuickCPP.
 * (c) Copyright 2011 Jie Wang(twj31470952@gmail.com)
 *
 * QuickCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuickCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QuickCPP.  If not, see <http://www.gnu.org/licenses/>.
 */

//==============================================================================
//
// $Revision$
// $Date$
//
//==============================================================================
/**
	@class qc::net::DatagramPacket
	
	@brief Class to represent a datagram packet.

	Datagram packets are packets of data which are transmitted from one host 
	to another via connectionless UDP sockets.
	
	UDP datagram sockets are not connected to another host (they are @a connectionless)
	and for this reason, information about the address and port of the peer
	host is contained within the DatagramPacket class itself.

	The DatagramPacket also contains a pointer to a buffer where data can be sent
	from or received into.  The data buffer can either be provided by the 
	application or automatically allocated by the DatagramPacket constructor.
	When the data buffer is provided by the application, ownership of the buffer
	(and the corresponding responsibility to delete it) can be assigned to the
	DatagramPacket object or retained by the application.

	The data buffer and the function signatures used to access it refer to non-const
	Bytes.  This is due to the fact that DatagramPackets can be used to
	both send and receive data, therefore the contained buffer must be modifiable.
	However, if the application will only use the DatagramPacket to send data,
	then it is safe to use a const_cast<> when constructing the DatagramPacket from
	a constant buffer, as in the example below:-

   @code
   AutoPtr<DatagramSocket> rpSocket(new DatagramSocket);
   AutoPtr<InetAddress> rpDest = InetAddress::GetByName("udp.destination.net");
   const int port = 1234;
   const Byte* msg = "THIS IS A UDP MESSAGE";
   //send a UDP packet to the remote host
   rpSocket->send(DatagramPacket(const_cast<Byte*>(msg), strlen(msg), rpDest.get(), port));
   @endcode

	@since 1.3
*/
//==============================================================================

#include "DatagramPacket.h"
#include "InetAddress.h"

#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/base/NullPointerException.h"

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// DatagramPacket::DatagramPacket
//
/**
   Creates a DatagramPacket and automatically allocates a data buffer
   of a given size.

   @param bufSize the size of the buffer to automatically allocate.  The 
          value of @c bufSize may be zero, in which case no data will be
          sent from or received into this datagram packet.
*/
//==============================================================================
DatagramPacket::DatagramPacket(size_t bufSize)
: m_port(-1),
  m_bufSize(bufSize),
  m_dataLength(bufSize),
  m_bBufferOwner(true),
  m_pBuffer(bufSize ? new Byte[bufSize] : 0)
{
}

//==============================================================================
// DatagramPacket::DatagramPacket
//
/**
	Creates a DatagramPacket, providing an application-supplied data buffer.

	The supplied data buffer may be created on the stack or dynamically
	allocated from the free store.  Ownership of dynamically allocated buffers
	may be transferred from the application to the DatagramPacket object
	by setting the @c bAssignBuffer parameter to @c true.  When the application
	retains ownership of the data buffer (which must always be the case when the buffer
	is created on the stack), it is responsible for ensuring that the 
	buffer's lifetime exceeds the lifetime of the DatagramPacket which
	references it.

	@param pBuffer a pointer to an array of Bytes to use as a data buffer. The
	       buffer is non constant so that the DatagramPacket can be
	       used for both send and receive operations.  If @c bufSize is zero
	       then @c pBuffer may be null.
	@param bufSize the size of the supplied buffer, which may be zero.
	@param bAssignBuffer if set to @c true, the data buffer is automatically
	       deleted by the DatagramPacket's destructor.
	@throws NullPointerException if both @c pBuffer is null and @c bufSize is non-zero.
*/
//==============================================================================
DatagramPacket::DatagramPacket(Byte* pBuffer, size_t bufSize, bool bAssignBuffer)
: m_port(-1),
  m_bufSize(bufSize),
  m_dataLength(bufSize),
  m_bBufferOwner(bAssignBuffer),
  m_pBuffer(pBuffer)
{
	if(!pBuffer && bufSize) throw NullPointerException();
}

//==============================================================================
// DatagramPacket::DatagramPacket
//
/**
	Creates a DatagramPacket, providing an application-supplied data buffer and
	pre-defined destination details.

	This constructor provides tha ability to set the address and port number
	to which data in this datagram may be sent or, in the case of received datagrams,
	from which it may be received.

	The supplied data buffer may be created on the stack or dynamically
	allocated from the free store.  Ownership of dynamically allocated buffers
	may be transferred from the application to the DatagramPacket object
	by setting the @c bAssignBuffer parameter to @c true.  When the application
	retains ownership of the data buffer (which must always be the case when the buffer
	is created on the stack), it is responsible for ensuring that the 
	buffer's lifetime exceeds the lifetime of the DatagramPacket which
	references it.

	@param pBuffer a pointer to an array of Bytes to use as a data buffer. The
	       buffer is non constant so that the DatagramPacket can be
	       used for both send and receive operations.  If @c bufSize is zero
	       then @c pBuffer may be null.
	@param bufSize the size of the supplied buffer, which may be zero.
	@param pAddress the destination IP address to which data will be sent or from
	       which data may be received.
	@param port the destination port number or -1 if this datagram may be used to
	       receive messages from any port.
	@param bAssignBuffer if set to @c true, the data buffer is automatically
	       deleted by the DatagramPacket's destructor.
	@throws NullPointerException if both @c pBuffer is null and @c bufSize is non-zero.
*/
//==============================================================================
DatagramPacket::DatagramPacket(Byte* pBuffer, size_t bufSize,
                               InetAddress* pAddress, int port,
                               bool bAssignBuffer)
: m_port(port),
  m_bufSize(bufSize),
  m_dataLength(bufSize),
  m_bBufferOwner(bAssignBuffer),
  m_pBuffer(pBuffer),
  m_rpAddress(pAddress)
{
	if(!pBuffer && bufSize) throw NullPointerException();
}

//==============================================================================
// DatagramPacket::DatagramPacket
//
/**
	DatagramPacket destructor.  This will automatically delete the referenced 
	data buffer when it is marked as being owned by this DatagramPacket object.
*/
//==============================================================================
DatagramPacket::~DatagramPacket()
{
	if(m_bBufferOwner)
		delete m_pBuffer;
}

//==============================================================================
// DatagramPacket::getData
//
/**
	Returns a pointer to the data buffer referenced by this DatagramPacket.
*/
//==============================================================================
Byte* DatagramPacket::getData() const
{
	return m_pBuffer;
}

//==============================================================================
// DatagramPacket::getAddress
//
/**
	Returns an InetAddress which represents the IP address of the host from
	which this DatagramPacket was received or to which it will be sent.
	@returns an InetAddress representing the IP address from which this
	         datagram packet was received, or the address to which it will
	         be sent.  A null value indicates that the address has not yet
	         been set for send operations.
	@sa setAddress
*/
//==============================================================================
AutoPtr<InetAddress> DatagramPacket::getAddress() const
{
	return m_rpAddress;
}

//==============================================================================
// DatagramPacket::getLength
//
/**
	Returns the length (in bytes) of the data to be sent or, following a receive
	operation, the length of the data received.  The @c length field is initially
	set equal to the size of the data buffer, but is modified by calls to
	setLength() or DatagramSocket::receive().

	@returns the length (in bytes) of the data to be sent or received.
	@sa setLength
	@sa setData
*/
//==============================================================================
size_t DatagramPacket::getLength() const
{
	return m_dataLength;
}

//==============================================================================
// DatagramPacket::getBufferSize
//
/**
	Returns the size of the data buffer as specified in the constructor or one
	of the overloaded setData methods.

	@returns the size of the referenced data buffer.
	@sa getData
	@sa setData
*/
//==============================================================================
size_t DatagramPacket::getBufferSize() const
{
	return m_bufSize;
}

//==============================================================================
// DatagramPacket::getPort
//
/**
	Returns the port number on the remote host to which this datagram packet will
	be sent or from which it was received.

	A return value of @c -1 indicates that this datagram packet has not yet
	had a port number specified.

	@returns the port number on the remote host to which this datagram packet will
	be sent or from which it was received.
	@sa setPort
*/
//==============================================================================
int DatagramPacket::getPort() const
{
	return m_port;
}

//==============================================================================
// DatagramPacket::setData
//
/**
	Replaces the existing data buffer for this DatagramPacket with a new
	buffer of a different size.  
	
	If the existing buffer is 'owned' by this DatagramPacket then it is
	first deleted before allocating a new buffer.  Any data in the existing
	buffer is discarded.

	If the new size is less than the @c length field, the length field is
	reduced to be equal to the new buffer size.  Otherwise it is left unchanged.

	@param bufSize the size of the buffer to automatically allocate.  The 
           value of @c bufSize may be zero, in which case no data will be
           sent from or received into this datagram packet.
*/
//==============================================================================
void DatagramPacket::setData(size_t bufSize)
{
	//
	// If the contained buffer is managed by us, and is of the correct size
	// already, then simply return.
	//
	if(m_bBufferOwner && m_bufSize==bufSize)
		return;

	if(m_bBufferOwner)
		delete m_pBuffer;

	m_pBuffer = bufSize ? new Byte[bufSize] : 0;
	m_bBufferOwner = true;
	m_bufSize = bufSize;
	m_dataLength = (m_dataLength > bufSize) ? bufSize : m_dataLength; 
}

//==============================================================================
// DatagramPacket::setData
//
/**
	Replaces the existing data buffer for this DatagramPacket with the 
	supplied data buffer.  
	
	If the existing buffer is 'owned' by this DatagramPacket then it is
	first deleted before allocating a new buffer.

	The supplied data buffer may be created on the stack or dynamically
	allocated from the free store.  Ownership of dynamically allocated buffers
	may be transferred from the application to the DatagramPacket object
	by setting the @c bAssignBuffer parameter to @c true.  When the application
	retains ownership of the data buffer (which must always be the case when the buffer
	is created on the stack), it is responsible for ensuring that the 
	buffer's lifetime exceeds the lifetime of the DatagramPacket which
	references it.

	@param pBuffer a pointer to an array of Bytes to use as a data buffer. The
	       buffer is non constant so that the DatagramPacket can be
	       used for both send and receive operations.  If @c bufSize is zero
	       then @c pBuffer may be null.
	@param bufSize the size of the supplied buffer, which may be zero.
	@param bAssignBuffer if set to @c true, the data buffer is automatically
	       deleted by the DatagramPacket's destructor.
	@throws NullPointerException if both @c pBuffer is null and @c bufSize is non-zero.
*/
//==============================================================================
void DatagramPacket::setData(Byte* pBuffer, size_t bufSize, bool bAssignBuffer)
{
	if(!pBuffer && bufSize) throw NullPointerException();
	
	//
	// Delete the existing buffer it is managed by us, but test against the
	// situation where the application is passing us the same buffer again.
	//
	if(m_bBufferOwner && m_pBuffer != pBuffer)
		delete m_pBuffer;

	m_pBuffer = pBuffer;
	m_bBufferOwner = bAssignBuffer;
	m_bufSize = bufSize;
}

//==============================================================================
// DatagramPacket::setAddress
//
/**
	Sets the IP address of the host to which this datagram packet will be sent.
	Setting the address value has no affect on receive operations.  To specify the
	host from which data can be received, use the DatagramSocket::connect() function.
	
	@param pAddress a pointer to an InetAddress representing the IP address to which
	       this datagram packet will be sent.  It is permissible for this to be a null
	       pointer, in which case the DatagramSocket used to send this datagram packet
	       must be connected to a specific host/port.

	@sa getAddress
	@sa DatagramSocket::connect
*/
//==============================================================================
void DatagramPacket::setAddress(InetAddress* pAddress)
{
	m_rpAddress = pAddress;
}

//==============================================================================
// DatagramPacket::setLength
//
/**
	Sets the length (in bytes) of the datagram packet to send or the maximum 
	length to receive.  The specified length must not exceed the size of the
	referenced data buffer.
	
	When receiving a datagram packet, if the received packet is larger than
	the value of @c length, then it will be silently truncated to the specified
	length.

	@param length the length (in bytes) to send or receive.
	@throws IllegalArgumentException if @c length is less than the size of
	        the data buffer.

	@note  When a DatagramPacket is received via a DatagramSocket, its length 
	field is set equal to the size of the datagram packet received.  Unless this
	value is changed by a subsequent call to setLength(), this new length value
	will be used to control further send and receive operations.

	@sa getLength
*/
//==============================================================================
void DatagramPacket::setLength(size_t length)
{
	if(length > m_bufSize) throw IllegalArgumentException();

	m_dataLength = length;
}

//==============================================================================
// DatagramPacket::setPort
//
/**
	Sets the port number on the remote host(s) to which this datagram packet
	will be sent.  
	
	Setting the port number has no affect on receive operations.  To specify the
	host and port from which data can be received, use the
	DatagramSocket::connect() function.
	
	@param port the number of the port or -1 to unset the port, in which case 
	       the DatagramSocket used to send this datagram packet. must be
	       connected to a specific host/port.

	@sa getAddress
	@sa DatagramSocket::connect
*/
//==============================================================================
void DatagramPacket::setPort(int port)
{
	m_port = port;
}

QC_NET_NAMESPACE_END
