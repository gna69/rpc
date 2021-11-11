#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "rpcrt4.lib")
#include <iostream>
#include "Interface.h"
#include <fstream>
#include <string>
#include <windows.h>
#include <conio.h>

using namespace std;

// Server function.
void Output(
	/* [string][in] */ const unsigned char *szOutput)
{
	cout << szOutput << endl;
}

int Delete_File(
	/* [string][in] */ const unsigned char *path)
{
	if (remove((const char*)path) == -1)
		return 0; //error
	else
		return 1; //ok

}

int Copy_File(
	/* [string][in] */ const unsigned char *file_name,
	/* [string][in] */ const unsigned char *path_server,
	/* [size_is][in] */ unsigned char *content,
	/* [in] */ int *size)
{
	ofstream file((const char *)path_server);
	if (!file)
		return 0; //error
	else
	{
		file << content << endl; //write to file
		file.close();
		return 1;
	}
}

int Copy_File(const char *file_name, const char *path_server, unsigned char* content, int *size)
{
	string name, path;
	path = path_server;
	if (path_server[strlen(path_server) - 1] != '\\')
	{
		path += "\\";
	}
	path += file_name;
	FILE* f = fopen(path.c_str(), "wb");
	if (f == NULL)
	{
		return 0; //error when opening the file
	}
	fseek(f, 0, SEEK_SET);
	fwrite(content, sizeof(unsigned char), *size, f);
	fclose(f);
	return 1;
}

int Get_Size(
	/* [string][in] */ const unsigned char *path_file,
	/* [out] */ int *size)
{
	FILE* f = fopen((const char*)path_file, "rb");
	if (f == NULL)
	{
		return 0; //error when opening the file
	}
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fclose(f);
	return 1;
}

int Download_File(
	/* [string][in] */ const unsigned char *path_server,
	/* [size_is][out][in] */ unsigned char *content,
	/* [in] */ int *size)
{
	FILE* f = fopen((const char*)path_server, "rb");
	if (f == NULL)
	{
		return 0; //error when opening the file
	}

	fseek(f, 0, SEEK_SET);
	fread(content, sizeof(unsigned char), *size, f);
	fclose(f);
	return 1;
}

int Impersonate(
	/* [string][in] */ const unsigned char *user,
	/* [string][in] */ const unsigned char *password)
{
	handle_t handle;
	if (LogonUser((LPCWSTR)user, NULL, (LPCWSTR)password, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &handle) == 0) return 1;
	else
	{
		if (ImpersonateLoggedOnUser(handle) == 0) return 2;
		else return 0;
	}

}
//CloseHandle (lngTokenHandle)
int Exit_Login(void)
{
	if (RevertToSelf() != 0) return 1; //ok
	else return 0; //error
}

// Naive security callback.
RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE /*hInterface*/, void* /*pBindingHandle*/)
{
	return RPC_S_OK; // Always allow anyone.
}

int main()
{
	RPC_STATUS status;

	// Uses the protocol combined with the endpoint for receiving
	// remote procedure calls.
	status = RpcServerUseProtseqEpA(
		(RPC_CSTR)("ncacn_ip_tcp"), // Use TCP/IP protocol.
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // Backlog queue length for TCP/IP.
		(RPC_CSTR)("4747"), // TCP/IP port to use.
		NULL); // No security.

	if (status)
		exit(status);

	// Registers the Example1 interface.
	status = RpcServerRegisterIf2(
		Example1_v1_0_s_ifspec, // Interface to register.
		NULL, // Use the MIDL generated entry-point vector.
		NULL, // Use the MIDL generated entry-point vector.
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, // Forces use of security callback.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT, // Use default number of concurrent calls.
		(unsigned)-1, // Infinite max size of incoming data blocks.
		SecurityCallback); // Naive security callback.

	if (status)
		exit(status);

	// Start to listen for remote procedure
	// calls for all registered interfaces.
	// This call will not return until
	// RpcMgmtStopServerListening is called.
	status = RpcServerListen(
		1, // Recommended minimum number of threads.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT, // Recommended maximum number of threads.
		FALSE); // Start listening now.

	if (status)
		exit(status);
}

// Memory allocation function for RPC.
// The runtime uses these two functions for allocating/deallocating
// enough memory to pass the string to the server.
void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
	free(p);
}
