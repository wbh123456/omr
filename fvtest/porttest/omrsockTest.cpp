/*******************************************************************************
 * Copyright (c) 2019, 2020 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include "omrcfg.h"
#include "omrport.h"
#include "omrporterror.h"
#include "omrportsock.h"
#include "omrportsocktypes.h"
#include "testHelpers.hpp"

/**
 * Start a server which creates a socket, binds to an address/port and 
 * listens for clients.
 * 
 * @param[in] portLibrary
 * @param[in] addrStr The address of the server.
 * @param[in] port The server port.
 * @param[in] family Socket address family wanted.
 * @param[out] serverSocket A pointer to the server socket. 
 * @param[out] serverAddr The socket address of the server created.
 * 
 * @return 0 on success, return an error otherwise.
 */ 
int32_t
start_server(struct OMRPortLibrary *portLibrary, int32_t family, int32_t socktype, omrsock_socket_t *serverSocket, omrsock_sockaddr_t serverSockAddr) 
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	EXPECT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, serverSocket, family, socktype, OMRSOCK_IPPROTO_DEFAULT), 0);
	EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, *serverSocket, serverSockAddr), 0);

	if (OMRSOCK_STREAM == socktype) {
		EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, *serverSocket, 10), 0);
	}

	return 0;
}

/**
 * Create the client socket, and then connect to the server.
 *  
 * @param[in] portLibrary
 * @param[in] addrStr The address of the server.
 * @param[in] port The server port.
 * @param[in] family Socket address family wanted.
 * @param[in] sessionClientSocket A pointer to the client socket. 
 * @param[in] sessionClientAddr The socket address of the client created. 
 * 
 * @return 0 on success, return an error otherwise.
 */ 
int32_t
connect_client_to_server(struct OMRPortLibrary *portLibrary, const char *addrStr, const char *port, int32_t family, int32_t socktype, omrsock_socket_t *clientSocket, omrsock_sockaddr_t clientSockAddr, omrsock_sockaddr_t serverSockAddr) 
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	omrsock_addrinfo_t hints = NULL;
	OMRAddrInfoNode result;
	uint32_t length = 0;
	int32_t resultFamily;
	int32_t resultSocktype;
	int32_t resultProtocol;

	EXPECT_EQ(OMRPORTLIB->sock_getaddrinfo_create_hints(OMRPORTLIB, &hints, family, socktype, OMRSOCK_IPPROTO_DEFAULT, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_getaddrinfo(OMRPORTLIB, addrStr, port, hints, &result), 0);
	EXPECT_EQ(OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, &result, &length), 0);
	EXPECT_NE(length, uint32_t(0));

	/* Create a socket with results. */
	for (uint32_t i = 0; i < length; i++) {
		EXPECT_EQ(OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, &result, i, &resultFamily), 0);
		EXPECT_EQ(OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, &result, i, &resultSocktype), 0);
		EXPECT_EQ(OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, &result, i, &resultProtocol), 0);

		if(0 == OMRPORTLIB->sock_socket(OMRPORTLIB, clientSocket, resultFamily, resultSocktype, resultProtocol)) {
			EXPECT_NE(clientSocket, (void *)NULL);
			EXPECT_EQ(OMRPORTLIB->sock_addrinfo_address(OMRPORTLIB, &result, i, clientSockAddr), 0);
			EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, *clientSocket, clientSockAddr), 0);
			break;
		}
	}

	if(OMRSOCK_STREAM == resultSocktype) {
		EXPECT_EQ(OMRPORTLIB->sock_connect(OMRPORTLIB, *clientSocket, serverSockAddr), 0);
	}
	return 0;
}

/**
 * Test if port library is properly set up to run OMRSock tests. 
 *
 * This will iterate through all OMRSock functions in @ref omrsock.c and simply check 
 * that the location of the function is not NULL.
 *
 * @note Errors such as an OMRSock function call with a NULL address will be reported.
 */
TEST(PortSockTest, library_function_pointers_not_null)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	EXPECT_NE(OMRPORTLIB->sock_getaddrinfo_create_hints, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_getaddrinfo, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_addrinfo_length, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_addrinfo_family, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_addrinfo_socktype, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_addrinfo_protocol, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_freeaddrinfo, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_socket, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_bind, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_listen, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_accept, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_send, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_sendto, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_recv, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_recvfrom, (void *)NULL);
	EXPECT_NE(OMRPORTLIB->sock_close, (void *)NULL);
}

