#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdlib>

using namespace std;

class User
{
public:
    string username;
    string password;

    User(const string &uname, const string &pwd)
        : username(uname), password(pwd) {}

    void setPassword(const string &oldPwd, const string &newPwd)
    {
        if (oldPwd == password)
        {
            password = newPwd;
            cout << "Password changed successfully." << endl;
        }
        else
        {
            cout << "Incorrect old password. Password not changed." << endl;
        }
    }
};

class Transaction
{
public:
    string category;
    double amount;
    string timestamp;

    Transaction(const string &cat, double amt, const string &time)
        : category(cat), amount(amt), timestamp(time) {}
};

class Account
{
protected:
    string accountNumber;
    double balance;
    vector<Transaction> transactionHistory;
    User *user;

public:
    virtual ~Account() = default;

    Account(const string &accNumber, double initialBalance, User *accUser)
        : accountNumber(accNumber), balance(initialBalance), user(accUser) {}

    virtual void deposit(double amount)
    {
        balance += amount;
        recordTransaction("Deposit", amount);
    }

    virtual void withdraw(double amount)
    {
        string enteredPassword;
        cout << "Enter your password to withdraw: ";
        cin >> enteredPassword;

        if (enteredPassword != user->password)
        {
            cout << "Incorrect password. Withdrawal canceled." << endl;
            return;
        }

        if (balance >= amount)
        {
            balance -= amount;
            recordTransaction("Withdrawal", amount);
            cout << "Withdrawal successful. Remaining balance: $" << balance << endl;
        }
        else
        {
            cout << "Insufficient balance." << endl;
        }
    }

    virtual void display() const
    {
        cout << "Account Number: " << accountNumber << endl;
        cout << "Balance: $" << balance << endl;
    }

    virtual string getAccountNumber() const
    {
        return accountNumber;
    }

    virtual void showTransactionHistory() const
    {
        cout << "Transaction History for Account: " << accountNumber << endl;
        for (const auto &transaction : transactionHistory)
        {
            cout << transaction.timestamp << " - " << transaction.category << ": $" << transaction.amount << endl;
        }
    }

    virtual void deleteAccount()
    {
        cout << "Account deleted successfully." << endl;
    }

    virtual void transfer(Account *targetAccount, double amount)
    {
        string enteredPassword;
        cout << "Enter your password to transfer funds: ";
        cin >> enteredPassword;

        if (enteredPassword != user->password)
        {
            cout << "Incorrect password. Transfer canceled." << endl;
            return;
        }

        if (balance >= amount)
        {
            balance -= amount;
            targetAccount->deposit(amount);
            recordTransaction("Transfer (To: " + targetAccount->getAccountNumber() + ")", amount);
            cout << "Transfer successful. Remaining balance: $" << balance << endl;
        }
        else
        {
            cout << "Insufficient balance for transfer." << endl;
        }
    }

protected:
    void recordTransaction(const string &type, double amount)
    {
        chrono::system_clock::time_point now = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(now);

        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&tt));

        string transactionRecord = timestamp;
        transactionRecord += " - " + type + ": $" + to_string(amount);
        transactionHistory.push_back(Transaction(type, amount, transactionRecord));
    }
};

class SavingsAccount : public Account
{
public:
    SavingsAccount(const string &accNumber, double initialBalance, User *accUser)
        : Account(accNumber, initialBalance, accUser) {}

    void applyInterest()
    {
        // No interest in SavingsAccount
    }
};

class CurrentAccount : public Account
{
private:
    double overdraftLimit;

public:
    CurrentAccount(const string &accNumber, double initialBalance, double overdraft, User *accUser)
        : Account(accNumber, initialBalance, accUser), overdraftLimit(overdraft) {}

