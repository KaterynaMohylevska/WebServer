#include <string.h>
#include <sstream>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>


using namespace std;

int main()
{
    string filename; //= "/home/kateryna/WS/index.html";
    cout << "Enter path to html file: ";
    cin >> filename;
    int bufSize = 1024;

    char buffer[bufSize];

    // структура, хранящая информацию
    // об IP-адресе  слущающего сокета

    // Шаблон для инициализации структуры адреса
    struct sockaddr_in server_addr;

    cout << "create listening socket"<< endl;

    // AF_INET определяет, что используется сеть для работы с сокетом
    //  SOCK_STREAM Задаем потоковый тип сокета
    // Используем протокол TCP
    int listen_socket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
    // Если создание сокета завершилось с ошибкой, выводим сообщение,
    if (listen_socket < 0) {
        perror("ERROR opening socket");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8008); // HTTP-сервер будет висеть на 8000-м порту локалхоста
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

cout << "Binding"<< endl;
    /* Now bind the host address using bind() call.*/
    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR on binding");
        return 0;
    }

    // 5 - число одновременных TCP-соединений

    cout << "Listening"<< endl;
    listen(listen_socket,5);


    // Принимаем входящие соединения
    cout << "Accept"<< endl;
    int client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket < 0) {
        perror("ERROR on accept");
        return 0;
    }
    cout << "Go to http://127.0.0.1:8008/" << endl;
    int result = recv(client_socket, buffer, bufSize, 0);

    stringstream response; // сюда будет записываться ответ клиенту
    stringstream response_body; // тело ответа

    if (result < 0) {
        // ошибка получения данных
        perror("ERROR on recv");
        return 0;
    } else if (result == 0) {
        // соединение закрыто клиентом
        cout << "connection closed...\n";
    } else if (result > 0) {
        cout << "Response"<< endl;
        // Мы знаем фактический размер полученных данных, поэтому ставим метку конца строки
        // В буфере запроса.
        buffer[result] = '\0';


        char *buff;
        long size;
        ifstream file(filename, ios::in|ios::binary|ios::ate);
        size = file.tellg();
        file.seekg (0, ios::beg);
        buff = new char[size];
        file.read (buff, size);
        file.close();

        response_body << buff << buffer;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Version: HTTP/1.1\r\n"
                 << "Content-Type: text/html; charset=utf-8\r\n"
                 << "Content-Length: " << response_body.str().length()
                 << "\r\n\r\n"
                 << response_body.str();
        // Отправляем ответ клиенту с помощью функции send
        result = send(client_socket, response.str().c_str(),
                      response.str().length(), 0);

        if (result < 0) {
            // произошла ошибка при отправле данных
            perror("Error send failed");
        }

        close(client_socket);
        close(listen_socket);
        return 0;
    }
}