/**
 * Test the omrsock per thread buffer using @ref omrsock_getaddrinfo_create_hints
 * and all functions that extract details from the per thread buffer.
 * 
 * In this test, per thread buffer related functions set up a buffer to store
 * information such as the hints structures created in 
 * @ref omrsock_getaddrinfo_create_hints. Since the per thread buffer can't be easily 
 * directly tested, it will be tested with @ref omrsock_getaddrinfo_create_hints.
 *
 * @note Errors such as invalid arguments, and/or returning the wrong family or 
 * socket type from hints compared to the ones passed into hints, will be reported.
 */
TEST(PortSockTest, create_hints_and_element_extraction)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	omrsock_addrinfo_t hints = NULL;
	uint32_t length = 0;
	int32_t family = 0;
	int32_t sockType = 0;
	int32_t protocol = 0;

	int32_t hintsFamily = OMRSOCK_AF_UNSPEC;
	int32_t hintsSockType = OMRSOCK_STREAM;
	int32_t hintsProtocol = OMRSOCK_IPPROTO_TCP;
	int32_t hintsFlags = 0;

	OMRPORTLIB->sock_getaddrinfo_create_hints(OMRPORTLIB, &hints, hintsFamily, hintsSockType, hintsProtocol, hintsFlags);
	
	ASSERT_NE(hints, (void *)NULL);

	/* Testing invalid arguments: Pointer to OMRAddrInfoNode that is NULL */

	int32_t rc;
	rc = OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, NULL, &length);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, NULL, 0, &family);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, NULL, 0, &sockType);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, NULL, 0, &protocol);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	/* Testing invalid arguments: Index is bigger than the length when querying */

	rc = OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, hints, &length);
	EXPECT_EQ(rc, 0);

	rc = OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, hints, length, &family);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, hints, length, &sockType);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, hints, length, &protocol);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	/* Get and verify elements of the newly created hints. */

	rc = OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, hints, &length);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(length, uint32_t(1));

	rc = OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, hints, 0, &family);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(family, hintsFamily);

	rc = OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, hints, 0, &sockType);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(sockType, hintsSockType);

	rc = OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, hints, 0, &protocol);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(protocol, hintsProtocol);

	/* Recreate hints with different parameters, see if hints elements are overwriten properly. */

	hintsProtocol = OMRSOCK_IPPROTO_DEFAULT;
	hintsFlags = 6;

	OMRPORTLIB->sock_getaddrinfo_create_hints(OMRPORTLIB, &hints, hintsFamily, hintsSockType, hintsProtocol, hintsFlags);

	rc = OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, hints, &length);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(length, uint32_t(1));

	rc = OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, hints, 0, &family);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(family, hintsFamily);

	rc = OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, hints, 0, &sockType);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(sockType, hintsSockType);

	rc = OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, hints, 0, &protocol);
	EXPECT_EQ(rc, 0);
	EXPECT_EQ(protocol, hintsProtocol);

	/* Testing invalid arguments: User exposes API and changes length, then query. */

	hints->length = 5;

	rc = OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, hints, 3, &family);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, hints, 3, &sockType);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	rc = OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, hints, 3, &protocol);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);
}

/**
 * Test @ref omrsock_getaddrinfo, @ref omrsock_freeaddrinfo and all functions 
 * that extract details from @ref omrsock_getaddrinfo results.
 * 
 * In this test, the relevant variables are passed into  
 * @ref omrsock_getaddrinfo_create_hints. The generated hints are passed into 
 * @ref omrsock_getaddrinfo. The results generated are used to create a socket.
 *
 * Address families tested will include IPv4, IPv6(if supported on machine), 
 * unspecified family. Socket types tested will include Stream and Datagram.
 *
 * @note Errors such as failed function calls, and/or returning the wrong family or 
 * socket type from @ref omrsock_getaddrinfo compared to the ones passed into hints, 
 * will be reported.
 */