    void withdraw(double amount) override
    {
        string enteredPassword;
        cout << "Enter your password to withdraw: ";
        cin >> enteredPassword;

        if (enteredPassword != user->password)
        {
            cout << "Incorrect password. Withdrawal canceled." << endl;
            return;
        }

        if (balance + overdraftLimit >= amount)
        {
            balance -= amount;
            recordTransaction("Withdrawal", amount);
            cout << "Withdrawal successful. Remaining balance: $" << balance << endl;
        }
        else
        {
            recordTransaction("Transaction Declined - Overdraft Limit Exceeded", 0.0);
            cout << "Transaction declined. Overdraft limit exceeded." << endl;
        }
    }

    void deleteAccount() override
    {
        // Additional cleanup or specific actions for CurrentAccount deletion
        cout << "Current account deleted successfully." << endl;
    }
};

class BankingSystem
{
private:
    unordered_map<string, User> users;
    vector<Account *> accounts;

public:
    const vector<Account *> &getAccounts() const
    {
        return accounts;
    }

    void createUser(const string &username, const string &password)
    {
        users.emplace(username, User(username, password));
    }

    bool authenticateUser(const string &username, const string &password)
    {
        auto it = users.find(username);
        return (it != users.end() && it->second.password == password);
    }

    void displayMenu()
    {
        cout << "Menu:" << endl;
        cout << "1. Create Account" << endl;
        cout << "2. Deposit Funds" << endl;
        cout << "3. Withdraw Funds" << endl;
        cout << "4. Display Account Details" << endl;
        cout << "5. Show Transaction History" << endl;
        cout << "6. Change Password" << endl;
        cout << "7. Delete Account" << endl;
        cout << "8. Transfer Funds" << endl;
        cout << "9. Exit" << endl;
    }

    void executeChoice(int choice)
    {
        switch (choice)
        {
        case 1:
            createAccount();
            break;
        case 2:
            depositFunds();
            break;
        case 3:
            withdrawFunds();
            break;
        case 4:
            displayAccountDetails();
            break;
        case 5:
            showTransactionHistory();
            break;
        case 6:
            changePassword();
            break;
        case 7:
            deleteAccount();
            break;
        case 8:
            transferFunds();
            break;
        case 9:
            // Clean up memory
            for (auto &acc : accounts)
            {
                delete acc;
            }
            accounts.clear();
            cout << "Exiting the program." << endl;
            break;
        default:
            cout << "Invalid choice. Try again." << endl;
        }
    }

private:
    void createAccount()
    {
        string accNumber;
        double initialBalance;
        string accType;
        string username;
        string password;

        cin.ignore(); // Clear the input buffer

        cout << "Enter account number: ";
        getline(cin, accNumber);

        // Check if an account with the same account number already exists
        for (const auto &acc : accounts)
        {
            if (acc->getAccountNumber() == accNumber)
            {
                cout << "Account with this account number already exists." << endl;
                return;
            }
        }

        cout << "Enter initial balance: ";
        cin >> initialBalance;

        cin.ignore(); // Clear the input buffer

        cout << "Enter account type (Savings or Current): ";
        getline(cin, accType);

        cout << "Enter your username: ";
        getline(cin, username);

        cout << "Create a password for your account: ";
        getline(cin, password);

        createUser(username, password);

        User *newUser = &users.find(username)->second;

        if (accType == "Savings")
        {
            accounts.push_back(new SavingsAccount(accNumber, initialBalance, newUser));
            cout << "Savings account created successfully." << endl;
        }
        else if (accType == "Current")
        {
            accounts.push_back(new CurrentAccount(accNumber, initialBalance, 100.0, newUser));
            cout << "Current account created successfully." << endl;
        }
        else
        {
            cout << "Invalid account type." << endl;
        }
    }

    void depositFunds()
    {
        string accNumber;
        double depositAmount;

        cout << "Enter account number: ";
        cin.ignore(); // Ignore newline character
        getline(cin, accNumber);

        cout << "Enter deposit amount: ";
        cin >> depositAmount;

        Account *account = findAccountByNumber(accNumber);
        if (account)
        {
            account->deposit(depositAmount);
        }
        else
        {
            cout << "Account not found." << endl;
        }
    }

