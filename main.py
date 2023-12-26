import csv
import tkinter as tk
from tkinter import messagebox, simpledialog
import datetime



class User:
    def __init__(self, uname, pwd):
        self.username = uname
        self.password = pwd

    def set_password(self, old_pwd, new_pwd):
        if old_pwd == self.password:
            self.password = new_pwd
            messagebox.showinfo("Success", "Password changed successfully.")
        else:
            messagebox.showerror("Error", "Incorrect old password. Password not changed.")


class Transaction:
    def __init__(self, cat, amt, time):
        self.category = cat
        self.amount = amt
        self.timestamp = time


class Account:
    def __init__(self, acc_number, initial_balance, acc_user):
        self.account_number = acc_number
        self.balance = initial_balance
        self.transaction_history = []
        self.user = acc_user

    def deposit(self, amount):
        self.balance += amount
        self.record_transaction("Deposit", amount)

    def withdraw(self, amount):
        entered_password = simpledialog.askstring("Withdraw", "Enter your password to withdraw:")
        if entered_password != self.user.password:
            messagebox.showerror("Error", "Incorrect password. Withdrawal canceled.")
            return

        if self.balance >= amount:
            self.balance -= amount
            self.record_transaction("Withdrawal", amount)
            messagebox.showinfo("Success", f"Withdrawal successful. Remaining balance: ${self.balance}")
        else:
            messagebox.showerror("Error", "Insufficient balance.")

    def display(self):
        messagebox.showinfo("Account Details", f"Account Number: {self.account_number}\nBalance: ${self.balance}")

    def get_account_number(self):
        return self.account_number

    def show_transaction_history(self):
        history_text = f"Transaction History for Account: {self.account_number}\n"
        for transaction in self.transaction_history:
            history_text += f"{transaction.timestamp} - {transaction.category}: ${transaction.amount}\n"

        messagebox.showinfo("Transaction History", history_text)

    def delete_account(self):
        messagebox.showinfo("Account Deleted", "Account deleted successfully.")

    def transfer(self, target_account, amount):
        entered_password = simpledialog.askstring("Transfer", "Enter your password to transfer funds:")
        if entered_password != self.user.password:
            messagebox.showerror("Error", "Incorrect password. Transfer canceled.")
            return

        if self.balance >= amount:
            self.balance -= amount
            target_account.deposit(amount)
            self.record_transaction(f"Transfer (To: {target_account.get_account_number()})", amount)
            messagebox.showinfo("Success", f"Transfer successful. Remaining balance: ${self.balance}")
        else:
            messagebox.showerror("Error", "Insufficient balance for transfer.")

    def record_transaction(self, transaction_type, amount):
        now = datetime.datetime.now()
        timestamp = now.strftime("%Y-%m-%d %H:%M:%S")

        transaction_record = f"{timestamp} - {transaction_type}: ${amount}"
        self.transaction_history.append(Transaction(transaction_type, amount, transaction_record))


class SavingsAccount(Account):
    def apply_interest(self):
        pass


class CurrentAccount(Account):
    def __init__(self, acc_number, initial_balance, overdraft, acc_user):
        super().__init__(acc_number, initial_balance, acc_user)
        self.overdraft_limit = overdraft

    def withdraw(self, amount):
        entered_password = simpledialog.askstring("Withdraw", "Enter your password to withdraw:")
        if entered_password != self.user.password:
            messagebox.showerror("Error", "Incorrect password. Withdrawal canceled.")
            return

        if self.balance + self.overdraft_limit >= amount:
            self.balance -= amount
            self.record_transaction("Withdrawal", amount)
            messagebox.showinfo("Success", f"Withdrawal successful. Remaining balance: ${self.balance}")
        else:
            self.record_transaction("Transaction Declined - Overdraft Limit Exceeded", 0.0)
            messagebox.showerror("Error", "Transaction declined. Overdraft limit exceeded.")


