#include "transactionswindow.h"
#include "ui_transactionswindow.h"

TransactionsWindow::TransactionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionsWindow)
{
    ui->setupUi(this);
}

TransactionsWindow::~TransactionsWindow()
{
    delete ui;
}

void TransactionsWindow::putSessionData(SessionData *session)
{
    this->session = session;
    updateUI();
}

void TransactionsWindow::updateUI()
{
    //see if previous can be shown
    if(listIndex == 0)
    {
        ui->buttonPrevious->setFlat(true);
        ui->buttonPrevious->setDisabled(true);
    }
    else
    {
        ui->buttonPrevious->setFlat(false);
        ui->buttonPrevious->setDisabled(false);
    }

    //see if next can be shown
    if(session->additionalAccountNames.count() - listIndex <= 5)
    {
        ui->buttonNext->setFlat(true);
        ui->buttonNext->setDisabled(true);
    }
    else
    {
        ui->buttonNext->setFlat(false);
        ui->buttonNext->setDisabled(false);
    }

    //prints the list
    QList<QLabel *> transactionLabels = ui->layoutWrapper->findChildren<QLabel *>();

    QListIterator<QString> transactionDates(session->transactionDates);
    transactionDates.toBack();
    QListIterator<double> transactionAmounts(session->transactionAmounts);
    transactionAmounts.toBack();

    //reel iterator to index
    for (int i = 0; i < listIndex; ++i) {
        transactionDates.previous();
        transactionAmounts.previous();
    }

    for (int i = 0; i < 5; ++i) {
        qDebug() << "transactionDates list size: " << session->transactionDates.count() <<
                    "listIndex: " << listIndex;
        //index would traverse outside of list
        if(i + listIndex >= session->transactionDates.count())
        {
            qDebug() << "Index outside of list, outputting a blank";
            transactionLabels[i]->setText(" ");
        }
        else
        {
            qDebug() << "Index inside of list, outputting transaction[" << i + listIndex;
            transactionLabels[i]->setText(transactionDates.previous() +
                    ": " + QString::number(-1 * transactionAmounts.previous()));
        }
    }
}

void TransactionsWindow::on_buttonLogout_clicked()
{
    emit session->sendLogout();
}


void TransactionsWindow::on_buttonReturn_clicked()
{
    done(Accepted);
}


void TransactionsWindow::on_buttonPrevious_clicked()
{
    listIndex -= 5;
    updateUI();
}


void TransactionsWindow::on_buttonNext_clicked()
{
    listIndex += 5;
    updateUI();
}