TEST(PortSockTest, getaddrinfo_and_freeaddrinfo)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRAddrInfoNode result;
	OMRSockAddrStorage sockAddr;
	omrsock_addrinfo_t hints = NULL;
	omrsock_socket_t socket = NULL;
	int32_t rc = 0;
	uint32_t length = 0;
	int32_t family = 0;
	int32_t sockType = 0;
	int32_t protocol = 0;

	int32_t hintsFamily = OMRSOCK_AF_INET;
	int32_t hintsSockType = OMRSOCK_STREAM;
	int32_t hintsProtocol = OMRSOCK_IPPROTO_DEFAULT;
	int32_t hintsFlags = 0;

	rc = OMRPORTLIB->sock_getaddrinfo_create_hints(OMRPORTLIB, &hints, hintsFamily, hintsSockType, hintsProtocol, hintsFlags);
	EXPECT_EQ(rc, 0);

	/* Testing invalid arguments: getaddrinfo pointer to result is NULL. */
	omrsock_addrinfo_t resultPtr = NULL;

	rc = OMRPORTLIB->sock_getaddrinfo(OMRPORTLIB, (char *)"localhost", NULL, hints, resultPtr);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	/* Testing invalid arguments: freeaddrinfo pointer to result is NULL. */
	rc = OMRPORTLIB->sock_freeaddrinfo(OMRPORTLIB, resultPtr);
	EXPECT_EQ(rc, OMRPORT_ERROR_INVALID_ARGUMENTS);

	/* Verify that omrsock_getaddrinfo and omrsock_freeaddrinfo works with NULL hints. */
	rc = OMRPORTLIB->sock_getaddrinfo(OMRPORTLIB, (char *)"localhost", NULL, NULL, &result);
	ASSERT_EQ(rc, 0);

	OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, &result, &length);
	ASSERT_NE(length, uint32_t(0));
	
	OMRPORTLIB->sock_freeaddrinfo(OMRPORTLIB, &result);

	/* Get and verify that omrsock_getaddrinfo and omrsock_freeaddrinfo works. */
	rc = OMRPORTLIB->sock_getaddrinfo(OMRPORTLIB, (char *)"localhost", NULL, hints, &result);
	ASSERT_EQ(rc, 0);

	OMRPORTLIB->sock_addrinfo_length(OMRPORTLIB, &result, &length);
	ASSERT_NE(length, uint32_t(0));

	for (uint32_t i = 0; i < length; i++) {
		rc = OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, &result, i, &family);
		EXPECT_EQ(rc, 0);
		EXPECT_EQ(family, hintsFamily);

		rc = OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, &result, i, &sockType);
		EXPECT_EQ(rc, 0);
		EXPECT_EQ(sockType, hintsSockType);

		rc = OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, &result, i, &protocol);
		EXPECT_EQ(rc, 0);
	}

	/* Try to create a socket with results. */
	for (uint32_t i = 0; i < length; i++) {
		EXPECT_EQ(OMRPORTLIB->sock_addrinfo_family(OMRPORTLIB, &result, i, &family), 0);
		EXPECT_EQ(OMRPORTLIB->sock_addrinfo_socktype(OMRPORTLIB, &result, i, &sockType), 0);
		EXPECT_EQ(OMRPORTLIB->sock_addrinfo_protocol(OMRPORTLIB, &result, i, &protocol), 0);

		rc = OMRPORTLIB->sock_socket(OMRPORTLIB, &socket, family, sockType, protocol);
		if(0 == rc) {
			ASSERT_NE(socket, (void *)NULL);
			ASSERT_EQ(OMRPORTLIB->sock_addrinfo_address(OMRPORTLIB, &result, i, &sockAddr), 0);
			break;
		}
	}

	EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, socket, &sockAddr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, socket, 10), 0);
	
	rc = OMRPORTLIB->sock_close(OMRPORTLIB, &socket);
	EXPECT_EQ(rc, 0);

	rc = OMRPORTLIB->sock_freeaddrinfo(OMRPORTLIB, &result);
	EXPECT_EQ(rc, 0);
}

/**
 * Test functions involving IPv4 and IPv6 socket addresses:
 * To Create a Socket Address with INADDR_ANY IPv4 address
 *
 * After creating the address, their ability to bind and listen will be tested.
 * 
 * Address families tested is IPv4.
 */
