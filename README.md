#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

using namespace std;

// Структура для хранения данных счета
struct Account {
    string cardNumber;
    string pin;
    double balance;
    string ownerName;
};

// Глобальные переменные (вместо классов)
Account currentAccount;
bool isLoggedIn = false;

// Функции АТМ
void initializeATM() {
    // Инициализация тестового счета
    currentAccount.cardNumber = "1234-5678-9012-3456";
    currentAccount.pin = "1234";
    currentAccount.balance = 10000.0;
    currentAccount.ownerName = "Иван Иванов";
}

void showWelcomeScreen() {
    cout << "=================================\n";
    cout << "         ДОБРО ПОЖАЛОВАТЬ       \n";
    cout << "             В БАНКОМАТ         \n";
    cout << "=================================\n";
}

void showMainMenu() {
    cout << "\n======== ОСНОВНОЕ МЕНЮ ========\n";
    cout << "1. Проверить баланс\n";
    cout << "2. Снять наличные\n";
    cout << "3. Внести наличные\n";
    cout << "4. Перевод средств\n";
    cout << "5. Сменить PIN-код\n";
    cout << "6. Информация о счете\n";
    cout << "7. Выйти\n";
    cout << "Выберите операцию: ";
}

bool authenticateUser() {
    string enteredCard, enteredPin;
    
    cout << "\n=== АВТОРИЗАЦИЯ ===\n";
    cout << "Введите номер карты: ";
    cin >> enteredCard;
    cout << "Введите PIN-код: ";
    cin >> enteredPin;
    
    if (enteredCard == currentAccount.cardNumber && enteredPin == currentAccount.pin) {
        cout << "Авторизация успешна! Добро пожаловать, " << currentAccount.ownerName << "!\n";
        return true;
    } else {
        cout << "Ошибка: Неверный номер карты или PIN-код!\n";
        return false;
        void checkBalance() {
    cout << "\n=== БАЛАНС СЧЕТА ===\n";
    cout << "Владелец: " << currentAccount.ownerName << endl;
    cout << "Номер карты: " << currentAccount.cardNumber << endl;
    cout << "Текущий баланс: " << fixed << setprecision(2) << currentAccount.balance << " руб.\n";
}

void withdrawCash() {
    double amount;
    
    cout << "\n=== СНЯТИЕ НАЛИЧНЫХ ===\n";
    cout << "Доступные суммы: 100, 500, 1000, 2000, 5000\n";
    cout << "Введите сумму для снятия: ";
    cin >> amount;
    
    // Проверка ввода
    if (cin.fail() || amount <= 0) {
        cout << "Ошибка: Неверная сумма!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    // Проверка доступных сумм
    if (amount != 100 && amount != 500 && amount != 1000 && amount != 2000 && amount != 5000) {
        cout << "Ошибка: Неверная сумма! Выберите из доступных вариантов.\n";
        return;
    }
    
    // Проверка баланса
    if (amount > currentAccount.balance) {
        cout << "Ошибка: Недостаточно средств на счете!\n";
        cout << "Запрошено: " << amount << " руб.\n";
        cout << "Доступно: " << currentAccount.balance << " руб.\n";
        return;
    }
    
    // Выполнение операции
    currentAccount.balance -= amount;
    cout << "Операция успешна!\n";
    cout << "Снято: " << amount << " руб.\n";
    cout << "Остаток на счете: " << currentAccount.balance << " руб.\n";
}

void depositCash() {
    double amount;
    
    cout << "\n=== ВНЕСЕНИЕ НАЛИЧНЫХ ===\n";
    cout << "Введите сумму для внесения: ";
    cin >> amount;
    
    // Проверка ввода
    if (cin.fail() || amount <= 0) {
        cout << "Ошибка: Неверная сумма!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    // Проверка максимальной суммы
    if (amount > 50000) {
        cout << "Ошибка: Превышена максимальная сумма для внесения (50,000 руб.)!\n";
        return;
    }
    
    // Выполнение операции
    currentAccount.balance += amount;
    cout << "Операция успешна!\n";
    cout << "Внесено: " << amount << " руб.\n";
    cout << "Текущий баланс: " << currentAccount.balance << " руб.\n";
}

void transferMoney() {
    string recipientCard;
    double amount;
    
    cout << "\n=== ПЕРЕВОД СРЕДСТВ ===\n";
    cout << "Введите номер карты получателя: ";
    cin >> recipientCard;
    cout << "Введите сумму перевода: ";
    cin >> amount;
    
    // Проверка ввода
    if (cin.fail() || amount <= 0) {
        cout << "Ошибка: Неверная сумма!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    // Проверка баланса
    if (amount > currentAccount.balance) {
        cout << "Ошибка: Недостаточно средств для перевода!\n";
        return;
    }
    
    // Проверка номера карты
    if (recipientCard.length() != 16 && recipientCard.length() != 19) {
        cout << "Ошибка: Неверный формат номера карты!\n";
        return;
    }
    
    // Подтверждение операции
    cout << "\nПодтвердите перевод:\n";
    cout << "Получатель: Карта " << recipientCard << endl;
    cout << "Сумма: " << amount << " руб.\n";
    cout << "Комиссия: 0 руб.\n";
    cout << "Подтвердить? (1 - Да, 0 - Нет): ";
    
    int confirm;
    cin >> confirm;
    
    if (confirm == 1) {
        currentAccount.balance -= amount;
        cout << "Перевод выполнен успешно!\n";
        cout << "Переведено: " << amount << " руб. на карту " << recipientCard << endl;
        cout << "Остаток на счете: " << currentAccount.balance << " руб.\n";
    } else {
        cout << "Перевод отменен.\n";
    }
}

void changePIN() {
    string currentPIN, newPIN, confirmPIN;
    
    cout << "\n=== СМЕНА PIN-КОДА ===\n";
    cout << "Введите текущий PIN-код: ";
    cin >> currentPIN;
    
    // Проверка текущего PIN
    if (currentPIN != currentAccount.pin) {
        cout << "Ошибка: Неверный текущий PIN-код!\n";
        return;
    }
    
    cout << "Введите новый PIN-код (4 цифры): ";
    cin >> newPIN;
    
    // Проверка нового PIN
    if (newPIN.length() != 4) {
        cout << "Ошибка: PIN-код должен состоять из 4 цифр!\n";
        return;
    }
    
    // Проверка, что PIN состоит только из цифр
    for (char c : newPIN) {
        if (!isdigit(c)) {
            cout << "Ошибка: PIN-код должен содержать только цифры!\n";
            return;
        }
    }
    
    cout << "Подтвердите новый PIN-код: ";
    cin >> confirmPIN;
    
    if (newPIN == confirmPIN) {
        currentAccount.pin = newPIN;
        cout << "PIN-код успешно изменен!\n";
    } else {
        cout << "Ошибка: PIN-коды не совпадают!\n";
    }
}

void showAccountInfo() {
    cout << "\n=== ИНФОРМАЦИЯ О СЧЕТЕ ===\n";
    cout << "Владелец: " << currentAccount.ownerName << endl;
    cout << "Номер карты: " << currentAccount.cardNumber << endl;
    cout << "Тип счета: Расчетный\n";
    cout << "Валюта: RUB\n";
    cout << "Дата открытия: 01.01.2023\n";
}

void processOperation(int choice) {
    switch (choice) {
        case 1:
            checkBalance();
            break;
        case 2:
            withdrawCash();
            break;
        case 3:
            depositCash();
            break;
        case 4:
            transferMoney();
            break;
        case 5:
            changePIN();
            break;
        case 6:
            showAccountInfo();
            break;
        case 7:
            cout << "\nСпасибо за использование нашего банкомата!\n";
            cout << "Не забудьте забрать карту!\n";
            isLoggedIn = false;
            break;
        default:
            cout << "Ошибка: Неверный выбор операции!\n";
            break;
    }
}

int main() {
    // Установка локализации для русского языка
    setlocale(LC_ALL, "Russian");
    
    initializeATM();
    
    while (true) {
        showWelcomeScreen();
        
        // Авторизация
        if (!authenticateUser()) {
            cout << "Пожалуйста, попробуйте еще раз.\n";
            continue;
        }
        
        isLoggedIn = true;
        
        // Основной цикл операций
        while (isLoggedIn) {
            showMainMenu();
            
            int choice;
            cin >> choice;
            
            // Проверка корректности ввода
            if (cin.fail()) {
                cout << "Ошибка: Введите число от 1 до 7!\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            
            processOperation(choice);
            
            // Пауза между операциями
            if (isLoggedIn && choice != 7) {
                cout << "\nНажмите Enter для продолжения...";
                cin.ignore();
                cin.get();
            }
        }
        
        // Запрос на продолжение работы
        cout << "\nХотите выполнить еще операции? (1 - Да, 0 - Нет): ";
        int continueOperation;
        cin >> continueOperation;
        
        if (continueOperation != 1) {
            cout << "До свидания! Хорошего дня!\n";
            break;
        }
    }
    
    return 0;
}
    }
}
 
