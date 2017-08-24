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

#ifndef QC_NET_FtpClient_h
#define QC_NET_FtpClient_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "TcpNetworkClient.h"

#include "QcCore/io/Writer.h"
#include "QcCore/io/BufferedReader.h"

QC_NET_NAMESPACE_BEGIN

using io::Writer;
using io::BufferedReader;

class QC_NET_PKG FtpClient : public TcpNetworkClient
{
public:

	enum TransferType { Binary /*!< Treats files an an opaque stream of bytes */,
	                    Ascii  /*!< Translates line-feeds into the appropriate local format */ };

	enum DataConnectionType { Passive /*!< Client connects to the (passive) server for data transfers */,
	                          Active  /*!< (Active) server connects to the client for data transfers */ };

	FtpClient();
	~FtpClient();

	virtual void preDisconnect();

	void login(const String& user, const String& password);

	DataConnectionType getDataConnectionType() const;
	TransferType getTransferType() const;
	size_t getDataConnectionTimeout() const;
	bool getCheckInboundConnection() const;

	String printWorkingDirectory();

	void setTransferType(TransferType type);
	void setDataConnectionType(DataConnectionType type);
	void setDataConnectionTimeout(size_t timeoutMS);
	void setCheckInboundConnection(bool bCheckInboundConnection);

	size_t getFileSize(const String& path);
	bool changeWorkingDirectory(const String& path);
	bool changeToParentDirectory();

	AutoPtr<InputStream> listDetails(const String& spec);
	AutoPtr<InputStream> listNames(const String& spec);

	AutoPtr<InputStream> retrieveFile(const String& path, size_t offset=0);
	void retrieveFile(const String& path, OutputStream* pOut, size_t offset=0);

	AutoPtr<OutputStream> storeFile(const String& path, size_t offset=0);
	void storeFile(const String& path, InputStream* pIn, size_t offset=0);

	AutoPtr<OutputStream> appendFile(const String& path);
	void appendFile(const String& path, InputStream* pIn);

	void dataTransferComplete();
	void abortDataTransfer();

	void renameFile(const String& from, const String& to);

protected: // Overridden TcpNetworkClient virtual methods
	virtual int getDefaultPort() const;
	virtual void postConnect(const String& server, int port, size_t timeoutMS);

private:
	virtual AutoPtr<InputStream> inputDataCommand(const String& command);
	virtual AutoPtr<OutputStream> outputDataCommand(const String& command);

	void port(InetAddress* pAddr, int port);
	int syncCommand(const String& command);
	void asyncCommand(const String& command);
	int readCommandResponse();
	void restart(size_t offset);

	void handleInvalidResponse(const String& cmd);
	void handleInvalidFileResponse(const String& cmd, const String& path);

	AutoPtr<Socket> createDataConnection(const String& command);
	AutoPtr<Socket> createPassiveDataConnection(const String& command);
	AutoPtr<Socket> createActiveDataConnection(const String& command);
	void copyInputStream(InputStream* pFrom, OutputStream* pTo);
	void copyOutputStream(OutputStream* pFrom, InputStream* pTo);

private:
	String                 m_lastResponseMessage;
	int                    m_nResponseCode;
	DataConnectionType     m_dataConnectionType;
	TransferType           m_transferType;
	AutoPtr<Writer>         m_rpControlWriter;
	AutoPtr<BufferedReader> m_rpControlReader;
	size_t                 m_dataConnectionTimeout;
	bool                   m_bCheckInboundConnection;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_FtpClient_h
