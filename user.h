#pragma once
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>

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

double get_rate(int from, int to);
int read_users(User a[], int lim);
int write_users(User a[], int cnt);
int find_user(User a[], int cnt, const char* login);

void balance_to_string(const User* u, char* out, size_t outsz);

int do_deposit(User* u, int cur, double sum);
int do_withdraw(User* u, int cur, double sum);
int do_transfer(User a[], int cnt, int s_idx, const char* to, int cur, double sum);
int do_exchange(User* u, int from, int to, double sum);