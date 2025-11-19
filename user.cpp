#define _CRT_SECURE_NO_WARNINGS
#include "user.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Таблица курсов (статично)
double get_rate(int from, int to) {
    double rates[CURRENCY_NUM][CURRENCY_NUM] = {
        {1.0, 0.011, 0.01, 5.0, 1.7},         // RUB
        {93.0, 1.0, 0.91, 470.0, 156.0},      // USD
        {101.0,1.1, 1.0, 515.0, 170.0},       // EUR
        {0.2, 0.0021,0.0019,1.0, 0.33},       // KZT
        {0.6, 0.0064,0.0058,3.0, 1.0}         // JPY
    };
    return rates[from][to];
}

// Чтение из файла - все пользователи
int read_users(User arr[], int lim) {
    FILE* f = fopen("Server.txt", "r");
    if (!f) return 0;
    int idx = 0;
    while (idx < lim &&
        fscanf(f, "%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            arr[idx].login,
            arr[idx].password,
            &arr[idx].account[0],
            &arr[idx].account[1],
            &arr[idx].account[2],
            &arr[idx].account[3],
            &arr[idx].account[4],
            &arr[idx].cash[0],
            &arr[idx].cash[1],
            &arr[idx].cash[2],
            &arr[idx].cash[3],
            &arr[idx].cash[4]) == 12)
    {
        idx++;
    }
    fclose(f);
    return idx;
}

// Запись обратно в файл
int write_users(User arr[], int cnt) {
    FILE* f = fopen("Server.txt", "w");
    if (!f) return 0;
    for (int i = 0; i < cnt; ++i) {
        fprintf(f, "%s %s %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf\n",
            arr[i].login, arr[i].password,
            arr[i].account[0], arr[i].account[1], arr[i].account[2], arr[i].account[3], arr[i].account[4],
            arr[i].cash[0], arr[i].cash[1], arr[i].cash[2], arr[i].cash[3], arr[i].cash[4]
        );
    }
    fclose(f);
    return 1;
}

// Поиск юзера по логину
int find_user(User arr[], int cnt, const char* login) {
    for (int i = 0; i < cnt; ++i) {
        if (strcmp(arr[i].login, login) == 0)
            return i;
    }
    return -1;
}

// Меню входа
int login_menu(User arr[], int cnt, char* login_out) {
    char login[MAX_NAME_LEN], pass[MAX_PASS_LEN];
    printf("Логин: ");
    scanf("%s", login);
    printf("Пароль: ");
    scanf("%s", pass);

    if (strlen(pass) > MAX_PASS_LEN - 1) {
        printf("Слишком длинный пароль!\n");
        return -1;
    }
    int idx = find_user(arr, cnt, login);
    if (idx != -1 && strcmp(arr[idx].password, pass) == 0) {
        strcpy(login_out, login);
        printf("\nВошли как %s\n", login);
        return idx;
    }
    printf("\nПровал входа!\n");
    return -1;
}

// Регистрация нового пользователя
int register_menu(User arr[], int cnt, char* login_new) {
    char login[MAX_NAME_LEN], pass[MAX_PASS_LEN];
    printf("\nПридумайте логин (до 31): ");
    scanf("%s", login);

    if (find_user(arr, cnt, login) != -1) {
        printf("Пользователь с таким логином уже есть!\n");
        return -1;
    }
    printf("Пароль (до 15 символов): ");
    scanf("%s", pass);

    if (strlen(pass) > MAX_PASS_LEN - 1) {
        printf("Пароль длиннее чем положено!\n");
        return -1;
    }

    strcpy(arr[cnt].login, login);
    strcpy(arr[cnt].password, pass);

    for (int i = 0; i < CURRENCY_NUM; ++i) {
        arr[cnt].account[i] = 0;
        printf("Стартовый кэш (%s): ",
            i == 0 ? "RUB" : i == 1 ? "USD" : i == 2 ? "EUR" : i == 3 ? "KZT" : "JPY");
        double num;
        scanf("%lf", &num);
        if (num < 0) num = 0;
        arr[cnt].cash[i] = num;
    }
    write_users(arr, cnt + 1);
    strcpy(login_new, login);
    printf("Регистрация завершена!\n");
    return cnt;
}