TEST(PortSockTest, create_addressany_IPv4_socket_address)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage sockAddr;
	omrsock_socket_t socket = NULL;
	uint16_t port = 4930;
	uint8_t addr[4];

	uint32_t inaddrAny = OMRPORTLIB->sock_htonl(OMRPORTLIB, OMRSOCK_INADDR_ANY);
	memcpy(addr, &inaddrAny, 4);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init(OMRPORTLIB, &sockAddr, OMRSOCK_AF_INET, addr, OMRPORTLIB->sock_htons(OMRPORTLIB, port)), 0);
	EXPECT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, &socket, OMRSOCK_AF_INET, OMRSOCK_STREAM, OMRSOCK_IPPROTO_DEFAULT), 0);
	EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, socket, &sockAddr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, socket, 10), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &socket), 0);
}

/**
 * Test functions involving IPv4 and IPv6 socket addresses:
 * To Create a Socket Address with dotted-decimal IPv4 address.
 *
 * After creating the address, their ability to bind and listen will be tested.
 * 
 * Address families tested is IPv4.
 */
TEST(PortSockTest, create_dotted_decimal_IPv4_socket_address)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage sockAddr;
	omrsock_socket_t socket = NULL;
	uint16_t port = 4930;
	uint8_t addr[4];

	EXPECT_EQ(OMRPORTLIB->sock_inet_pton(OMRPORTLIB, OMRSOCK_AF_INET, "127.0.0.1", addr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init(OMRPORTLIB, &sockAddr, OMRSOCK_AF_INET, addr, OMRPORTLIB->sock_htons(OMRPORTLIB, port)), 0);
	EXPECT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, &socket, OMRSOCK_AF_INET, OMRSOCK_STREAM, OMRSOCK_IPPROTO_DEFAULT), 0);
	EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, socket, &sockAddr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, socket, 10), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &socket), 0);
}

/**
 * Test functions involving IPv4 and IPv6 socket addresses:
 * To Create a Socket Address with the loopback IPv6 address.
 *
 * After creating the address, their ability to bind and listen will be tested.
 * 
 * Address families tested include IPv4 and IPv6.
 */
TEST(PortSockTest, create_IPv6_socket_address)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage sockAddr;
	omrsock_socket_t socket = NULL;
	uint16_t port = 4930;
	uint8_t addr6[16];

	EXPECT_EQ(OMRPORTLIB->sock_inet_pton(OMRPORTLIB, OMRSOCK_AF_INET6, "::1", addr6), 0);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init6(OMRPORTLIB,  &sockAddr, OMRSOCK_AF_INET6, addr6, OMRPORTLIB->sock_htons(OMRPORTLIB, port), 0, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, &socket, OMRSOCK_AF_INET6, OMRSOCK_STREAM, OMRSOCK_IPPROTO_DEFAULT), 0);
	if (0 != (OMRPORTLIB->sock_bind(OMRPORTLIB, socket, &sockAddr))) {
		portTestEnv->log(LEVEL_ERROR, "WARNING: Cannot test IPV6: Failed to find to IPV6 interface. \n");
		EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &socket), 0);
		return;
	}
	EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, socket, 10), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &socket), 0);
}

/**
 * Test functions involving IPv4 and IPv6 socket addresses:
 * To Create a Socket Address with the in6addr_any IPv6 address.
 *
 * After creating the address, their ability to bind and listen will be tested.
 * 
 * Address families tested include IPv4 and IPv6.
 */
TEST(PortSockTest, create_in6addrany_IPv6_socket_address)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage sockAddr;
	omrsock_socket_t socket = NULL;
	uint16_t port = 4930;
	uint8_t addr6[16];

	EXPECT_EQ(OMRPORTLIB->sock_inet_pton(OMRPORTLIB, OMRSOCK_AF_INET6, "::0", addr6), 0);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init6(OMRPORTLIB, &sockAddr, OMRSOCK_AF_INET6, addr6, OMRPORTLIB->sock_htons(OMRPORTLIB, port), 0, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, &socket, OMRSOCK_AF_INET6, OMRSOCK_STREAM, OMRSOCK_IPPROTO_DEFAULT), 0);
	EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, socket, &sockAddr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, socket, 10), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &socket), 0);
}

