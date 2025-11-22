#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_NAME_LEN 32
#define MAX_PASS_LEN 16
#define CURRENCY_NUM 5
#define MAX_USERS 100

struct User {
    char login[MAX_NAME_LEN];
    char password[MAX_PASS_LEN];
    double account[CURRENCY_NUM];
    double cash[CURRENCY_NUM];
};

static const char* DB_FILE = "Server.txt";

double get_rate(int from, int to) {
    static const double r[CURRENCY_NUM][CURRENCY_NUM] = {
        {1.0, 0.011, 0.01, 5.0, 1.7},
        {93.0, 1.0, 0.91, 470.0, 156.0},
        {101.0, 1.1, 1.0, 515.0, 170.0},
        {0.2, 0.0021, 0.0019, 1.0, 0.33},
        {0.6, 0.0064, 0.0058, 3.0, 1.0}
    };
    return r[from][to];
}

int read_users(User a[], int lim) {
    FILE* f = fopen(DB_FILE, "r");
    if (!f) return 0;
    int i = 0;
    while (i < lim) {
        User u{};
        int n = fscanf(f, "%31s %15s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            u.login, u.password,
            &u.account[0], &u.account[1], &u.account[2], &u.account[3], &u.account[4],
            &u.cash[0], &u.cash[1], &u.cash[2], &u.cash[3], &u.cash[4]);
        if (n != 12) break;
        a[i++] = u;
    }
    fclose(f);
    return i;
}

int write_users(User a[], int cnt) {
    FILE* f = fopen(DB_FILE, "w");
    if (!f) return 0;
    for (int i = 0; i < cnt; ++i) {
        fprintf(f, "%s %s %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
            a[i].login, a[i].password,
            a[i].account[0], a[i].account[1], a[i].account[2], a[i].account[3], a[i].account[4],
            a[i].cash[0], a[i].cash[1], a[i].cash[2], a[i].cash[3], a[i].cash[4]);
    }
    fclose(f);
    return 1;
}

int find_user(User a[], int cnt, const char* login) {
    for (int i = 0; i < cnt; ++i) if (strcmp(a[i].login, login) == 0) return i;
    return -1;
}

int do_deposit(User* u, int cur, double sum) {
    if (cur < 0 || cur >= CURRENCY_NUM || sum < 0) return 0;
    if (u->cash[cur] + 1e-9 < sum) return -1;
    u->account[cur] += sum;
    u->cash[cur] -= sum;
    return 1;
}

int do_withdraw(User* u, int cur, double sum) {
    if (cur < 0 || cur >= CURRENCY_NUM || sum < 0) return 0;
    if (u->account[cur] + 1e-9 < sum) return -1;
    u->account[cur] -= sum;
    u->cash[cur] += sum;
    return 1;
}

int do_transfer(User a[], int cnt, int s_idx, const char* to, int cur, double sum) {
    if (cur < 0 || cur >= CURRENCY_NUM || sum < 0) return 0;
    int r = find_user(a, cnt, to);
    if (r < 0) return -2;
    if (a[s_idx].account[cur] + 1e-9 < sum) return -1;
    a[s_idx].account[cur] -= sum;
    a[r].account[cur] += sum;
    return 1;
}

int do_exchange(User* u, int from, int to, double sum) {
    if (from < 0 || from >= CURRENCY_NUM || to < 0 || to >= CURRENCY_NUM || from == to || sum < 0) return 0;
    if (u->account[from] + 1e-9 < sum) return -1;
    double got = sum * get_rate(from, to);
    u->account[from] -= sum;
    u->account[to] += got;
    return 1;
}

static int send_all(SOCKET s, const char* d, int n) {
    int sent = 0;
    while (sent < n) {
        int k = send(s, d + sent, n - sent, 0);
        if (k == SOCKET_ERROR) return SOCKET_ERROR;
        sent += k;
    }
    return sent;
}

static void trim_newline(char* b) {
    int n = (int)strlen(b);
    while (n > 0 && (b[n - 1] == '\n' || b[n - 1] == '\r')) b[--n] = 0;
}

static int split(char* s, char* t[], int m) {
    int n = 0;
    char* ctx = 0;
    for (char* p = strtok_s(s, " \t", &ctx); p && n < m; p = strtok_s(NULL, " \t", &ctx)) t[n++] = p;
    return n;
}

static void render(User a[], int cnt, int cur_idx, const char* last) {
    system("cls");
    printf("SERVER MENU\n");
    printf("Users: %d  Current user: %s\n", cnt, cur_idx >= 0 ? a[cur_idx].login : "-");
    if (last && *last) printf("Last operation: %s\n", last);
    printf("------------------------------\n");
    printf("%-12s %-10s %-10s %-10s %-10s %-10s\n", "Login", "RUB", "USD", "EUR", "KZT", "JPY");
    for (int i = 0; i < cnt; ++i) {
        printf("%-12s %10.2f %10.2f %10.2f %10.2f %10.2f\n",
            a[i].login,
            a[i].account[0], a[i].account[1], a[i].account[2], a[i].account[3], a[i].account[4]);
    }
    printf("------------------------------\n");
}

