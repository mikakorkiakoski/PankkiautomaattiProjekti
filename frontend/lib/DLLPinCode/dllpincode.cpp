#include "dllpincode.h"
#include "ui_DLLPinCode.h"

DLLPinCode::DLLPinCode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DLLPinCode)
{
    ui->setupUi(this);
    //cardHexCode = "060006491c";
     ui->labelInterrupt->setVisible(false);
     ui->lineEdit->setMaxLength(4); // Set the maximum length to 4 digits
     ui->lineEdit->setReadOnly(true);
     ui->lineEdit->setEchoMode(QLineEdit::Password);
    connect(ui->button1,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button2,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button3,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button4,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button5,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button6,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button7,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button8,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button9,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->button0,SIGNAL(clicked()),this,SLOT(numberClickHandler()));
    connect(ui->buttonEnter,SIGNAL(clicked()),this,SLOT(enterClickHandler()));
    connect(ui->ButtonClear,SIGNAL(clicked()),this,SLOT(clearClickHandler()));
    connect(ui->ButtonStop,SIGNAL(clicked()),this,SLOT(stopClickHandler()));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(stopClickHandler()));
    timer->start(10000);

}

DLLPinCode::~DLLPinCode()
{
    delete ui;
}
//tämä funktio vastaanottaa cardhexcoden Mikan DLLpincoden käyttöön (kts. mainwindow.cpp:n signaalit)
void DLLPinCode::handleCardHexCodeReceived(const QString& hexCode)
{
    //asettaa testinä labellille sen hexcodearvon, joka ensin tuli DLLSerialPortin kautta Exeen ja sieltä vielä tänne.
    ui->cardhexcodeLabel->setText(hexCode);
}

void DLLPinCode::numberClickHandler()
{
    QPushButton *numberButton = qobject_cast<QPushButton *>(sender());
        QString clickedValue = numberButton->text();
        timer->stop();
        timer->start(10000);

        InsertingPin += clickedValue;
        ui->lineEdit->setText(InsertingPin);
        CheckPin = clickedValue;

}

void DLLPinCode::enterClickHandler()
{
      timer->stop();
      CheckPin = ui->lineEdit->text();
      //emit sendPin(CheckPin.toShort());
      if(CompareStrings(CheckPin,SQLPin))
      {
           emit sendPin(SQLPin.toShort());
      }
      else
      {
          ui->label->setText("Väärin, syötä tunnusluku uudestaan.");
          timer->start(10000);
      }
}
void DLLPinCode::clearClickHandler()
{
      InsertingPin = "";
      ui->lineEdit->clear();
      timer->stop();
      timer->start(10000);

}

void DLLPinCode::stopClickHandler()
{
      ui->labelInterrupt->setVisible(true);
      ui->label->setVisible(false);
      ui->label_3->setVisible(false);
      ui->labelAttempts->setVisible(false);
      ui->button0->setVisible(false);
      ui->button1->setVisible(false);
      ui->button2->setVisible(false);
      ui->button3->setVisible(false);
      ui->button4->setVisible(false);
      ui->button5->setVisible(false);
      ui->button6->setVisible(false);
      ui->button7->setVisible(false);
      ui->button8->setVisible(false);
      ui->button9->setVisible(false);
      ui->ButtonStop->setVisible(false);
      ui->ButtonClear->setVisible(false);
      ui->buttonEnter->setVisible(false);
      ui->lineEdit->setVisible(false);
}






bool DLLPinCode::CompareStrings(QString str1,QString str2)
{
    if(str1.compare(str2) == 0) {
        return true;
    }
    else {
        return false;
    }
}