/**
 * Test functions involving IPv4 and IPv6 socket addresses:
 * To Create a Socket Address with IPv4-mapped IPv6 address.
 *
 * After creating the address, their ability to bind and listen will be tested.
 * 
 * Address families tested include IPv4 and IPv6.
 */
TEST(PortSockTest, create_IPv4_mapped_IPv6_Socket_Address)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage sockAddr;
	omrsock_socket_t socket = NULL;
	uint16_t port = 4930;
	uint8_t addr[4];

	EXPECT_EQ(OMRPORTLIB->sock_inet_pton(OMRPORTLIB, OMRSOCK_AF_INET, "127.0.0.1", addr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init6(OMRPORTLIB, &sockAddr, OMRSOCK_AF_INET, addr, OMRPORTLIB->sock_htons(OMRPORTLIB, port), 0, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, &socket, OMRSOCK_AF_INET6, OMRSOCK_STREAM, OMRSOCK_IPPROTO_DEFAULT), 0);
	EXPECT_EQ(OMRPORTLIB->sock_bind(OMRPORTLIB, socket, &sockAddr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_listen(OMRPORTLIB, socket, 10), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &socket), 0);
}

/**
 * Test functions to set up a stream connection by using two sockets, which talk to each other.
 *
 * First, the server starts and listens for connections. Then, the
 * client starts and sends a request to connect to the server. The messages are
 * sent both ways, and it is checked if they were sent correctly.
 * 
 * Address families tested include IPv4 and socket types tested is stream.
 *
 * @note Errors such as failed function calls, failure to create server and/or client, wrong 
 * message sent/received, will be reported.
 */
TEST(PortSockTest, two_socket_stream_communication)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage serverStreamSockAddr;
	omrsock_socket_t serverStreamSocket = NULL;
	uint16_t port = 4930;
	uint8_t serverAddr[4];

	/* To Create a Server Socket and Address */
	uint32_t inaddrAny = OMRPORTLIB->sock_htonl(OMRPORTLIB, OMRSOCK_INADDR_ANY);
	memcpy(serverAddr, &inaddrAny, 4);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init(OMRPORTLIB, &serverStreamSockAddr, OMRSOCK_AF_INET, serverAddr, OMRPORTLIB->sock_htons(OMRPORTLIB, port)), 0);
	EXPECT_EQ(start_server(OMRPORTLIB, OMRSOCK_AF_INET, OMRSOCK_STREAM, &serverStreamSocket, &serverStreamSockAddr), 0);

	/* To Create a Client Socket and Address */
	OMRSockAddrStorage clientStreamSockAddr;
	omrsock_socket_t clientStreamSocket = NULL;
	EXPECT_EQ(connect_client_to_server(OMRPORTLIB, (char *)"localhost", NULL, OMRSOCK_AF_INET, OMRSOCK_STREAM, &clientStreamSocket, &clientStreamSockAddr, &serverStreamSockAddr), 0);

	/* Accept Connection */
	OMRSockAddrStorage connectedClientStreamSockAddr;
	omrsock_socket_t connectedClientStreamSocket = NULL;
	EXPECT_EQ(OMRPORTLIB->sock_accept(OMRPORTLIB, serverStreamSocket, &connectedClientStreamSockAddr, &connectedClientStreamSocket), 0);

	const char *msg = "This is an omrsock test for 2 socket stream communications.";
	int32_t bytesLeft = strlen(msg) + 1;
	uint8_t *cursor = (uint8_t *)msg;
	int32_t bytesSent = 0;
	while (0 != bytesLeft) {
		bytesSent = OMRPORTLIB->sock_send(OMRPORTLIB, connectedClientStreamSocket, cursor, bytesLeft, 0);
		ASSERT_GE(bytesSent, 0);
		bytesLeft -= bytesSent;
		cursor += bytesSent;
	}

	char buf[100] = {0};
	bytesLeft = strlen(msg) + 1;
	cursor = (uint8_t *)buf;
	int32_t bytesRecv = 0;
	while (0 != bytesLeft) {
		bytesRecv = OMRPORTLIB->sock_recv(OMRPORTLIB, clientStreamSocket, cursor, bytesLeft, 0);
		ASSERT_GE(bytesRecv, 0);
		bytesLeft -= bytesRecv;
		cursor += bytesRecv;
	}
	
	EXPECT_STREQ(msg, buf);

	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &connectedClientStreamSocket), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &clientStreamSocket), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &serverStreamSocket), 0);
}

