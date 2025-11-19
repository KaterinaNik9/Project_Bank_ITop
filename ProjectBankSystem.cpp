#define _CRT_SECURE_NO_WARNINGS
#include "user.h"
#include <stdio.h>
#include <locale.h>

// Главное меню
int main() {
    setlocale(LC_ALL, "Russian");
    User users[MAX_USERS];
    int user_cnt = read_users(users, MAX_USERS);

    int active = -1;
    char curr_login[MAX_NAME_LEN];

    while (1) {
        printf("\n=== Добро пожаловать! ===\n");
        printf("1. Вход\n2. Регистрация\n0. Выход\nВаш выбор: ");
        int choice;
        scanf("%d", &choice);

        if (choice == 1) {
            active = login_menu(users, user_cnt, curr_login);
            if (active != -1) {
                // пользовательское меню
                while (1) {
                    printf("\nМеню: 1-Баланс 2-Пополнить 3-Снять 4-Перевод 5-Обмен 0-Выйти\n>");
                    int act;
                    scanf("%d", &act);
                    if (act == 1) show_balance(&users[active]);
                    else if (act == 2) { do_deposit(&users[active]); write_users(users, user_cnt); }
                    else if (act == 3) { take_withdraw(&users[active]); write_users(users, user_cnt); }
                    else if (act == 4) { do_transfer(users, user_cnt, active); write_users(users, user_cnt); }
                    else if (act == 5) { exchange(&users[active]); write_users(users, user_cnt); }
                    else if (act == 0) break;
                    else printf("Что-то не то выбрано\n");
                }
            }
        }
        else if (choice == 2) {
            int i = register_menu(users, user_cnt, curr_login);
            if (i != -1) user_cnt++;
        }
        else if (choice == 0)
            break;
        else
            printf("Некорректный ввод!\n");
    }
    return 0;
}
