#include "withdrawwindow.h"
#include "ui_withdrawwindow.h"

#include <QDebug>

WithdrawWindow::WithdrawWindow(QWidget *parent, SessionData *session) :
    QWidget(parent),
    ui(new Ui::WithdrawWindow)
{
    ui->setupUi(this);

    this->session = session;

    session->resetTimer();

    updateUI();

    connect(ui->buttonGroup, SIGNAL(idClicked(int)),
            this, SLOT(withdrawButtonClicked(int)));
}

WithdrawWindow::~WithdrawWindow()
{
    delete ui;

    session->resetTimer();
}

void WithdrawWindow::updateUI()
{
    ui->labelWithdrawInfo->setText(session->customerName + " - " + session->withdrawMode);

    ui->labelCustomSum->setText(QString::number(customWithdrawAmount));

    if(invalidAttempt == true)
    {
        if(session->language == "fi")
        {
            if(session->withdrawMode == "debit")
            {
                ui->labelWithdrawWarning->setText("Kate ei riitä!");
            }
            else
            {
                ui->labelWithdrawWarning->setText("Luotto ei riitä!");
            }
        }
        if(session->language == "en")
        {
            if(session->withdrawMode == "debit")
            {
                ui->labelWithdrawWarning->setText("Balance is not Sufficient!");
            }
            else
            {
                ui->labelWithdrawWarning->setText("Credit is not Sufficient!");
            }
        }
    }
    else
    {
        ui->labelWithdrawWarning->setText("");
    }

    //other ui elements
    if(session->language == "fi")
    {
        ui->buttonCustomWithdraw->setText("Nosta muu summa");
        ui->labelWithCustomWithdrawInfo->setText("Pienin sallittu oma summa: 40");
        ui->buttonLogout->setText("Kirjaudu Ulos");
        ui->buttonReturn->setText("Palaa");
        ui->labelWithMoreInfo->setText("Valitse summa:");
    }
    if(session->language == "en")
    {
        ui->buttonCustomWithdraw->setText("Withdraw custom");
        ui->labelWithCustomWithdrawInfo->setText("Min. amount for custom withdraw: 40");
        ui->buttonLogout->setText("Log out");
        ui->buttonReturn->setText("Return");
        ui->labelWithMoreInfo->setText("Select the amount:");
    }
}

void WithdrawWindow::freezeUI(bool frozen)
{
    if(frozen)
    {
        ui->buttonLogout->setDisabled(true);
        ui->buttonReturn->setDisabled(true);

        QList<QAbstractButton *> accountButtons = ui->buttonGroup->buttons();

        foreach (QAbstractButton * button, accountButtons) {
            button->setDisabled(true);
        }
    }
    else
    {
        ui->buttonLogout->setDisabled(false);
        ui->buttonReturn->setDisabled(false);

        QList<QAbstractButton *> accountButtons = ui->buttonGroup->buttons();

        foreach (QAbstractButton * button, accountButtons) {
            button->setDisabled(false);
        }
    }
}

void WithdrawWindow::withdrawButtonClicked(int buttonID)
{
    //get button value (text of the button)
    int buttonValue = ui->buttonGroup->button(buttonID)->text().toInt();

    qDebug() << Q_FUNC_INFO << "Withdrawal amount selected: " << buttonValue;
    withdrawMoney(buttonValue);
}

void WithdrawWindow::withdrawMoney(int amount)
{
    invalidAttempt = false;
    freezeUI(true);
    updateUI();

    session->withdrawAmount = amount;

    if(session->withdrawMode == "debit")
    {
        double newAmount = session->accountBalance - (double)amount;

        if(newAmount < 0.00)
        {
            qDebug() << Q_FUNC_INFO << "withdrawAmount exceeds accountBalance!\n"
                     << "\tBalance: " << session->accountBalance
                     << "| After attempted withdrawal: " << newAmount;

            invalidAttempt = true;
            freezeUI(false);
            updateUI();
            return;
        }

        qDebug() << Q_FUNC_INFO << "Old accountBalance: " << session->accountBalance;
        session->accountBalance = newAmount;
        qDebug() << Q_FUNC_INFO << "New accountBalance: " << session->accountBalance;
    }
    else
    {
        double newAmount = session->accountCredit + (double)amount;

        if(newAmount > session->accountCreditMax)
        {
            qDebug() << Q_FUNC_INFO << "withdrawAmount exceeds creditMax!\n"
                     << "\tBalance: " << session->accountCredit
                     << "| After attempted withdrawal: " << newAmount
                     << "\n\tcreditMax: " << session->accountCreditMax;

            invalidAttempt = true;
            freezeUI(false);
            updateUI();
            return;
        }
        qDebug() << Q_FUNC_INFO << "Old accountCredit: " << session->accountCredit;
        session->accountCredit = newAmount;
        qDebug() << Q_FUNC_INFO << "New accountCredit: " << session->accountCredit;
    }

    //communicate changes to server
    connect(session->restApi, SIGNAL(withdrawalSuccess(bool)),
            this, SLOT(handleResponse(bool)));
    session->restApi->setAccountBalance(session->accountID,
                                        session->withdrawAmount, session->withdrawMode);
}

void WithdrawWindow::handleResponse(bool success)
{
    freezeUI(false);

    if(!success)
    {
        qDebug() << Q_FUNC_INFO << "transaction was unsuccessful";
        invalidAttempt = true;
        updateUI();
        return;
    }

    receiptWindow = new ReceiptWindow(this, session);
    receiptWindow->show();
}


void WithdrawWindow::on_buttonLogout_clicked()
{
    emit session->sendLogout(this);
}


void WithdrawWindow::on_buttonReturn_clicked()
{
    delete this;
}


void WithdrawWindow::on_buttonIncrement_clicked()
{
    if (customWithdrawAmount + 10 <= 1000) { //cäpätty tuhanteen nostoraja yhdellä kerralla
        customWithdrawAmount += 10;
        ui->labelCustomSum->setText(QString::number(customWithdrawAmount));
    } else {
        customWithdrawAmount = 1000;
    }
}


void WithdrawWindow::on_buttonDecrement_clicked()
{
    if (customWithdrawAmount <= 40) { //minimisumma minkä voi nostaa on 40 (2x 20 euron seteliä)
        customWithdrawAmount += 10;
    } else {
        customWithdrawAmount -= 10;
        ui->labelCustomSum->setText(QString::number(customWithdrawAmount));
    }
}


void WithdrawWindow::on_buttonCustomWithdraw_clicked()
{
    //get button value (text of the button)
    int buttonValue = ui->labelCustomSum->text().toInt();
    qDebug() << Q_FUNC_INFO << "Custom withdrawal amount selected: " << buttonValue;
    withdrawMoney(buttonValue);
}

