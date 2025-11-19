#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")// системная библиотека

using namespace std;

int main()
{
    //Key constants
    const char IP_SERV[] = "127.0.0.1";
    const int PORT_NUM = 1234;
    const int BUFF_SIZE = 1024;

    int erStat;
    // Этап 1: Привязка сокета к паре IP-адрес/Порт

    in_addr ip_to_num;// структура с сетевым адресом сокета
    erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);// inet_pton() - переводит строку типа char[] в структуру типа in_addr
	// AF_INET - семейство адресов, строка, содержащая IP-адрес в виде с точкой-разделителем
	// &ip_to_num - указатель на структуру in_addr, куда будет записан переведенный результат из строчного адреса в численный
    if (erStat <= 0)
    {
        cout << "IP translation error" << endl;
        return 1;
    }
    // Этап 2: Инициализация сокетных интерфейсов Win32API

    WSADATA wsData;// создание структуры, в которую загружаются данные о веряии сокета 
    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);// вызов функции запуска сокетов WSAStartup() 
	// (WORD запрашивает версию, &wsData указатель на структуру
	// WORD - тип данных, представляяет собой двухбайтовое слово

    if (erStat != 0)
    {
        cout << "WinSock init error: " << WSAGetLastError() << endl;
        return 1;
    }
    // Этап 3: Создание сокета и его инициализация

    SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0); // структура данных SOCKET инициализируется с помощью функции
        // socket (AF_INET - семейство адресов IPv4, SOCK_STREAM - тип сокета, 0 - тип протакола
    if (ServSock == INVALID_SOCKET)
    {
        cout << "Server socket error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in servInfo;// структура содержит ту же информацию, что и sockaddr, но в другом формате
    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;// семейство адресов
    servInfo.sin_addr = ip_to_num;// вложенная структура хранит сетевой адрес sin_addr
    servInfo.sin_port = htons(PORT_NUM);// порт htons(номер порта)

    erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));// назначение внешнего адреса
	// (ServSock - имя сокета, к которуму привязывается адрес и порт (хранит семейство адресов), (sockaddr*)&servInfo - указатель на структуру
	// с информацией ою адресе и порте, к которуму привязывается сокет(14 байтная упорядочная информация), sizeof(servInfo) - размер структуры
    if (erStat != 0)
    {
        cout << "Bind error: " << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    // Этап 4 (для сервера): «Прослушивание» привязанного порта для идентификации подключений
    erStat = listen(ServSock, SOMAXCONN);// ServSock - "слушащий" сокет, SOMAXCONN - максимальное количество процессов
	// разрешенных к подключению
	// для ограничения подключений SOMAXCONN_HINT(N) N- количество подключений (остальные будут сброшены)
    if (erStat != 0)
    {
        cout << "Listen error: " << WSAGetLastError() << endl;// программа не возобнавится до тех пор, пока не будет соединени с Клиентом/ошибка
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    // Этап 5 (только для Сервера). Подтверждение подключения

    sockaddr_in clientInfo;
    ZeroMemory(&clientInfo, sizeof(clientInfo));
    int clientInfo_size = sizeof(clientInfo);

    SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);// ServSock - слушащий
	// сокет на Сервере, (sockaddr*)&clientInfo - указатель на пустую структуру, &clientInfo_size - указатель на размер структуры
	// accept() - возвращает номер сокета в ОС
    if (ClientConn == INVALID_SOCKET)
    {
        cout << "Accept error: " << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }

    char clientIP[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);

    cout << "Client connected: " << clientIP << endl;

    char servBuff[BUFF_SIZE + 1];
    char clientBuff[BUFF_SIZE + 1];

    int packet_size = 0;

    while (true)
    {
        packet_size = recv(ClientConn, servBuff, BUFF_SIZE, 0);// ClientConn - сокет акцептованного соединения
		// servBuff - буфер приема информации, BUFF_SIZE - размер буфера
        if (packet_size <= 0)
        {
            break;
        }

        servBuff[packet_size] = '\0';
        cout << "Client: " << servBuff << endl;

        cout << "Host: ";
        if (!fgets(clientBuff, BUFF_SIZE, stdin))
        {
            break;
        }
        // Check whether server would like to stop chatting 
        int len = 0;
        while (clientBuff[len] != '\0' && clientBuff[len] != '\n')
        {
            len++;
        }
        clientBuff[len] = '\0';

        if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x')
        {
            break;
        }

        packet_size = send(ClientConn, clientBuff, len, 0);
        if (packet_size == SOCKET_ERROR)
        {
            cout << "Send error: " << WSAGetLastError() << endl;
            break;
        }
    }

    shutdown(ClientConn, SD_BOTH);
    closesocket(ClientConn);
    closesocket(ServSock);
    WSACleanup();

    return 0;
}
