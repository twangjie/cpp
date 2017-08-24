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
// Class FtpClient
/**
	@class qc::net::FtpClient
	
	@brief Creates and manages a client connection with a remote
	FTP server.

	The FTP protocol, described in 
	<a href="http://www.ietf.org/rfc/rfc959.txt">RFC 959</a>,
	facilitates the transfer of files from one host to another.  This class
	manages the socket connections with the remote server as well as providing
	a high-level interface to the commands that are defined by the protocol.

	The following example shows how FtpClient can simply be used
	to retrieve a remote file using FTP:

    @code
    FtpClient ftp;
    try
    {
        // Connect to the remote FTP server and login as a guest
        ftp.connect(QC_T("ftp.gnu.org"));
        ftp.login(QC_T("anonymous"), QC_T("joe@example.com"));
        // Copy the file: welcome.msg to the local current directory
        String filename = QC_T("welcome.msg");
        AutoPtr<FileOutputStream> rpOStream = new FileOutputStream(filename);
        ftp.retrieveFile(filename, rpOStream);
    }
    catch(IOException& e)
    {
        Console::cout() << e.toString() << endl;
    }
    @endcode

	<h4>FTP Restart</h4>
	FtpClient allows applications to restart failed transfers when the remote
	FTP server supports stream-mode restart.  Stream-mode restart is not
	specified in RFC 959, but is specified in 
	<a href="http://www.ietf.org/internet-drafts/draft-ietf-ftpext-mlst-16.txt">
	Extensions to FTP</a> and is widely supported.

	Where stream-mode restart is supported, restarting binary transfers is
	considerably easier than restarting ASCII transfers.  This is due to the
	fact that the FTP @a SIZE command, which reports the size of the remote
	file, reports the <i>transfer size</i> of the file, which is not necessarily
	the same as the physical size of the file.  The is described in more detail
	in the documentation for the getFileSize() method.
*/
//==============================================================================

#include "FtpClient.h"
#include "NvtAsciiInputStream.h"
#include "NvtAsciiOutputStream.h"
#include "Socket.h"
#include "ServerSocket.h"
#include "InetAddress.h"
#include "ProtocolException.h"

#include "QcCore/base/IllegalStateException.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/debug.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/util/StringTokenizer.h"

QC_NET_NAMESPACE_BEGIN

using namespace util;
using namespace io;

const int DATA_CONNECTION_OPEN = 125;
const int OPENING_DATA_CONNECTION = 150;
const int COMMAND_OK = 200;
const int FILE_STATUS = 213;
const int READY_FOR_NEW_USER = 220;
const int CONTROL_CONNECTION_CLOSED = 221;
const int CLOSING_DATA_CONNECTION = 226;
const int ENTERING_PASSIVE_MODE = 227;
const int USER_LOGGED_IN = 230;
const int FILE_ACTION_OK = 250;
const int DIRECTORY_CREATED = 257;
const int NEED_PASSWORD = 331;
const int FILE_ACTION_PENDING = 350;
const int TRANSFER_ABORTED = 426;
const int BAD_LOGIN = 530;
const int FILE_ACTION_NOT_TAKEN = 450;
const int ACTION_NOT_TAKEN = 550;

const int FTP_PORT = 21;

//==============================================================================
// FtpClient::FtpClient
//
/**
   Constructs a new FtpClient with default values for all properties.
*/
//==============================================================================
FtpClient::FtpClient() :
	m_nResponseCode(0),
	m_dataConnectionType(Active),
	m_transferType(Ascii),
	m_dataConnectionTimeout(0),
	m_bCheckInboundConnection(true)
{
}

//==============================================================================
// FtpClient::~FtpClient
//
/**
   Disconnects from the remote FTP server before destroying this FtpClient.

   @note It is possible to destroy an FtpClient while a data transfer operation
   is still in progress.  Even though the control connection with the remote
   server will be closed, the data connection will remain open until the
   transfer is complete.
*/
//==============================================================================
FtpClient::~FtpClient()
{
	try
	{
		disconnect();
	}
	catch(Exception& /*e*/)
	{
	}
}

//==============================================================================
// FtpClient::getDefaultPort
//
/**
   Returns the default port for FTP, which is 21.
*/
//==============================================================================
int FtpClient::getDefaultPort() const
{
	return FTP_PORT;
}

//==============================================================================
// FtpClient::preDisconnect
//
//==============================================================================
void FtpClient::preDisconnect()
{
	try
	{
		const String sQUIT = QC_T("QUIT");
		asyncCommand(sQUIT);
	}
	catch(Exception& /*e*/)
	{
	}
}