/**
 * Test functions to set up a datagram connection by using two sockets, which talk to each other.
 *
 * First, the server is set up, and then, the client is set up sends a request to 
 * connect to the server (this is optional). The messages are sent both ways, and 
 * it is checked if they were sent correctly.
 * 
 * Address families tested include IPv4 in this test case. Socket types
 * tested is datagram.
 *
 * @note Errors such as failed function calls, failure to create server and/or client, wrong 
 * message sent/received, will be reported.
 */
TEST(PortSockTest, two_socket_datagram_communication)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());

	OMRSockAddrStorage serverSockAddr;
	omrsock_socket_t serverSocket = NULL;
	uint16_t port = 4930;
	uint8_t serverAddr[4];

	/* To Create a Server Socket and Address */
	EXPECT_EQ(OMRPORTLIB->sock_inet_pton(OMRPORTLIB, OMRSOCK_AF_INET, "127.0.0.1", serverAddr), 0);
	EXPECT_EQ(OMRPORTLIB->sock_sockaddr_init(OMRPORTLIB,  &serverSockAddr, OMRSOCK_AF_INET, serverAddr, OMRPORTLIB->sock_htons(OMRPORTLIB, port)), 0);
	/* Datagram server sockets does not need to listen and accept */
	EXPECT_EQ(start_server(OMRPORTLIB, OMRSOCK_AF_INET, OMRSOCK_DGRAM, &serverSocket, &serverSockAddr), 0);

	OMRSockAddrStorage clientSockAddr;
	omrsock_socket_t clientSocket = NULL;
	/* Connect is optional for datagram clients */
	EXPECT_EQ(connect_client_to_server(OMRPORTLIB, (char *)"localhost", (char *)"16403", OMRSOCK_AF_INET, OMRSOCK_DGRAM, &clientSocket, &clientSockAddr, &serverSockAddr), 0);

	/* Datagram Sendto and Recvfrom test. Sendto and Recvfrom are called multiple tests to ensure it has been
	*	sent and received.
	*/
	OMRTimeval timeSend;
	OMRTimeval timeRecv;
	EXPECT_EQ(OMRPORTLIB->sock_timeval_init(OMRPORTLIB, &timeSend, 2, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_timeval_init(OMRPORTLIB, &timeRecv, 3, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_timeval(OMRPORTLIB, serverSocket, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_SNDTIMEO, &timeSend), 0);
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_timeval(OMRPORTLIB, clientSocket, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_RCVTIMEO, &timeRecv), 0);

	/* Send 50 times. This should be enough for the message to be sent. */
	char msgDgram[100] = "This is an omrsock test for datagram communications.";
	int32_t	bytesTotal = strlen(msgDgram) + 1;
	int32_t bytesSent = 0;

	for (int32_t i = 0; i < 100; i++) {
		bytesSent = OMRPORTLIB->sock_sendto(OMRPORTLIB, serverSocket, (uint8_t *)msgDgram, bytesTotal, 0, &clientSockAddr);
		if (bytesSent == bytesTotal) {
			break;
		}
	}

	/* Receive */
	char bufDgram[100] = "";
	int32_t bytesRecv = 0;

	for (int32_t i = 0; i < 100; i++) {
		bytesRecv = OMRPORTLIB->sock_recvfrom(OMRPORTLIB, clientSocket, (uint8_t *)bufDgram, bytesTotal, 0, &serverSockAddr);
		if (bytesRecv == bytesTotal) {
			break;
		}
	}

	EXPECT_EQ(strcmp(msgDgram, bufDgram), 0);

	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &clientSocket), 0);
	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &serverSocket), 0);
}

/**
 * Equal operator to compare contents of OMRTimeval struct socket option values.
 */
