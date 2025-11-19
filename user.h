#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define MAX_USERS 100
#define MAX_NAME_LEN 32
#define MAX_PASS_LEN 16
#define CURRENCY_NUM 5

typedef struct User {
    char login[MAX_NAME_LEN];
    char password[MAX_PASS_LEN];
    double account[CURRENCY_NUM]; // 0-RUB, 1-USD, 2-EUR, 3-KZT, 4-JPY
    double cash[CURRENCY_NUM];    // Наличные на руках (та же логика)
} User;

int read_users(User arr[], int lim);
int write_users(User arr[], int cnt);

int find_user(User arr[], int cnt, const char* login);
int login_menu(User arr[], int cnt, char* login_out);
int register_menu(User arr[], int cnt, char* login_new);

void show_balance(const User* u);
void do_deposit(User* u);
void take_withdraw(User* u);
void do_transfer(User arr[], int cnt, int sender_idx);
void exchange(User* u);