class BankingSystem:
    def __init__(self):
        self.users = {}
        self.accounts = []

        # Load existing account data from CSV file
        self.load_accounts_from_csv()

    def get_accounts(self):
        return self.accounts

    def create_user(self, username, password):
        self.users[username] = User(username, password)

    def authenticate_user(self, username, password):
        user = self.users.get(username)
        return user and user.password == password

    def create_account(self, acc_number, initial_balance, acc_type, username, password):
        new_user = self.users[username]

        if acc_type == "Savings":
            self.accounts.append(SavingsAccount(acc_number, initial_balance, new_user))
            messagebox.showinfo("Success", "Savings account created successfully.")
        elif acc_type == "Current":
            self.accounts.append(CurrentAccount(acc_number, initial_balance, 100.0, new_user))
            messagebox.showinfo("Success", "Current account created successfully.")
        else:
            messagebox.showerror("Error", "Invalid account type.")

    def deposit_funds(self, acc_number, deposit_amount):
        account = self.find_account_by_number(acc_number)
        if account:
            account.deposit(deposit_amount)
        else:
            messagebox.showerror("Error", "Account not found.")

    def withdraw_funds(self, acc_number, withdraw_amount):
        account = self.find_account_by_number(acc_number)
        if account:
            account.withdraw(withdraw_amount)
        else:
            messagebox.showerror("Error", "Account not found.")

    def display_account_details(self, acc_number):
        account = self.find_account_by_number(acc_number)
        if account:
            account.display()
        else:
            messagebox.showerror("Error", "Account not found.")

    def show_transaction_history(self, acc_number):
        account = self.find_account_by_number(acc_number)
        if account:
            account.show_transaction_history()
        else:
            messagebox.showerror("Error", "Account not found.")

    def change_password(self, username, old_password, new_password):
        user = self.users.get(username)
        if user:
            user.set_password(old_password, new_password)
        else:
            messagebox.showerror("Error", "User not found.")

    def delete_account(self, acc_number):
        for acc in self.accounts:
            if acc.get_account_number() == acc_number:
                acc.delete_account()
                del acc
                self.accounts.remove(acc)
                messagebox.showinfo("Success", "Account deleted successfully.")
                return

        messagebox.showerror("Error", "Account not found.")

    def transfer_funds(self, source_acc_number, target_acc_number, amount):
        source_account = self.find_account_by_number(source_acc_number)
        target_account = self.find_account_by_number(target_acc_number)

        if source_account and target_account:
            source_account.transfer(target_account, amount)
        else:
            messagebox.showerror("Error", "One or both accounts not found.")

    def find_account_by_number(self, acc_number):
        for acc in self.accounts:
            if acc.get_account_number() == acc_number:
                return acc
        return None

    def save_accounts_to_csv(self, filename='accounts.csv'):
        with open(filename, mode='w', newline='') as file:
            writer = csv.writer(file)
            # Write header
            writer.writerow(['account_number', 'balance', 'user_username', 'user_password'])
            for acc in self.accounts:
                writer.writerow([acc.account_number, acc.balance, acc.user.username, acc.user.password])

    def load_accounts_from_csv(self, filename='accounts.csv'):
        try:
            with open(filename, mode='r') as file:
                reader = csv.DictReader(file)
                self.accounts = []

                for row in reader:
                    acc_number = row['account_number']
                    balance = float(row['balance'])
                    username = row['user_username']
                    password = row['user_password']

                    user = self.users.get(username, User(username, password))
                    account = Account(acc_number, balance, user)

                    self.accounts.append(account)

        except (FileNotFoundError, csv.Error) as e:
            # Handle the case when the file is not found or the data is not valid CSV
            print(f"Error loading accounts from CSV: {e}")

    def create_account(self, acc_number, initial_balance, acc_type, username, password):
        new_user = self.users.get(username, User(username, password))

        if acc_type == "Savings":
            self.accounts.append(SavingsAccount(acc_number, initial_balance, new_user))
            messagebox.showinfo("Success", "Savings account created successfully.")
        elif acc_type == "Current":
            self.accounts.append(CurrentAccount(acc_number, initial_balance, 100.0, new_user))
            messagebox.showinfo("Success", "Current account created successfully.")
        else:
            messagebox.showerror("Error", "Invalid account type.")

        # Save updated accounts to CSV after creating a new account
        self.save_accounts_to_csv()




class BankingApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Banking System")

        self.banking_system = BankingSystem()

        self.create_widgets()

        # Set up an exit handler to save account data before exiting
        root.protocol("WM_DELETE_WINDOW", self.on_exit)

    def on_exit(self):
        # Save account data to file before exiting
        self.banking_system.save_accounts_to_file()
        self.root.destroy()

    def create_widgets(self):
        tk.Label(self.root, text="Banking System", font=("Helvetica", 16)).pack(pady=10)

        tk.Button(self.root, text="Create Account", command=self.create_account).pack()
        tk.Button(self.root, text="Deposit Funds", command=self.deposit_funds).pack()
        tk.Button(self.root, text="Withdraw Funds", command=self.withdraw_funds).pack()
        tk.Button(self.root, text="Display Account Details", command=self.display_account_details).pack()
        tk.Button(self.root, text="Show Transaction History", command=self.show_transaction_history).pack()
        tk.Button(self.root, text="Change Password", command=self.change_password).pack()
        tk.Button(self.root, text="Delete Account", command=self.delete_account).pack()
        tk.Button(self.root, text="Transfer Funds", command=self.transfer_funds).pack()
        tk.Button(self.root, text="Exit", command=self.root.destroy).pack()

    def create_account(self):
        acc_number = simpledialog.askstring("Create Account", "Enter account number:")
        initial_balance = float(simpledialog.askstring("Create Account", "Enter initial balance:"))
        acc_type = simpledialog.askstring("Create Account", "Enter account type (Savings or Current):")
        username = simpledialog.askstring("Create Account", "Enter your username:")
        password = simpledialog.askstring("Create Account", "Create a password for your account:")

        self.banking_system.create_user(username, password)
        self.banking_system.create_account(acc_number, initial_balance, acc_type, username, password)

    def deposit_funds(self):
        acc_number = simpledialog.askstring("Deposit Funds", "Enter account number:")
        deposit_amount = float(simpledialog.askstring("Deposit Funds", "Enter deposit amount:"))

        self.banking_system.deposit_funds(acc_number, deposit_amount)

    def withdraw_funds(self):
        acc_number = simpledialog.askstring("Withdraw Funds", "Enter account number:")
        withdraw_amount = float(simpledialog.askstring("Withdraw Funds", "Enter withdrawal amount:"))

        self.banking_system.withdraw_funds(acc_number, withdraw_amount)

    def display_account_details(self):
        acc_number = simpledialog.askstring("Account Details", "Enter account number:")

        self.banking_system.display_account_details(acc_number)

    def show_transaction_history(self):
        acc_number = simpledialog.askstring("Transaction History", "Enter account number:")

        self.banking_system.show_transaction_history(acc_number)

    def change_password(self):
        username = simpledialog.askstring("Change Password", "Enter your username:")
        old_password = simpledialog.askstring("Change Password", "Enter your old password:")
        new_password = simpledialog.askstring("Change Password", "Enter your new password:")

        self.banking_system.change_password(username, old_password, new_password)

    def delete_account(self):
        acc_number = simpledialog.askstring("Delete Account", "Enter account number to delete:")

        self.banking_system.delete_account(acc_number)

    def transfer_funds(self):
        source_acc_number = simpledialog.askstring("Transfer Funds", "Enter your account number:")
        target_acc_number = simpledialog.askstring("Transfer Funds", "Enter target account number:")
        amount = float(simpledialog.askstring("Transfer Funds", "Enter transfer amount:"))

        self.banking_system.transfer_funds(source_acc_number, target_acc_number, amount)


if __name__ == "__main__":
    root = tk.Tk()
    app = BankingApp(root)
    root.mainloop()
