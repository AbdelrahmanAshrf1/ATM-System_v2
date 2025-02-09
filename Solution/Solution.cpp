#pragma warning(disable : 4996)
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>

using namespace std;

void Login();
void ShowMainMenu();
void GoBackToMainMenu();
void ShowQuickWithdrawScreen();
void ShowNormalWithdrawScreen();
void ShowDepositScreen();
void ShowCheckBalanceScreen();
void ShowTransactionHistoryScreen();
void ShowChangeAccountInfoScreen();

struct sClient 
{
    string AccountNumber;
    string PinCode;
    string Name;
    string Phone;
    double AccountBalance;
    bool MarkForDelete = false;
};

struct stDate 
{
    short Year;
    short Month;
    short Day;
};

struct sTransactionData
{
    string OperationType;
    double Amount;
    double CurrentBalance;
    double NewBalance;
    stDate OperationDate;
};

const string ClientFileName = "Clients.txt";
sClient CurrentClient;

stDate GetSystemDate() 
{
    time_t CurrentTime = time(0);
    tm* Time = localtime(&CurrentTime);

    stDate Date;
    Date.Day = Time->tm_mday;
    Date.Month = (Time->tm_mon + 1);
    Date.Year = (Time->tm_year + 1900);

    return Date;
}

vector<string> SplitString(const string& S1, const string& Delim) 
{
    vector<string> vString;
    size_t pos = 0, start = 0;

    while ((pos = S1.find(Delim, start)) != string::npos) 
    {
        vString.push_back(S1.substr(start, pos - start));
        start = pos + Delim.length();
    }

    if (start < S1.length())
    {
        vString.push_back(S1.substr(start));
    }

    return vString;
}

sClient ConvertLineToRecord(const string& Line, const string& Seperator = "#//#")
{
    sClient Client;
    vector<string> vClientData = SplitString(Line, Seperator);

    Client.AccountNumber = vClientData[0];
    Client.PinCode = vClientData[1];
    Client.Name = vClientData[2];
    Client.Phone = vClientData[3];
    Client.AccountBalance = stod(vClientData[4]);

    return Client;
}

sTransactionData ConvertHistoryDataLineToRecord(const string& Line, const string& Seperator = "#//#")
{
    sTransactionData Data;
    vector<string> vTransactionData = SplitString(Line, Seperator);

    Data.OperationType = vTransactionData[0];
    Data.Amount = stod(vTransactionData[1]);
    Data.CurrentBalance = stod(vTransactionData[2]);
    Data.NewBalance = stod(vTransactionData[3]);
    Data.OperationDate.Day = stoi(vTransactionData[4]);
    Data.OperationDate.Month = stoi(vTransactionData[5]);
    Data.OperationDate.Year = stoi(vTransactionData[6]);

    return Data;
}

string ConvertRecordToLine(const sClient& Client, const string& Seperator = "#//#") 
{
    return Client.AccountNumber + Seperator + Client.PinCode + Seperator + Client.Name + Seperator + Client.Phone + Seperator + to_string(Client.AccountBalance);
}

string ConvertHistoryDataRecordToLine(const sTransactionData& Data, const string& Seperator = "#//#") 
{
    return Data.OperationType + Seperator + to_string(Data.Amount) + Seperator + to_string(Data.CurrentBalance) + Seperator + to_string(Data.NewBalance) + Seperator + to_string(Data.OperationDate.Day) + Seperator + to_string(Data.OperationDate.Month) + Seperator + to_string(Data.OperationDate.Year);
}

bool SaveClientsDataToFile(const string& FileName, const vector<sClient>& vClients)
{
    ofstream MyFile(FileName, ios::out);
    if (!MyFile.is_open()) 
    {
        cerr << "Error opening file for writing: " << FileName << endl;
        return false;
    }

    for (const auto& C : vClients) 
    {
        if (!C.MarkForDelete)
        {
            MyFile << ConvertRecordToLine(C) << endl;
        }
    }

    MyFile.close();
    return true;
}

bool SaveHistoryDataToFile(const string& FileName, const vector<sTransactionData>& vHistoryData) 
{
    ofstream MyFile(FileName, ios::out);
    if (!MyFile.is_open()) 
    {
        cerr << "Error opening file for writing: " << FileName << endl;
        return false;
    }

    for (const auto& Data : vHistoryData) 
    {
        MyFile << ConvertHistoryDataRecordToLine(Data) << endl;
    }

    MyFile.close();
    return true;
}

vector<sClient> LoadClientsDataFromFile(const string& FileName) 
{
    vector<sClient> vClients;
    ifstream MyFile(FileName, ios::in);
    if (!MyFile.is_open()) 
    {
        cerr << "Error opening file for reading: " << FileName << endl;
        return vClients;
    }

    string Line;
    while (getline(MyFile, Line)) 
    {
        vClients.push_back(ConvertLineToRecord(Line));
    }

    MyFile.close();
    return vClients;
}