//==============================================================================
// FtpClient::postConnect
//
/**
   Protected function called when a socket connection has been established with 
   the TCP/IP network server.

   If this method is overridden in classes @a derived from FtpClient,
   it is important that this base class implementation is called
   in order to perform necessary initialization.
*/
//==============================================================================
void FtpClient::postConnect(const String& /*server*/, int /*port*/, size_t /*timeoutMS*/)
{
	const String encoding = QC_T("ISO-8859-1");

	m_rpControlWriter = new OutputStreamWriter(
	                    new NvtAsciiOutputStream(getOutputStream().get()), encoding);

	m_rpControlReader = new BufferedReader(
	                    new InputStreamReader(
						new NvtAsciiInputStream(getInputStream().get()), encoding));

	int response = readCommandResponse();
	if(response != READY_FOR_NEW_USER)
	{
		handleInvalidResponse(QC_T("connect"));
	}
}

//==============================================================================
// FtpClient::login
//
/**
   Sends a login request to the remote FTP server.

   Note that the user name and password are sent over the network in plain text,
   so it is not a good idea to use FTP authentication with sensitive data or
   passwords.

   @param user the userid
   @param password the password
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
void FtpClient::login(const String& user, const String& password)
{
	const String cmdUser = QC_T("USER ");
	int response = syncCommand(cmdUser + user);

	if(response == NEED_PASSWORD)
	{
		const String cmdPass = QC_T("PASS ");
		response = syncCommand(cmdPass + password);
	}

	if(response == BAD_LOGIN)
	{
		disconnect();
		throw IOException(QC_T("FTP user authentication failed"));
	}
	else if(response != USER_LOGGED_IN)
	{
		handleInvalidResponse(QC_T("connect"));
	}
}

//==============================================================================
// FtpClient::syncCommand
//
// Send a command to the FTP server and wait for a response
//==============================================================================
int FtpClient::syncCommand(const String& cmd)
{
	asyncCommand(cmd);
	return readCommandResponse();
}

//==============================================================================
// FtpClient::asyncCommand
//
// Send a command to the FTP server and wait for a response
//==============================================================================
void FtpClient::asyncCommand(const String& cmd)
{
	if(!isConnected())
		throw IllegalStateException(QC_T("not connected to FTP server"));

	m_rpControlWriter->write(cmd);
	m_rpControlWriter->write(QC_T("\r\n"));
	m_rpControlWriter->flush();
}

//==============================================================================
// FtpClient::retrieveFile
//
/**
   Retrieves the specified file from the remote server and makes it available
   as an InputStream.

   The application should read from the InputStream until it receives an EndOfFile
   marker.  At this point the application should check the success of the remote
   operation by calling dataTransferComplete().

   If this FtpClient is going to be used for further operations it is essential
   that dataTransferComplete() is called.  However, if the application is not
   interested in testing the success of the transfer, and no other FTP operations
   are required, the FtpClient may be deleted before the InputStream is fully
   processed.

   This method can be used to restart a failed retrieve operation
   by specifying a value for the @c offset parameter.  Note that this value refers
   to the number of bytes to skip from the network transfer, not a number of
   bytes from the remote file.  However, for binary transfers, these two values are 
   the same.

   @param path the file name to retrieve.
   @param offset the number of bytes of the transfer to skip.
   @returns An InputStream attached to the remote file.
   
   @throws IOException if an error occurs retrieving the file from the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws ProtocolException if a value has been provided for the @c offset
           parameter and the FTP server does not support the REST command for
           stream-mode operations.
   @throws IllegalStateException if the FtpClient is not connected.

   @sa dataTransferComplete()
*/
//==============================================================================
AutoPtr<InputStream> FtpClient::retrieveFile(const String& path, size_t offset)
{
	if(offset)
		restart(offset);

	const String sRETR = QC_T("RETR ");
	AutoPtr<InputStream> rpIS = inputDataCommand(sRETR + path);
	if(!rpIS)
	{
		handleInvalidFileResponse(sRETR, path); // throws an exception
		QC_DBG_ASSERT(false);
	}

	return rpIS;
}