int main() {
    const char IP[] = "127.0.0.1";
    const int PORT = 1234;
    User us[MAX_USERS]{};
    int ucnt = read_users(us, MAX_USERS);

    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) return 1;

    SOCKET ss = socket(AF_INET, SOCK_STREAM, 0);
    if (ss == INVALID_SOCKET) { WSACleanup(); return 1; }

    sockaddr_in si{};
    si.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &si.sin_addr);
    si.sin_port = htons(PORT);

    if (bind(ss, (sockaddr*)&si, sizeof(si)) != 0) { closesocket(ss); WSACleanup(); return 1; }
    if (listen(ss, SOMAXCONN) != 0) { closesocket(ss); WSACleanup(); return 1; }

    sockaddr_in ci{};
    int cis = sizeof(ci);
    SOCKET cs = accept(ss, (sockaddr*)&ci, &cis);
    if (cs == INVALID_SOCKET) { closesocket(ss); WSACleanup(); return 1; }

    int cur_idx = -1;
    char rb[2048], sb[4096], last[256];
    last[0] = 0;
    render(us, ucnt, cur_idx, last);

    for (;;) {
        int k = recv(cs, rb, (int)sizeof(rb) - 1, 0);
        if (k == 0 || k == SOCKET_ERROR) break;
        rb[k] = 0;
        trim_newline(rb);

        char* tok[16]{};
        char tmp[2048]; strncpy(tmp, rb, sizeof(tmp)); tmp[sizeof(tmp) - 1] = 0;
        int n = split(tmp, tok, 16);
        if (n == 0) { const char* m = "ERR Empty\n"; send_all(cs, m, (int)strlen(m)); continue; }

        if (strcmp(tok[0], "HELP") == 0) {
            const char* m =
                "COMMANDS:\nREGISTER login pass c0 c1 c2 c3 c4\nLOGIN login pass\nBALANCE\nDEPOSIT cur sum\nWITHDRAW cur sum\nTRANSFER to cur sum\nEXCHANGE from to sum\nLOGOUT\nQUIT\n";
            send_all(cs, m, (int)strlen(m));
            continue;
        }

        if (strcmp(tok[0], "QUIT") == 0) break;

        if (strcmp(tok[0], "REGISTER") == 0) {
            if (n != 8) { const char* m = "ERR Args\n"; send_all(cs, m, (int)strlen(m)); continue; }
            if (ucnt >= MAX_USERS) { const char* m = "ERR Full\n"; send_all(cs, m, (int)strlen(m)); continue; }
            if (find_user(us, ucnt, tok[1]) != -1) { const char* m = "ERR Exists\n"; send_all(cs, m, (int)strlen(m)); continue; }
            User u{};
            strncpy(u.login, tok[1], MAX_NAME_LEN - 1);
            strncpy(u.password, tok[2], MAX_PASS_LEN - 1);
            for (int i = 0; i < CURRENCY_NUM; ++i) {
                u.account[i] = 0.0;
                u.cash[i] = strtod(tok[3 + i], NULL);
                if (u.cash[i] < 0) u.cash[i] = 0;
            }
            us[ucnt++] = u;
            write_users(us, ucnt);
            const char* m = "OK Registered\n";
            send_all(cs, m, (int)strlen(m));
            snprintf(last, sizeof(last), "REGISTER %s", u.login);
            render(us, ucnt, cur_idx, last);
            continue;
        }

        if (strcmp(tok[0], "LOGIN") == 0) {
            if (n != 3) { const char* m = "ERR Args\n"; send_all(cs, m, (int)strlen(m)); continue; }
            int i = find_user(us, ucnt, tok[1]);
            if (i == -1 || strcmp(us[i].password, tok[2]) != 0) { const char* m = "ERR Auth\n"; send_all(cs, m, (int)strlen(m)); continue; }
            cur_idx = i;
            const char* m = "OK Logged\n";
            send_all(cs, m, (int)strlen(m));
            snprintf(last, sizeof(last), "LOGIN %s", us[cur_idx].login);
            render(us, ucnt, cur_idx, last);
            continue;
        }

        if (strcmp(tok[0], "LOGOUT") == 0) {
            if (cur_idx >= 0) snprintf(last, sizeof(last), "LOGOUT %s", us[cur_idx].login);
            else snprintf(last, sizeof(last), "LOGOUT -");
            cur_idx = -1;
            const char* m = "OK LoggedOut\n";
            send_all(cs, m, (int)strlen(m));
            render(us, ucnt, cur_idx, last);
            continue;
        }

        if (cur_idx == -1) {
            const char* m = "ERR LoginFirst\n";
            send_all(cs, m, (int)strlen(m));
            continue;
        }

        if (strcmp(tok[0], "BALANCE") == 0) {
            int len = snprintf(sb, sizeof(sb),
                "BALANCE [%s]\nRUB: %.2f (cash: %.2f)\nUSD: %.2f (cash: %.2f)\nEUR: %.2f (cash: %.2f)\nKZT: %.2f (cash: %.2f)\nJPY: %.2f (cash: %.2f)\n",
                us[cur_idx].login,
                us[cur_idx].account[0], us[cur_idx].cash[0],
                us[cur_idx].account[1], us[cur_idx].cash[1],
                us[cur_idx].account[2], us[cur_idx].cash[2],
                us[cur_idx].account[3], us[cur_idx].cash[3],
                us[cur_idx].account[4], us[cur_idx].cash[4]);
            send_all(cs, sb, len);
            snprintf(last, sizeof(last), "BALANCE %s", us[cur_idx].login);
            render(us, ucnt, cur_idx, last);
            continue;
        }

        if (strcmp(tok[0], "DEPOSIT") == 0) {
            if (n != 3) { const char* m = "ERR Args\n"; send_all(cs, m, (int)strlen(m)); continue; }
            int c = atoi(tok[1]); double s = strtod(tok[2], NULL);
            int r = do_deposit(&us[cur_idx], c, s);
            if (r == 1) {
                write_users(us, ucnt);
                const char* m = "OK\n"; send_all(cs, m, (int)strlen(m));
                snprintf(last, sizeof(last), "DEPOSIT %s cur=%d sum=%.2f", us[cur_idx].login, c, s);
                render(us, ucnt, cur_idx, last);
            }
            else if (r == -1) send_all(cs, "ERR NoCash\n", 10);
            else send_all(cs, "ERR Args\n", 9);
            continue;
        }

        if (strcmp(tok[0], "WITHDRAW") == 0) {
            if (n != 3) { const char* m = "ERR Args\n"; send_all(cs, m, (int)strlen(m)); continue; }
            int c = atoi(tok[1]); double s = strtod(tok[2], NULL);
            int r = do_withdraw(&us[cur_idx], c, s);
            if (r == 1) {
                write_users(us, ucnt);
                const char* m = "OK\n"; send_all(cs, m, (int)strlen(m));
                snprintf(last, sizeof(last), "WITHDRAW %s cur=%d sum=%.2f", us[cur_idx].login, c, s);
                render(us, ucnt, cur_idx, last);
            }
            else if (r == -1) send_all(cs, "ERR NoFunds\n", 11);
            else send_all(cs, "ERR Args\n", 9);
            continue;
        }

        if (strcmp(tok[0], "TRANSFER") == 0) {
            if (n != 4) { const char* m = "ERR Args\n"; send_all(cs, m, (int)strlen(m)); continue; }
            const char* to = tok[1]; int c = atoi(tok[2]); double s = strtod(tok[3], NULL);
            int r = do_transfer(us, ucnt, cur_idx, to, c, s);
            if (r == 1) {
                write_users(us, ucnt);
                const char* m = "OK\n"; send_all(cs, m, (int)strlen(m));
                snprintf(last, sizeof(last), "TRANSFER %s->%s cur=%d sum=%.2f", us[cur_idx].login, to, c, s);
                render(us, ucnt, cur_idx, last);
            }
            else if (r == -1) send_all(cs, "ERR NoFunds\n", 11);
            else if (r == -2) send_all(cs, "ERR NoUser\n", 10);
            else send_all(cs, "ERR Args\n", 9);
            continue;
        }

        if (strcmp(tok[0], "EXCHANGE") == 0) {
            if (n != 4) { const char* m = "ERR Args\n"; send_all(cs, m, (int)strlen(m)); continue; }
            int f = atoi(tok[1]); int t = atoi(tok[2]); double s = strtod(tok[3], NULL);
            int r = do_exchange(&us[cur_idx], f, t, s);
            if (r == 1) {
                write_users(us, ucnt);
                const char* m = "OK\n"; send_all(cs, m, (int)strlen(m));
                snprintf(last, sizeof(last), "EXCHANGE %s %d->%d sum=%.2f", us[cur_idx].login, f, t, s);
                render(us, ucnt, cur_idx, last);
            }
            else if (r == -1) send_all(cs, "ERR NoFunds\n", 11);
            else send_all(cs, "ERR Args\n", 9);
            continue;
        }

        send_all(cs, "ERR Unknown\n", 12);
    }

    shutdown(cs, SD_BOTH);
    closesocket(cs);
    closesocket(ss);
    WSACleanup();
    return 0;
}