vector<sTransactionData> LoadHistoryDataFromFile(const string& FileName) 
{
    vector<sTransactionData> vData;
    ifstream MyFile(FileName, ios::in);
    if (!MyFile.is_open())
    {
        cerr << "Error opening transaction history file: " << FileName << endl;
        return vData;
    }

    string Line;
    while (getline(MyFile, Line)) 
    {
        vData.push_back(ConvertHistoryDataLineToRecord(Line));
    }

    MyFile.close();
    return vData;
}

void CreateTransactionHistoryFile(const string& FileName) 
{
    ofstream outfile(FileName);
    if (!outfile) {
        cerr << "Error creating transaction history file: " << FileName << endl;
    }
}

bool DepositBalanceToClientByAccountNumber(const string& AccountNumber, double Amount, vector<sClient>& vClients) {
    char Answer;
    cout << "Are you sure you want to perform this transaction? (y/n): ";
    cin >> Answer;

    if (toupper(Answer) == 'Y')
    {
        for (auto& Client : vClients)
        {
            if (Client.AccountNumber == AccountNumber) 
            {
                Client.AccountBalance += Amount;
                if (!SaveClientsDataToFile(ClientFileName, vClients)) 
                {
                    cerr << "Failed to save client data!" << endl;
                    return false;
                }

                string FileName = "TransactionsHistoriesClients/" + CurrentClient.AccountNumber + ".txt";
                sTransactionData TransactionData;
                TransactionData.OperationType = Amount > 0 ? "Deposit" : "Withdraw";
                TransactionData.Amount = Amount;
                TransactionData.CurrentBalance = CurrentClient.AccountBalance;
                TransactionData.NewBalance = CurrentClient.AccountBalance + Amount;
                TransactionData.OperationDate = GetSystemDate();

                vector<sTransactionData> vHistory = LoadHistoryDataFromFile(FileName);
                vHistory.push_back(TransactionData);
                if (!SaveHistoryDataToFile(FileName, vHistory)) 
                {
                    cerr << "Failed to save transaction history!" << endl;
                    return false;
                }

                CurrentClient.AccountBalance += Amount;
                cout << "Transaction successful! New balance: " << Client.AccountBalance << endl;
                return true;
            }
        }
    }

    return false;
}

enum enMainMenuOptions { QuickWithdraw = 1, NormalWithdraw, Deposit, CheckBalance, TransactionHistory, ChangeInfo, Logout };

short ReadQuickWithdrawOption() 
{
    short Choice;
    do 
    {
        cout << "Choose an option [1-9]: ";
        cin >> Choice;
    } while (Choice < 1 || Choice > 9);
    return Choice;
}

short GetQuickWithdrawAmount(short Option) 
{
    switch (Option) 
    {
    case 1: return 20;
    case 2: return 50;
    case 3: return 100;
    case 4: return 200;
    case 5: return 400;
    case 6: return 600;
    case 7: return 800;
    case 8: return 1000;
    default: return 0;
    }
}

void PerformQuickWithdrawOption(short QuickWithdrawOption)
{
    if (QuickWithdrawOption == 9) return;

    double WithdrawBalance = GetQuickWithdrawAmount(QuickWithdrawOption);
    if (WithdrawBalance > CurrentClient.AccountBalance) 
    {
        cout << "Insufficient balance. Please choose a different amount.\n";
        system("pause");
        ShowQuickWithdrawScreen();
        return;
    }

    vector<sClient> vClients = LoadClientsDataFromFile(ClientFileName);
    DepositBalanceToClientByAccountNumber(CurrentClient.AccountNumber, -WithdrawBalance, vClients);
    CurrentClient.AccountBalance -= WithdrawBalance;
}

void ShowQuickWithdrawScreen() 
{
    system("cls");
    cout << "===========================================\n";
    cout << "\tQuick Withdraw Screen\n";
    cout << "===========================================\n";
    cout << "\t[1] $20\t\t[2] $50\n";
    cout << "\t[3] $100\t[4] $200\n";
    cout << "\t[5] $400\t[6] $600\n";
    cout << "\t[7] $800\t[8] $1000\n";
    cout << "\t[9] Main Menu\n";
    cout << "===========================================\n";
    PerformQuickWithdrawOption(ReadQuickWithdrawOption());
    GoBackToMainMenu();
}

void PerformNormalWithdrawOption(double WithdrawBalance) 
{
    if (WithdrawBalance > CurrentClient.AccountBalance) 
    {
        cout << "Insufficient balance.\n";
        return;
    }

    vector<sClient> vClients = LoadClientsDataFromFile(ClientFileName);
    DepositBalanceToClientByAccountNumber(CurrentClient.AccountNumber, -WithdrawBalance, vClients);
    
    CurrentClient.AccountBalance -= WithdrawBalance;
    
}

void ShowNormalWithdrawScreen() 
{
    system("cls");
    double WithdrawBalance;
    cout << "===========================================\n";
    cout << "\tNormal Withdraw Screen\n";
    cout << "===========================================\n";
    cout << "Enter amount to withdraw: ";
    cin >> WithdrawBalance;
    PerformNormalWithdrawOption(WithdrawBalance);
    GoBackToMainMenu();
}