//==============================================================================
// FtpClient::retrieveFile
//
/**
   Retrieves the specified file from the remote server and copies it to the
   supplied OutputStream.

   This method can be used to restart a failed retrieve operation
   by specifying a value for the @c offset parameter.  Note that this value refers
   to the number of bytes to skip from the network transfer, not a number of
   bytes from the remote file.  However, for binary transfers, these two values are 
   the same.

   @param path the file name to retrieve.
   @param pOut the OutputStream which will act as a sink for the file.
   @param offset the number of bytes of the transfer to skip.
   @throws NullPointerException if @c pOut is null.
   @throws IOException if an error occurs retrieving the file from the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws ProtocolException if a value has been provided for the @c offset
           parameter and the FTP server does not support the REST command for
           stream-mode operations.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
void FtpClient::retrieveFile(const String& path, OutputStream* pOut, size_t offset)
{
	if(!pOut) throw NullPointerException();

	if(offset)
		restart(offset);

	AutoPtr<InputStream> rpIS = retrieveFile(path);
	copyInputStream(rpIS.get(), pOut);
	dataTransferComplete();
}

//==============================================================================
// FtpClient::storeFile
//
/**
   Returns an OutputStream that can be used to write to the specified file
   on the FTP server.

   The application should write data to the OutputStream, finally closing the OutputStream
   to signal the end of file.  At this point the application should check the success of 
   the remote operation by calling dataTransferComplete().

   If this FtpClient is going to be used for further operations it is essential
   that dataTransferComplete() is called.  However, if the application is not
   interested in testing the success of the transfer, and no other FTP operations
   are required, the FtpClient may be deleted before the OutputStream is closed.

   This method can be used to restart a failed store operation
   by specifying a value for the @c offset parameter.  Note that this value refers
   to the number of <i>equivalent network transfer</i> bytes to seek to in the
   remote file, which may not be the same as the physical offset within the 
   remote file for ASCII-mode transfers.  For binary transfers, these two values
   may be considered the same.

   @param path the file name to store.
   @param offset the number of <i>network transfer bytes</i> that the remote FTP
          server will seek past before recommencing the store operation.
   @returns An OutputStream attached to the remote file.
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws ProtocolException if a value has been provided for the @c offset
           parameter and the FTP server does not support the REST command for
           stream-mode operations.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
AutoPtr<OutputStream> FtpClient::storeFile(const String& path, size_t offset)
{
	if(offset)
		restart(offset);

	const String sSTOR = QC_T("STOR ");
	AutoPtr<OutputStream> rpOS = outputDataCommand(sSTOR + path);
	if(!rpOS)
	{
		handleInvalidFileResponse(sSTOR, path);
		QC_DBG_ASSERT(false);
	}

	return rpOS;
}

//==============================================================================
// FtpClient::storeFile
//
/**
   Uses an InputStream as the source of bytes to write to the specified file
   on the FTP server.

   This method can be used to restart a failed store operation
   by specifying a value for the @c offset parameter.  Note that this value refers
   to the number of <i>equivalent network transfer</i> bytes to seek to in the
   remote file, which may not be the same as the physical offset within the 
   remote file for ASCII-mode transfers.  For binary transfers, these two values
   may be considered the same.

   @param path the file name to store
   @param pIn the InputStream source
   @param offset the number of <i>network transfer bytes</i> that the remote FTP
          server will seek past before recommencing the store operation.
   @throws NullPointerException if @c pIn is null.
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
void FtpClient::storeFile(const String& path, InputStream* pIn, size_t offset)
{
	if(!pIn) throw NullPointerException();

	if(offset)
		restart(offset);

	AutoPtr<OutputStream> rpOS = storeFile(path);
	copyInputStream(pIn, rpOS.get());
	rpOS->close();
	dataTransferComplete();
}

//==============================================================================
// FtpClient::appendFile
//
/**
   Returns an OutputStream that can be used to write to the specified file
   on the FTP server.  If the file already exists, the data written
   to the OutputStream is appended to the existing file; otherwise a new file
   is created.

   @param path the file name to append to
   @returns An OutputStream attached to the remote file.
   @throws IOException if an error occurs appending to the file on the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
AutoPtr<OutputStream> FtpClient::appendFile(const String& path)
{
	const String sAPPE = QC_T("APPE ");
	AutoPtr<OutputStream> rpOS = outputDataCommand(sAPPE + path);

	if(!rpOS)
	{
		handleInvalidFileResponse(sAPPE, path);
		QC_DBG_ASSERT(false);
	}

	return rpOS;
}

//==============================================================================
// FtpClient::appendFile
//
/**
   Uses an InputStream as the source of bytes to write to the specified file
   on the FTP server.  If the file already exists, the InputStream data
   is appended to the end of the file, otherwise a new file is created.

   @param path the file name to append to
   @param pIn the InputStream source
   @throws IOException if an error occurs either reading from the InputStream
           or transmitting the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws NullPointerException if @c pIn is null.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
void FtpClient::appendFile(const String& path, InputStream* pIn)
{
	AutoPtr<OutputStream> rpOS = appendFile(path);
	copyInputStream(pIn, rpOS.get());
	dataTransferComplete();
}

//==============================================================================
// FtpClient::renameFile
//
/**
   Renames a file on the remote FTP server.

   @param from the name of the existing file.
   @param to the new name for the file.
   @throws IOException if an error occurs renaming the file.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
void FtpClient::renameFile(const String& from, const String& to)
{
	const String RNFR = QC_T("RNFR ");
	const String RNTO = QC_T("RNTO ");
	int response = syncCommand(RNFR + from);
	if(response == FILE_ACTION_PENDING)
	{
		response = syncCommand(RNTO + to);
		if(response != FILE_ACTION_OK)
		{
			handleInvalidFileResponse(RNTO, to);
		}
	}
	else
	{
		handleInvalidFileResponse(RNFR, from);
	}
}

//==============================================================================
// FtpClient::listDetails
//
/**
   Retrieves from the FTP server a directory listing of the current directory
   and makes it available as an InputStream.

   The application should read from the InputStream until it receives an EndOfFile
   marker.  At this point the application should check the success of the remote
   operations by calling dataTransferComplete().

   If this FtpClient is going to be used for further operations it is essential
   that dataTransferComplete() is called.  However, if the application is not
   interested in testing the success of the transfer, and no other FTP operations
   are required, the FtpClient may be deleted before the InputStream is fully
   processed.

   @param spec a filename specification which may be used by the server to control
          which files are listed.
   @returns An InputStream containing the results of the FTP @c LIST command.
  
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.

   @sa dataTransferComplete()
*/
//==============================================================================
AutoPtr<InputStream> FtpClient::listDetails(const String& spec)
{
	String command = QC_T("LIST");
	if(spec.length())
		command += (String(QC_T(" ")) + spec);

	AutoPtr<InputStream> rpIS = inputDataCommand(command);
	if(!rpIS)
		throw IOException(m_lastResponseMessage);

	return rpIS;
}

