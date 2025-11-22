#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

static int send_all(SOCKET s, const char* d, int n) {
    int sent = 0;
    while (sent < n) {
        int k = send(s, d + sent, n - sent, 0);
        if (k == SOCKET_ERROR) return SOCKET_ERROR;
        sent += k;
    }
    return sent;
}

void send_command(SOCKET s, const char* cmd) {
    send_all(s, cmd, (int)strlen(cmd));
}

void read_response(SOCKET s) {
    char rb[4096];
    int k = recv(s, rb, (int)sizeof(rb) - 1, 0);
    if (k > 0) {
        rb[k] = 0;
        printf("%s\n", rb);
    }
    else {
        printf("Connection lost or error.\n");
    }
}

int main() {
    setlocale(LC_ALL, "RUS");
    const char IP[] = "127.0.0.1";
    const int PORT = 1234;

    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) return 1;

    SOCKET cs = socket(AF_INET, SOCK_STREAM, 0);
    if (cs == INVALID_SOCKET) { WSACleanup(); return 1; }

    sockaddr_in si{};
    si.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &si.sin_addr);
    si.sin_port = htons(PORT);

    if (connect(cs, (sockaddr*)&si, sizeof(si)) != 0) {
        closesocket(cs);
        WSACleanup();
        printf("Connection failed.\n");
        return 1;
    }

    int choice;
    char buf[512];

    while (true) {
        printf("Выберите действие:\n");
        printf("1. Регистрация\n");
        printf("2. Вход\n");
        printf("3. Просмотр баланса\n");
        printf("4. Пополнение счета\n");
        printf("5. Снятие средств\n");
        printf("6. Перевод другому пользователю\n");
        printf("7. Обмен валют\n");
        printf("8. Выход\n");
        printf("Введите номер: ");

        if (scanf("%d", &choice) != 1) break;
        while (getchar() != '\n'); // Очистить буфер ввода

        switch (choice) {
        case 1: {
            char login[32], pass[16];
            double c0, c1, c2, c3, c4;
            printf("Введите логин: ");
            fgets(login, sizeof(login), stdin);
            login[strcspn(login, "\r\n")] = 0;
            printf("Введите пароль: ");
            fgets(pass, sizeof(pass), stdin);
            pass[strcspn(pass, "\r\n")] = 0;
            printf("Введите наличные валюты RUB USD EUR KZT JPY через пробел: ");
            scanf("%lf %lf %lf %lf %lf", &c0, &c1, &c2, &c3, &c4);
            while (getchar() != '\n');
            snprintf(buf, sizeof(buf), "REGISTER %s %s %.2f %.2f %.2f %.2f %.2f", login, pass, c0, c1, c2, c3, c4);
            send_command(cs, buf);
            read_response(cs);
            break;
        }
        case 2: {
            char login[32], pass[16];
            printf("Введите логин: ");
            fgets(login, sizeof(login), stdin);
            login[strcspn(login, "\r\n")] = 0;
            printf("Введите пароль: ");
            fgets(pass, sizeof(pass), stdin);
            pass[strcspn(pass, "\r\n")] = 0;
            snprintf(buf, sizeof(buf), "LOGIN %s %s", login, pass);
            send_command(cs, buf);
            read_response(cs);
            break;
        }
        case 3:
            send_command(cs, "BALANCE");
            read_response(cs);
            break;
        case 4: {
            int cur;
            double sum;
            printf("Выберите валюту (0-RUB,1-USD,2-EUR,3-KZT,4-JPY): ");
            scanf("%d", &cur);
            printf("Введите сумму: ");
            scanf("%lf", &sum);
            while (getchar() != '\n');
            snprintf(buf, sizeof(buf), "DEPOSIT %d %.2f", cur, sum);
            send_command(cs, buf);
            read_response(cs);
            break;
        }
        case 5: {
            int cur;
            double sum;
            printf("Выберите валюту (0-RUB,1-USD,2-EUR,3-KZT,4-JPY): ");
            scanf("%d", &cur);
            printf("Введите сумму: ");
            scanf("%lf", &sum);
            while (getchar() != '\n');
            snprintf(buf, sizeof(buf), "WITHDRAW %d %.2f", cur, sum);
            send_command(cs, buf);
            read_response(cs);
            break;
        }
        case 6: {
            char to[32];
            int cur;
            double sum;
            printf("Введите логин получателя: ");
            fgets(to, sizeof(to), stdin);
            to[strcspn(to, "\r\n")] = 0;
            printf("Выберите валюту (0-RUB,1-USD,2-EUR,3-KZT,4-JPY): ");
            scanf("%d", &cur);
            printf("Введите сумму: ");
            scanf("%lf", &sum);
            while (getchar() != '\n');
            snprintf(buf, sizeof(buf), "TRANSFER %s %d %.2f", to, cur, sum);
            send_command(cs, buf);
            read_response(cs);
            break;
        }
        case 7: {
            int from, to;
            double sum;
            printf("Из валюты (0-RUB,1-USD,2-EUR,3-KZT,4-JPY): ");
            scanf("%d", &from);
            printf("В валюту (0-RUB,1-USD,2-EUR,3-KZT,4-JPY): ");
            scanf("%d", &to);
            printf("Введите сумму: ");
            scanf("%lf", &sum);
            while (getchar() != '\n');
            snprintf(buf, sizeof(buf), "EXCHANGE %d %d %.2f", from, to, sum);
            send_command(cs, buf);
            read_response(cs);
            break;
        }
        case 8:
            send_command(cs, "QUIT");
            goto end_loop;
        default:
            printf("Неверный пункт меню.\n");
            break;
        }
    }
end_loop:
    shutdown(cs, SD_BOTH);
    closesocket(cs);
    WSACleanup();
    return 0;
}