bool
operator==(const OMRTimeval &lhs, const OMRTimeval &rhs)
{
	return (lhs.data.tv_sec == rhs.data.tv_sec) && (lhs.data.tv_usec == rhs.data.tv_usec);
}

/**
 * Equal operator to compare contents of OMRLinger struct socket option values.
 */
bool
operator==(const OMRLinger &lhs, const OMRLinger &rhs)
{
	if (lhs.data.l_onoff != 0) {
		return (rhs.data.l_onoff != 0) && (lhs.data.l_linger == rhs.data.l_linger);
	}
	return rhs.data.l_onoff == 0;
}

/**
 * Custom print for contents of OMRTimeval struct when comparing contents in test unit.
 */
void
PrintTo(const OMRTimeval &x, std::ostream *os)
{
	*os << "{" << x.data.tv_sec << ", " <<  x.data.tv_usec << "}";
}

/**
 * Custome print for contents of OMRLinger struct when comparing contents in test unit.
 */
void
PrintTo(const OMRLinger &x, std::ostream *os)
{
	*os << "{" << x.data.l_onoff << ", " <<  x.data.l_linger << "}";
}

/**
 * Test socket options functions by test setting and getting the socket options.
 *
 * First, the linger and timeval structs will be allocated and the corresponding
 * init functions will be called to set up those structs. Then, an arbitrary IPv4
 * stream socket will be opened and the socket options will be set and get. The 
 * test will check if the set and get option values are the same.
 */
TEST(PortSockTest, socket_options)
{
	OMRPORT_ACCESS_FROM_OMRPORT(portTestEnv->getPortLibrary());
	OMRTimeval timeVal;
	OMRLinger lingerVal;
	omrsock_socket_t sock = NULL;

	EXPECT_EQ(OMRPORTLIB->sock_timeval_init(OMRPORTLIB, &timeVal, 2, 0), 0);
	EXPECT_EQ(OMRPORTLIB->sock_linger_init(OMRPORTLIB, &lingerVal, 1, 2), 0);
	ASSERT_EQ(OMRPORTLIB->sock_socket(OMRPORTLIB, &sock, OMRSOCK_AF_INET, OMRSOCK_STREAM, OMRSOCK_IPPROTO_DEFAULT), 0);

	/* Call omrsock_setsockopt to set socket options. */
	int32_t flag = 1;
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_int(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_KEEPALIVE, &flag), 0);
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_linger(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_LINGER, &lingerVal), 0);
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_timeval(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_RCVTIMEO, &timeVal), 0);
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_timeval(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_SNDTIMEO, &timeVal), 0);
	EXPECT_EQ(OMRPORTLIB->sock_setsockopt_int(OMRPORTLIB, sock, OMRSOCK_IPPROTO_TCP, OMRSOCK_TCP_NODELAY, &flag), 0);

	/* Call omrsock_getsockopt to check if the results matches the inputs earlier. */
	OMRTimeval timeResult;
	OMRLinger lingerResult;
	int32_t flagResult;

	EXPECT_EQ(OMRPORTLIB->sock_getsockopt_int(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_KEEPALIVE, &flagResult), 0);
	EXPECT_NE(flagResult, 0);

	EXPECT_EQ(OMRPORTLIB->sock_getsockopt_linger(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_LINGER, &lingerResult), 0);
	EXPECT_EQ(lingerResult, lingerVal);

	EXPECT_EQ(OMRPORTLIB->sock_getsockopt_timeval(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_RCVTIMEO, &timeResult), 0);
	EXPECT_EQ(timeResult, timeVal);

	EXPECT_EQ(OMRPORTLIB->sock_getsockopt_timeval(OMRPORTLIB, sock, OMRSOCK_SOL_SOCKET, OMRSOCK_SO_SNDTIMEO, &timeResult), 0);
	EXPECT_EQ(timeResult, timeVal);

	EXPECT_EQ(OMRPORTLIB->sock_getsockopt_int(OMRPORTLIB, sock, OMRSOCK_IPPROTO_TCP, OMRSOCK_TCP_NODELAY, &flagResult), 0);
	EXPECT_NE(flagResult, 0);

	EXPECT_EQ(OMRPORTLIB->sock_close(OMRPORTLIB, &sock), 0);
}