// Баланс/остатки пользователя
void show_balance(const User* u) {
    printf("Баланс [%s]:\n", u->login);
    printf("RUB:  %.2lf   (нал: %.2lf)\n", u->account[0], u->cash[0]);
    printf("USD:  %.2lf   (нал: %.2lf)\n", u->account[1], u->cash[1]);
    printf("EUR:  %.2lf   (нал: %.2lf)\n", u->account[2], u->cash[2]);
    printf("KZT:  %.2lf   (нал: %.2lf)\n", u->account[3], u->cash[3]);
    printf("JPY:  %.2lf   (нал: %.2lf)\n", u->account[4], u->cash[4]);
    printf("-------------------------------\n");
}

// Пополнение счета с налички
void do_deposit(User* u) {
    int currency;
    double sum;
    printf("Какую валюту кладете (0-RUB 1-USD 2-EUR 3-KZT 4-JPY)? ");
    scanf("%d", &currency);
    if (currency < 0 || currency >= CURRENCY_NUM) {
        printf("Ошибка выбора!\n");
        return;
    }
    printf("Сколько кладете: ");
    scanf("%lf", &sum);
    if (sum < 0) { printf("Ошибка, отрицательное число!\n"); return; }
    if (u->cash[currency] < sum) {
        printf("Столько на руках нет! Наличные: %.2lf\n", u->cash[currency]);
        return;
    }
    u->account[currency] += sum;
    u->cash[currency] -= sum;
    printf("Готово! Осталось налички: %.2lf\n", u->cash[currency]);
}

// Снятие средств (счёт -> кэш)
void take_withdraw(User* u) {
    int currency;
    double sum;
    printf("Выбирайте валюту для снятия (0-RUB 1-USD 2-EUR 3-KZT 4-JPY): ");
    scanf("%d", &currency);
    if (currency < 0 || currency >= CURRENCY_NUM) { printf("Ошибка\n"); return; }
    printf("Сколько снимаем: ");
    scanf("%lf", &sum);
    if (sum < 0 || u->account[currency] < sum) {
        printf("Нет столько на счету!\n"); return;
    }
    u->account[currency] -= sum;
    u->cash[currency] += sum;
    printf("Деньги сняты, теперь у вас кэша: %.2lf\n", u->cash[currency]);
}

// Перевести деньги другому (между счетами, без кэша)
void do_transfer(User arr[], int cnt, int sender_idx) {
    char receiver[MAX_NAME_LEN];
    int currency;
    double sum;
    printf("Кому перевести (логин): ");
    scanf("%s", receiver);
    printf("Валюта? (0-RUB 1-USD 2-EUR 3-KZT 4-JPY): ");
    scanf("%d", &currency);
    printf("Сколько: ");
    scanf("%lf", &sum);

    if (currency < 0 || currency >= CURRENCY_NUM || sum < 0) {
        printf("Ошибка!\n");
        return;
    }
    if (arr[sender_idx].account[currency] < sum) {
        printf("Нет денег на счету\n"); return;
    }
    int r_idx = find_user(arr, cnt, receiver);
    if (r_idx == -1) {
        printf("Нет такого пользователя!\n");
        return;
    }
    arr[sender_idx].account[currency] -= sum;
    arr[r_idx].account[currency] += sum;
    printf("Перевод сделан!\n");
}

// Обмен валют (только между своими счетами)
void exchange(User* u) {
    int from, to;
    double sum;
    printf("Из (0-RUB 1-USD 2-EUR 3-KZT 4-JPY): ");
    scanf("%d", &from);
    printf("В (0-RUB 1-USD 2-EUR 3-KZT 4-JPY): ");
    scanf("%d", &to);
    if (from < 0 || from >= CURRENCY_NUM || to < 0 || to >= CURRENCY_NUM || from == to) {
        printf("Ошибка!\n");
        return;
    }
    printf("Сумма обмена: ");
    scanf("%lf", &sum);
    if (sum < 0 || u->account[from] < sum) {
        printf("Недостаточно средств или неверно!\n");
        return;
    }
    double got = sum * get_rate(from, to);
    u->account[from] -= sum;
    u->account[to] += got;
    printf("Обмен совершен: %.2lf --> %.2lf (новое значение)\n", sum, got);
}