//==============================================================================
// FtpClient::listNames
//
/**
   Retrieves from the FTP server a list of file names from the current directory
   and makes it available as an InputStream.

   The application should read from the InputStream until it receives an EndOfFile
   marker.  At this point the application should check the success of the remote
   operations by calling dataTransferComplete().

   If this FtpClient is going to be used for further operations it is essential
   that dataTransferComplete() is called.  However, if the application is not
   interested in testing the success of the transfer, and no other FTP operations
   are required, the FtpClient may be deleted before the InputStream is fully
   processed.

   @param spec a filename specification which may be used by the server to control
          which files are listed.

   @returns An InputStream containing the results of the FTP @c NLST command.
  
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.

   @sa dataTransferComplete()
*/
//==============================================================================
AutoPtr<InputStream> FtpClient::listNames(const String& spec)
{
	String command = QC_T("NLST");
	if(spec.length())
		command += (String(QC_T(" ")) + spec);

	AutoPtr<InputStream> rpIS = inputDataCommand(command);
	if(!rpIS)
		throw IOException(m_lastResponseMessage);

	return rpIS;
}

//==============================================================================
// FtpClient::inputDataCommand
//
// Private helper to allocate an InputStream associated with a data transfer
// command.
//
// The RFC requires that the data connection is closed before
// a confirmation is recvd on the control connection.  We don't 
// return a socket (just an Inputstream), so the client cannot close
// the socket - but that doesn't matter because the qc::net framework
// automatically shuts down unused simplex communication paths
// on the socket, so (because we don't request an OutputStream)
// the socket will automatically appear closed to the FTP server.
//
//==============================================================================
AutoPtr<InputStream> FtpClient::inputDataCommand(const String& cmd)
{
	AutoPtr<Socket> rpDataSocket = createDataConnection(cmd);
	if(rpDataSocket)
	{
		if(m_transferType == Ascii)
		{
			return new NvtAsciiInputStream(rpDataSocket->getInputStream().get());
		}
		else
		{
			return rpDataSocket->getInputStream();
		}
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// FtpClient::outputDataCommand
//
// Private helper to allocate an OutputStream associated with a data transfer
// command.  If the data transfer command fails then we return a null ptr -
// leaving it up to the caller to translate that into an appropriate exception.
//==============================================================================
AutoPtr<OutputStream> FtpClient::outputDataCommand(const String& cmd)
{
	AutoPtr<Socket> rpDataSocket = createDataConnection(cmd);

	if(rpDataSocket)
	{
		if(m_transferType == Ascii)
		{
			return new NvtAsciiOutputStream(rpDataSocket->getOutputStream().get());
		}
		else
		{
			return rpDataSocket->getOutputStream();
		}
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// FtpClient::createDataConnection
//
// Creates a data connection - whcih may be either active or passive
// depending on the options selected.
//
// Note: Will return null if the command fails in a predictable manner!
//==============================================================================
AutoPtr<Socket> FtpClient::createDataConnection(const String& command)
{
	if(!isConnected())
		throw IllegalStateException(QC_T("not connected to FTP server"));

	if(m_dataConnectionType == Passive)
		return createPassiveDataConnection(command);
	else
		return createActiveDataConnection(command);
}

//==============================================================================
// FtpClient::createPassiveDataConnection
//
// Passive Mode FTP
// ----------------
// As you know by now, the FTP protocol utilises two Tcp connections:
// the command connection and a data connection.
//
// In standard (AKA active) FTP, the data channel is created by the client
// listening on a socket (default port 20) and the server connecting to that
// port.  However, this regime causes difficulties for clients sitting behind
// firewalls because the firewall will normally prevent the (unknown) server
// connecting to the arbitrary FTP data port.
//
// In passive FTP it is the server which manages the data connection by
// listening for a connection on a server port.
//
// To enter passive mode, the client sends the PASV command.  The server
// should respond with a "227 entering passive mode (x,x,x,x,p,p)" message.
// The message tokens describe an IPv4 intenet address followed by two
// digits desribing the upper and lower 8-bits of the port number.
//
// In stream mode operations (the default), the data connection is closed
// to mark the end of each data transfer.  Additionally, the server only
// listens for a single connection on the passive port, so there is no 
// point in remembering the host/port assignement - it changes each time.
//==============================================================================
AutoPtr<Socket> FtpClient::createPassiveDataConnection(const String& command)
{
	const String sPASV = QC_T("PASV");  // Enter Passive mode

	int response = syncCommand(sPASV);

	if(response != ENTERING_PASSIVE_MODE)
	{
		handleInvalidResponse(sPASV);
	}

	StringTokenizer tokens(m_lastResponseMessage, QC_T("(,)"));
	String ipAddr;
	int port=0;
	for(int i=0; i<7; i++)
	{
		if(!tokens.hasMoreTokens())
		{
			handleInvalidResponse(sPASV);
		}

		String token = tokens.nextToken();
		switch (i)
		{
			case 2:
			case 3:
			case 4:
				ipAddr += QC_T(".");
			case 1:
				ipAddr += token;
				break;
			case 5:
				port = NumUtils::ToInt(token) * 256;
				break;
			case 6:
				port += NumUtils::ToInt(token);
				break;
		}
	}

	//
	// we have decoded the message, so now connect to
	// the designated IP address/Port.
	//
	AutoPtr<Socket> rpDataSocket = new Socket;
	rpDataSocket->connect(InetAddress::GetByName(ipAddr).get(), port, m_dataConnectionTimeout);

	response = syncCommand(command);

	//
	// If we get a bad response from the FTP server, this could be because the
	// file was not found.  We communicate this to the caller be returning
	// a null socket ptr, and the caller will translate the error into
	// an appropriate exception.
	//
	if(response != OPENING_DATA_CONNECTION && response != DATA_CONNECTION_OPEN)
	{
		rpDataSocket.release();
	}

	return rpDataSocket;
}

//==============================================================================
// FtpClient::createActiveDataConnection
//
/**
   Creates a local listening socket and informs the remote server of the 
   host address and port number to connect to.  When the host connection
   is received the listening socket is closed.

   If a timeout value has been specified by setDataConnectionTimeout()
   then the client will only wait for the specified time before
   throwing a SocketTimeoutException.

   If the @c CheckInboundConnection property has been set, then attempts
   to connect to the listening socket are checked to ensure they originate from
   the same network host as the FTP Server.
   @param command an FTP command to issue on the control connection before
          establishing the data connection
   @returns a connected Socket or null if the command returns an invalid
            (but recognized) response
*/
//==============================================================================
AutoPtr<Socket> FtpClient::createActiveDataConnection(const String& command)
{
	//
	// Create a ServerSocket bound to a kernel-assigned port.
	// We don't specify the local IP address to bind to, so any
	// local address will do, but we'll publish the port number of
	// the local IP address used by the TP client connection
	//
	AutoPtr<ServerSocket> rpServerSocket = new ServerSocket(0);
	rpServerSocket->setSoTimeout(m_dataConnectionTimeout);

	//
	// Inform the server which ipAddr/port to connect to
	// 
	//
	port(getLocalAddress().get(), rpServerSocket->getLocalPort());

	int response = syncCommand(command);

	//
	// An invalid response is indicated by returning
	// null.  The caller is responsible for translating this to an appropriate
	// exception
	//
	if(response != OPENING_DATA_CONNECTION && response != DATA_CONNECTION_OPEN)
	{
		return 0;
	}

	//
	// Accept a new connection (bear in mind the timeout though, this may throw)
	//
	AutoPtr<Socket> rpDataSocket = rpServerSocket->accept();

	//
	// Finally, if we need to check the validity of the inbound connection
	// do so now.
	//
	if(m_bCheckInboundConnection)
	{
		AutoPtr<InetAddress> rpRemoteAddr = rpDataSocket->getInetAddress();
		AutoPtr<InetAddress> rpControlAddr = getServerAddress();
		if(!rpRemoteAddr->equals(*rpControlAddr.get()))
		{
			String errMsg = QC_T("inbound data connection from invalid host: ");
			errMsg += rpRemoteAddr->toString();
			throw IOException(errMsg);
		}
	}
	
	return rpDataSocket;
}

//==============================================================================
// FtpClient::port
//
// Private helper function to issue the port command associated with an 
// active data connection (i.e. the client is passive).
//==============================================================================
void FtpClient::port(InetAddress* pAddr, int port)
{
	if(!pAddr) throw NullPointerException();

	const String sComma = QC_T(",");
	const String sPort = QC_T("PORT ");

	String address = pAddr->getHostAddress();
	StringUtils::ReplaceAll(address, '.', sComma);
	int hiPort = port >> 8;
	address += sComma + NumUtils::ToString(hiPort);
	int lowPort = port & 0xff;
	address += sComma + NumUtils::ToString(lowPort);

	int response = syncCommand(sPort + address);
	if(response != COMMAND_OK)
	{
		handleInvalidResponse(sPort);
	}
}

//==============================================================================
// FtpClient::readCommandResponse
//
//==============================================================================
int FtpClient::readCommandResponse() 
{
	if(!isConnected())
		throw IllegalStateException(QC_T("not connected to FTP server"));

	m_nResponseCode = 0;
	String strResponse;

	bool bMultiLine = false;

	while(true)
	{
		String responseLine;

		m_rpControlReader->readLine(responseLine);

		if(responseLine.size() < 4 && !bMultiLine)
		{
			throw ProtocolException(QC_T("FTP response too short"));
		}

		strResponse += responseLine;

		if(!bMultiLine)
		{
			m_nResponseCode = NumUtils::ToInt(responseLine.substr(0,3));
			bMultiLine = (responseLine[3] == '-');
			if(!bMultiLine)
			{
				break;
			}
		}
		else if(responseLine.size() > 3 && responseLine[3] == ' ')
		{
			int code = NumUtils::ToInt(responseLine.substr(0,3));
			if(code == m_nResponseCode)
			{
				break;
			}
		}
	}
	
	m_lastResponseMessage = strResponse;
	return m_nResponseCode;
}

//==============================================================================
// FtpClient::printWorkingDirectory
//
/**
   Returns the FTP server's current working directory.

   @throws IOException if an error occurs sending the request to the FTP server
   @throws ProtocolException if an invalid response is received from the 
           FTP server
*/
//==============================================================================
String FtpClient::printWorkingDirectory()
{
	const String sPWD = QC_T("PWD");    // Print working directory

	int response = syncCommand(sPWD);
	if(response == DIRECTORY_CREATED)
	{
		String ret;
		size_t sPos = m_lastResponseMessage.find('"');
		size_t ePos = m_lastResponseMessage.find_last_of('"');
		if(sPos == String::npos || ePos == String::npos || sPos == ePos)
		{
			handleInvalidResponse(sPWD);
		}

		return m_lastResponseMessage.substr(sPos+1, ePos-sPos-1);
	}
	else
	{
		handleInvalidResponse(sPWD);
		// should never reach here
		QC_DBG_ASSERT(false);
		return String(); 
	}
}

//==============================================================================
// FtpClient::changeWorkingDirectory
//
/**
   Changes the current working directory on the remote FTP server.

   @param @path the name of the directory
   @returns @c true if the current working directory was changed; @c false otherwise
   @throws IOException if an error occurs sending the request to the FTP server
   @throws ProtocolException if an invalid response is received from the 
           FTP server
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
bool FtpClient::changeWorkingDirectory(const String& path)
{
	const String sCWDb = QC_T("CWD ");  // Change working directory

	int response = syncCommand(sCWDb + path);

	// RFC 959 states that the correct response to CDUP is 200 - command_ok
	// But also states that it should respond with the same codes as 
	// CWD (250 - file_action_ok).  We accept both for both.
	//
	if(response == COMMAND_OK || response == FILE_ACTION_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// FtpClient::changeToParentDirectory
//
/**
   Changes the FTP server's working directory to the immediate parent of the 
   current working directory by issuing a @c CDUP command.
   
   @returns @c true if the current working directory was changed; @c false otherwise.
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
bool FtpClient::changeToParentDirectory()
{
	const String sCDUP = QC_T("CDUP");

	int response = syncCommand(sCDUP);
	//
	// RFC 959 states that the correct response to CDUP is 200 - command_ok
	// But also states that it should respond with the same codes as 
	// CWD (250 - file_action_ok).  We accept both for both.
	//
	if(response == COMMAND_OK || response == FILE_ACTION_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// FtpClient::handleInvalidFileResponse
//
// Private helper to handle error responses to file commands
//==============================================================================
void FtpClient::handleInvalidFileResponse(const String& cmd, const String& path)
{
	if(m_nResponseCode == FILE_ACTION_NOT_TAKEN || m_nResponseCode >= 530)
		throw IOException(m_lastResponseMessage);
	else
		handleInvalidResponse(cmd);
}

//==============================================================================
// FtpClient::handleInvalidResponse
//
// Private helper to disconnect and throw an exception when we receive
// a response that we just don't understand or expect.
//==============================================================================
void FtpClient::handleInvalidResponse(const String& cmd)
{
	String errMsg = QC_T("invalid FTP response received from ");
	errMsg += cmd;
	errMsg += QC_T(" command: ");
	errMsg += m_lastResponseMessage;

	disconnect();

	throw ProtocolException(errMsg);
}

//==============================================================================
// FtpClient::setTransferType
//
/**
   Sets the transfer type that will be used for subsequent data operations:
   TransferType::Ascii or TransferType::Binary.  Binary transfers treat 
   files as an opaque stream of bytes whereas Ascii transfers translate line-feeds
   into <CRLF> pairs for transmission over the network, and then translate
   these back into the format appropriate for the target platform.

   @throws IOException if an error occurs sending the request to the FTP server
   @throws ProtocolException if an invalid response is received from the 
           FTP server

   @sa getTransferType()
*/
//==============================================================================
void FtpClient::setTransferType(TransferType type)
{
	if(m_transferType != type)
	{
		String command = QC_T("TYPE ");

		if(type == Binary)
		{
			command += QC_T("I");
		}
		else
		{
			QC_DBG_ASSERT(type == Ascii);
			command += QC_T("A");
		}
		
		int response = syncCommand(command);
		if(response != COMMAND_OK)
		{
			handleInvalidResponse(command);
		}

		m_transferType = type;
	}
}

//==============================================================================
// FtpClient::getDataConnectionType
//
/**
   Returns the type of data connection to be used: FtpClient::Active or 
   FtpClient::Passive.

   @sa setDataConnectionType()
*/
//==============================================================================
FtpClient::DataConnectionType FtpClient::getDataConnectionType() const
{
	return m_dataConnectionType;
}

//==============================================================================
// FtpClient::getTransferType
//
/**
   Returns the transfer type that will be used for subsequent data operations:
   FtpClient::Ascii or FtpClient::Binary.

   @sa setTransferType()
*/
//==============================================================================
FtpClient::TransferType FtpClient::getTransferType() const
{
	return m_transferType;
}

//==============================================================================
// FtpClient::setDataConnectionType
//
/**
   Sets the type of data connection to be used for subsequent data 
   transmission operations.

   Two types of data connection are supported:-
   - @a FtpClient::Active where the client creates a listening socket and waits for
        an inbound connection from the FTP server
   - @a FtpClient::Passive where the client connects to a listening socket on the
        FTP server

   The default connection type is @a Active as this was the original mode
   of operation in early FTP implementations.  However, the @a Active mode
   can present difficulties when attempting to communicate through
   firewall software; @a Passive mode is often preferable in this situation.

   @param type the data connection type to be used.
   @sa getDataConnectionType()
*/
//==============================================================================
void FtpClient::setDataConnectionType(DataConnectionType type)
{
	m_dataConnectionType = type;
}

//==============================================================================
// FtpClient::getFileSize
//
/**
   Returns the transfer size of the remote file.

   This uses the @a SIZE FTP command which is not defined in RFC 959, but is
   usually implemented by FTP servers nonetheless.

   The @a SIZE command is supposed to return the <i>transfer size</i> of the 
   file, which is determined for the transfer mode in operation.  For IMAGE 
   mode, this will equate to the size (in bytes) of the remote file.  For 
   ASCII mode, this will equate to the number of bytes that will be used to
   transfer the file over the network, with line-feeds translated into
   <CRLF> pairs.
   
   <h4>Using getFileSize() to control restart operations</h4>
   getFileSize() can be used to restart remote store operations if the
   transfer mode is IMAGE (binary), but care must be taken when using it for
   ASCII mode transfers from UNIX-based hosts.
   
   Line feeds in text files on UNIX hosts are represented by a single <LF> character,
   and therefore, if a local file is fully transfered to a remote host, the
   getFileSize() command is likely to report a size larger than the actual size
   of the local file.

   @param path the path name of the file
   @throws IOException if an error occurs retrieving the file size from the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
size_t FtpClient::getFileSize(const String& path)
{
	const String sSIZEb = QC_T("SIZE ");
	String command = sSIZEb + path;
	int response = syncCommand(command);
	if(response == FILE_STATUS)
	{
		String strSize = m_lastResponseMessage.substr(4); // skip over the response code
		return NumUtils::ToLong(strSize);
	}
	else
	{
		handleInvalidFileResponse(command, path);
		// should not reach here
		QC_DBG_ASSERT(false);
		return 0;
	}
}

//==============================================================================
// FtpClient::getDataConnectionTimeout
//
/**
   Returns the timeout value (in milliseconds) used when establishing passive
   or active data connections.

   @sa setDataConnectionTimeout()
*/
//==============================================================================
size_t FtpClient::getDataConnectionTimeout() const
{
	return m_dataConnectionTimeout;
}

//==============================================================================
// FtpClient::getCheckInboundConnection
//
/**
   Returns the value of the @c CheckInboundConnection property.  When this is
   @c true, inbound connection attempts from remote FTP servers are checked to
   ensure they originate from the same host as the FTP server to which this FtpClient
   is connected.

   @sa setCheckInboundConnection()
*/
//==============================================================================
bool FtpClient::getCheckInboundConnection() const
{
	return m_bCheckInboundConnection;
}

//==============================================================================
// FtpClient::setDataConnectionTimeout
//
/**
   Sets a timeout value for establishing a data connection.

   @param timeoutMS the number of milliseconds that the FtpClient will wait for a data
		connection to be established before timing out.  A value of zero forces the
		FtpClient to wait forever.
   @sa getDataConnectionTimeout()
*/
//==============================================================================
void FtpClient::setDataConnectionTimeout(size_t timeoutMS)
{
	m_dataConnectionTimeout = timeoutMS;
}

//==============================================================================
// FtpClient::setCheckInboundConnection
//
/**
   Sets the @c CheckInboundConnection property.  When @c true (the default),
   connection attempts from remote hosts will be checked to ensure they come
   from the same network host as the FTP control connection.  An IOException
   is thrown if a connection request comes from a different host.

   This only takes effect when @a Active data connections are being used.

   @sa getCheckInboundConnection()
*/
//==============================================================================
void FtpClient::setCheckInboundConnection(bool bCheckInboundConnection)
{
	m_bCheckInboundConnection = bCheckInboundConnection;
}

//==============================================================================
// FtpClient::dataTransferComplete
//
/**
   Tests whether the preceding data transfer request completed successfully.

   Asynchronous data transfer requests (i.e. read operations
   that return an InputStream or write operations that return an OutputStream)
   signal their completion by returning InputStream::EndOfFile (for read)
   or by the application closing the OutputStream (for write).  In both cases,
   the remote FTP server sends a response message on the control connection
   to indicate if the remote operation completed successfully.  This method
   interrogates the response and indicates the success of the operation.

   @throws IOException if an error occurs reading the response from the FTP server
   @throws ProtocolException if an invalid response is received from the 
           FTP server
*/
//==============================================================================
void FtpClient::dataTransferComplete()
{
	int response = readCommandResponse();

	if(response != CLOSING_DATA_CONNECTION)
	{
		handleInvalidResponse(QC_T("data transfer complete"));
	}
}

//==============================================================================
// FtpClient::abortDataTransfer
//
/**
   Forces the FTP server to abort the current data transfer operation (if any)
   and close the associated data connection.
   @throws IOException if an error occurs sending the request to the FTP server.
   @throws ProtocolException if an invalid response is received from the 
           FTP server.
   @throws IllegalStateException if the FtpClient is not connected.
*/
//==============================================================================
void FtpClient::abortDataTransfer()
{
	int response = syncCommand(QC_T("ABORT"));

	if(response == TRANSFER_ABORTED)
	{
		// Following a 426, we expect a further reply
		response = readCommandResponse();
	}

	if(response != CLOSING_DATA_CONNECTION)
	{
		handleInvalidResponse(QC_T("ABORT"));
	}
}

//==============================================================================
// FtpClient::copyInputStream
//
// Private helper function that performs a buffered copy from an
// InputStream to an OutputStream.
//==============================================================================
void FtpClient::copyInputStream(InputStream* pFrom, OutputStream* pTo)
{
	if(!pFrom || !pTo) throw NullPointerException();

	const size_t BufferSize = 1024;
	ArrayAutoPtr<Byte> apBuffer(new Byte[BufferSize]);
	long bytesRead;
	while( (bytesRead = pFrom->read(apBuffer.get(), BufferSize)) != InputStream::EndOfFile)
	{
		pTo->write(apBuffer.get(), bytesRead);
	}
}

//==============================================================================
// FtpClient::copyInputStream
//
// Private helper function to issue the REST command.
//
// The offset value gives the number of octets of the immediately
// following transfer to not actually send, effectively causing the
// transmission to be restarted at a later point.  A value of zero
// effectively disables restart, causing the entire file to be
// transmitted.  The server will respond to the REST command with a
// 350 reply, indicating that the REST parameter has been saved, and
// that another command, which should be either RETR or STOR, should
// then follow to complete the restart.
//==============================================================================
void FtpClient::restart(size_t offset)
{
	String rest = QC_T("REST ");
	rest += NumUtils::ToString(offset);

	int response = syncCommand(rest);

	if(response != FILE_ACTION_PENDING)
	{
		handleInvalidResponse(QC_T("REST"));
		// should not reach here
		QC_DBG_ASSERT(false);
	}
}

QC_NET_NAMESPACE_END