void PerformDepositOption(double DepositBalance)
{
    vector<sClient> vClients = LoadClientsDataFromFile(ClientFileName);
    DepositBalanceToClientByAccountNumber(CurrentClient.AccountNumber, DepositBalance, vClients);
    CurrentClient.AccountBalance += DepositBalance;
}

void ShowDepositScreen() 
{
    system("cls");
    double DepositBalance;
    cout << "===========================================\n";
    cout << "\tDeposit Screen\n";
    cout << "===========================================\n";
    cout << "Enter amount to deposit: ";
    cin >> DepositBalance;
    PerformDepositOption(DepositBalance);
    GoBackToMainMenu();
}

void ShowCheckBalanceScreen() 
{
    system("cls");
    cout << "===========================================\n";
    cout << "\tBalance Inquiry Screen\n";
    cout << "===========================================\n";
    cout << "Your current balance is: $" << CurrentClient.AccountBalance << endl;
    GoBackToMainMenu();
}

void ShowTransactionHistoryScreen()
{
    system("cls");
    cout << "============================================================\n";
    cout << "\t\tTransaction History Screen\n";
    cout << "============================================================\n";
    string FileName = "TransactionsHistoriesClients/" + CurrentClient.AccountNumber + ".txt";
    vector<sTransactionData> vTransactions = LoadHistoryDataFromFile(FileName);

    cout << "Date\t\tType\t\tAmount\t\tBalance\n";
    cout << "------------------------------------------------------------\n";
    for (const auto& Transaction : vTransactions) 
    {
        cout << Transaction.OperationDate.Day << "/" << Transaction.OperationDate.Month << "/" << Transaction.OperationDate.Year;
        cout << "\t" << Transaction.OperationType;
        cout << "\t" << fixed << setprecision(2) << Transaction.Amount << "\t";
        cout << "\t" << fixed << setprecision(2) << Transaction.NewBalance << endl;
    }
    GoBackToMainMenu();
}

void ShowChangeAccountInfoScreen() 
{
    // Placeholder for future development
    system("cls");
    cout << "===========================================\n";
    cout << "\tChange Account Info Screen\n";
    cout << "===========================================\n";
    cout << "This feature is under construction.\n";
    GoBackToMainMenu();
}

void GoBackToMainMenu() 
{
    cout << "\nPress any key to go back to the main menu...\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
    ShowMainMenu();
}

short ReadMainMenuOption() 
{
    short Choice;
    do 
    {
        cout << "Choose an option [1-7]: ";
        cin >> Choice;
    } while (Choice < 1 || Choice > 7);
    return Choice;
}

void ShowMainMenu()
{
    system("cls");
    cout << "===========================================\n";
    cout << "\tMain Menu\n";
    cout << "===========================================\n";
    cout << "\t[1] Quick Withdraw\n";
    cout << "\t[2] Normal Withdraw\n";
    cout << "\t[3] Deposit\n";
    cout << "\t[4] Check Balance\n";
    cout << "\t[5] Transaction History\n";
    cout << "\t[6] Change Account Info\n";
    cout << "\t[7] Logout\n";
    cout << "===========================================\n";
    switch (ReadMainMenuOption()) 
    {
    case enMainMenuOptions::QuickWithdraw:
        ShowQuickWithdrawScreen();
        break;
    case enMainMenuOptions::NormalWithdraw:
        ShowNormalWithdrawScreen();
        break;
    case enMainMenuOptions::Deposit:
        ShowDepositScreen();
        break;
    case enMainMenuOptions::CheckBalance:
        ShowCheckBalanceScreen();
        break;
    case enMainMenuOptions::TransactionHistory:
        ShowTransactionHistoryScreen();
        break;
    case enMainMenuOptions::ChangeInfo:
        ShowChangeAccountInfoScreen();
        break;
    case enMainMenuOptions::Logout:
        Login();
        break;
    }
}

sClient FindClient(const string& AccountNumber, const string& PinCode) 
{
    vector<sClient> vClients = LoadClientsDataFromFile(ClientFileName);

    for (const auto& Client : vClients) {
        if (Client.AccountNumber == AccountNumber && Client.PinCode == PinCode) 
        {
            return Client;
        }
    }

    return {};
}

bool LoadClientData(const string& AccountNumber, const string& PinCode)
{
    CurrentClient = FindClient(AccountNumber, PinCode);
    return !CurrentClient.AccountNumber.empty();
}

void Login() 
{
    string AccountNumber, PinCode;
    bool LoggedIn = false;

    while (!LoggedIn) 
    {
        system("cls");
        cout << "===========================================\n";
        cout << "\tLogin Screen\n";
        cout << "===========================================\n";
        cout << "Enter Account Number: ";
        cin >> AccountNumber;
        cout << "Enter PIN Code: ";
        cin >> PinCode;

        if (LoadClientData(AccountNumber, PinCode)) 
        {
            LoggedIn = true;
        }
        else {
            cout << "Invalid Account Number or PIN Code. Please try again.\n";
            system("pause");
        }
    }

    ShowMainMenu();
}

int main() 
{
    Login();
    return 0;
}
