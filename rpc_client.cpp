#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "rpcrt4.lib")
#include <iostream>
#include "Interface.h"
#include <conio.h>
#include <string>
#include <fstream>
#include <windows.h>

using namespace std;

bool logged = false;

void exit_login()
{
	if (Exit_Login() == 1)
		logged = false;
	else cout << "Error!" << endl;
}

void delete_file()
{
	string path;
	const char *path_ch;

	cout << "Path: ";
	getline(cin, path);
	path_ch = path.c_str();
	if (Delete_File((const unsigned char*)path_ch) == 1) cout << "Deleted!" << endl;
	else cout << "Error!" << endl;
}


void copy_file()
{
	string cl_path, serv_path;

	const char *file_name_ch, *serv_path_ch;
	unsigned char *content;
	int size;
	unsigned char *mas;

	cout << "Path to the file on the client: " << endl;
	getline(cin, cl_path);
	FILE* f = fopen(cl_path.c_str(), "rb");
	if (f == NULL)
	{
		cout << "Error when opening the file!" << endl;
		return;
	}

	unsigned found = cl_path.find_last_of("/\\");
	cl_path = cl_path.substr(found + 1);
	file_name_ch = cl_path.c_str();

	cout << "The path to the folder on the server: " << endl;
	getline(cin, serv_path);
	serv_path_ch = serv_path.c_str();

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	content = (unsigned char *)malloc(size);
	if (content == 0)
	{
		cout << "Failure to allocate memory!" << endl;
		return;
	}
	fseek(f, 0, SEEK_SET);
	fread(content, sizeof(unsigned char), size, f);
	if (Copy_File((const unsigned char*)file_name_ch, (const unsigned char*)serv_path_ch, content, &size) == 1) cout << "Copied!" << endl;
	else cout << "Error!" << endl;
	free(content);
	fclose(f);
}

void download_file()
{
	const char *serv_path_ch, *client_path_ch;
	unsigned char *content;
	int size;
	string serv_path, client_path;
	cout << "Path to the file on the server: " << endl;
	getline(cin, serv_path);
	serv_path_ch = serv_path.c_str();

	cout << "The path to the folder on the client: " << endl;
	getline(cin, client_path);
	if (client_path[client_path.length() - 1] != '\\') client_path += "\\";
	unsigned found = serv_path.find_last_of("/\\");
	client_path += serv_path.substr(found + 1);

	if (Get_Size((const unsigned char*)serv_path_ch, &size) == 1)
	{
		content = (unsigned char *)malloc(size);
		if (content == 0)
		{
			cout << "Failure to allocate memory!" << endl;
			return;
		}
		if (Download_File((const unsigned char*)serv_path_ch, content, &size) == 1)
		{
			FILE* f = fopen(client_path.c_str(), "wb");
			if (f == NULL)
			{
				cout << "Error when opening the file!" << endl;
				return;
			}
			fseek(f, 0, SEEK_SET);
			fwrite(content, sizeof(unsigned char), size, f);
			fclose(f);
			cout << "Downloaded!" << endl;
		}
		else cout << "Error!" << endl;
		free(content);
	}
	else cout << "Error!" << endl;
}

void menu()
{
	
	cout << "Enter your command:";
	string command;
	getline(cin, command);
	if (command == "?" || command == "help") cout << "Commands: delete, download, copy" << endl;
	else if (command == "delete") delete_file();
	else if (command == "copy") copy_file();
	else if (command == "download") download_file();
	else if (command == "exit")  exit_login();
	else cout << "Unknown command!" << endl;
}

int login()
{
	const char *user_ch, *password_ch;
	string user, password;
	int res;

	while (true)
	{
		cout << "User: ";
		getline(cin, user);
		user_ch = user.c_str();
		cout << "Password: ";
		getline(cin, password);
		password_ch = password.c_str();
		//ввод учетных данных
		res = Impersonate((const unsigned char*)user_ch, (const unsigned char*)password_ch);
		if (res == 0)
			return 0;
		if (res == 1)
			cout << "Incorrect login/password" << endl;
		if (res == 2)
			cout << "Error!" << endl;
	}
}

int main()
{
	RPC_STATUS status;
	unsigned char* szStringBinding = NULL;
	char ip_addr[25] = { 0 };

	cout << "Enter ip address: ";
	cin >> ip_addr;
	// Creates a string binding handle.
	// This function is nothing more than a printf.
	// Connection is not done here.
	status = RpcStringBindingComposeA(
		NULL, // UUID to bind to.
		(RPC_CSTR)("ncacn_ip_tcp"), // Use TCP/IP protocol.
		(RPC_CSTR)(ip_addr), // TCP/IP network address to use.
		(RPC_CSTR)("4747"), // TCP/IP port to use.
		NULL, // Protocol dependent network options to use.
		&szStringBinding); // String binding output.

	if (status)
		exit(status);

	// Validates the format of the string binding handle and converts
	// it to a binding handle.
	// Connection is not done here either.
	status = RpcBindingFromStringBindingA(
		szStringBinding, // The string binding to validate.
		&hExample1Binding); // Put the result in the implicit binding
							// handle defined in the IDL file.

	if (status)
		exit(status);

	RpcTryExcept
	{
		// Calls the RPC function. The hExample1Binding binding handle
		// is used implicitly.
		// Connection is done here.
		while (true)
		{
			status = RpcMgmtIsServerListening(hExample1Binding);
			if (status)
			{
				if (status == RPC_S_NOT_LISTENING)
				{
					printf("RPC server is not listening.\n");
					main();
				}
				else
				{
					if (logged == true)
						menu();
					else
					{
						login();
						logged = true;
					}
				}
			}
		}

	}
		RpcExcept(1)
	{
		std::cerr << "Runtime reported exception " << RpcExceptionCode()
			<< std::endl;
	}
	RpcEndExcept

	// Free the memory allocated by a string.
	status = RpcStringFreeA(
		&szStringBinding); // String to be freed.

	if (status)
		exit(status);

	// Releases binding handle resources and disconnects from the server.
	status = RpcBindingFree(
		&hExample1Binding); // Frees the implicit binding handle defined in the IDL file.

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