    void withdrawFunds()
    {
        string accNumber;
        double withdrawAmount;

        cout << "Enter account number: ";
        cin.ignore(); // Ignore newline character
        getline(cin, accNumber);

        Account *account = findAccountByNumber(accNumber);
        if (account)
        {
            cout << "Enter withdrawal amount: ";
            cin >> withdrawAmount;
            account->withdraw(withdrawAmount);
        }
        else
        {
            cout << "Account not found." << endl;
        }
    }

    void displayAccountDetails()
    {
        string accNumber;
        cout << "Enter account number: ";
        cin.ignore(); // Ignore newline character
        getline(cin, accNumber);

        Account *account = findAccountByNumber(accNumber);
        if (account)
        {
            account->display();
        }
        else
        {
            cout << "Account not found." << endl;
        }
    }

    void showTransactionHistory()
    {
        string accNumber;
        cout << "Enter account number: ";
        cin.ignore(); // Ignore newline character
        getline(cin, accNumber);

        Account *account = findAccountByNumber(accNumber);
        if (account)
        {
            account->showTransactionHistory();
        }
        else
        {
            cout << "Account not found." << endl;
        }
    }

    void changePassword()
    {
        string username;
        string oldPassword;
        string newPassword;

        cout << "Enter your username: ";
        cin >> username;

        auto it = users.find(username);
        if (it != users.end())
        {
            cout << "Enter your old password: ";
            cin >> oldPassword;
            it->second.setPassword(oldPassword, newPassword);
        }
        else
        {
            cout << "User not found." << endl;
        }
    }

    void deleteAccount()
    {
        string accNumber;
        cout << "Enter account number to delete: ";
        cin.ignore(); // Ignore newline character
        getline(cin, accNumber);

        for (auto it = accounts.begin(); it != accounts.end(); ++it)
        {
            if ((*it)->getAccountNumber() == accNumber)
            {
                (*it)->deleteAccount();
                delete *it;
                accounts.erase(it);
                cout << "Account deleted successfully." << endl;
                return;
            }
        }

        cout << "Account not found." << endl;
    }

    void transferFunds()
    {
        string sourceAccNumber;
        string targetAccNumber;
        double amount;

        cout << "Enter your account number: ";
        cin.ignore(); // Ignore newline character
        getline(cin, sourceAccNumber);

        cout << "Enter target account number: ";
        getline(cin, targetAccNumber);

        cout << "Enter transfer amount: ";
        cin >> amount;

        Account *sourceAccount = findAccountByNumber(sourceAccNumber);
        Account *targetAccount = findAccountByNumber(targetAccNumber);

        if (sourceAccount && targetAccount)
        {
            sourceAccount->transfer(targetAccount, amount);
        }
        else
        {
            cout << "One or both accounts not found." << endl;
        }
    }

    Account *findAccountByNumber(const string &accNumber)
    {
        for (auto &acc : accounts)
        {
            if (acc->getAccountNumber() == accNumber)
            {
                return acc;
            }
        }
        return nullptr;
    }
};

int main()
{
    cout << "\n\n\t\t\t\t======================\n";
    cout << "\t\t\t\tBANK MANAGEMENT SYSTEM";
    cout << "\n\t\t\t\t======================\n";

    cout << "\t\t\t\t    ::MAIN MENU::\n";
    BankingSystem bankingSystem;

    while (true)
    {
        bankingSystem.displayMenu();

        int choice;
        cout << "\n\n\t\t\t\tSelect Your Option (1-9): ";
        cin >> choice;

        if (choice == 9)
        {
            // Clean up memory using a pointer to the base class
            for (auto &acc : bankingSystem.getAccounts())
            {
                Account *baseAccount = acc;
                delete baseAccount;
            }
            break; // Exit the loop and end the program
        }

        bankingSystem.executeChoice(choice);
    }

    return 0;
}
