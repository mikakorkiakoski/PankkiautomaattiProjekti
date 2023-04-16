#include "dllpincode.h"
#include "ui_dllpincode.h"

DLLPinCode::DLLPinCode(QWidget *parent, QString cardHexCodeReceived, QString language) :
    QDialog(parent),
    ui(new Ui::DLLPinCode)
{
    ui->setupUi(this);
    languageGlobal = language;
    if(languageGlobal == "fi")
    {
        finnish();
    }
    else
    {
        english();
    }
     ui->labelInterrupt->setVisible(false);
     ui->labelFreezed1->setVisible(false);
     ui->labelFreezed2->setVisible(false);
     ui->labelAttempts->setVisible(false);
     ui->lineEdit->setMaxLength(4); // Set the maximum length to 4 digits
     ui->lineEdit->setReadOnly(true); //Cannot write t
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
    timer->start(30000);
    cardHexCode = cardHexCodeReceived;
    qDebug() << "cardHexCode konstruktorissa:" << cardHexCode;
    getCardIDFromDb();
}

DLLPinCode::~DLLPinCode()
{
    delete ui;
}

QString DLLPinCode::getBaseUrl()
{
    return "https://bankdb-r18.onrender.com";
}

//tämä funktio hakee cardHexCoden perusteella cardID:n eli rivin primary keyn
void DLLPinCode::getCardIDFromDb()
{
    QString site_url = DLLPinCode::getBaseUrl() + "/hexcode/hex/" + cardHexCode;
    QNetworkRequest request((site_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QByteArray authHeader = QString("Bearer %1").arg(token).toLatin1();
    request.setRawHeader("Authorization", authHeader);
    QUrl url = request.url();
    qDebug() << "cardID pyynnön URL" << url.toString();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, [=](QNetworkReply *reply) {
                if (reply->error()) {
                    qDebug() << reply->errorString();
                }
                else {
                    QByteArray response = reply->readAll();
                    QJsonDocument document = QJsonDocument::fromJson(response);
                    QJsonObject object = document.object();
                    qDebug() << "Tämän hexan perusteella haetaan cardID:tä" << cardHexCode;
                    qDebug() << "Response (pitäisi tulla raw response)" << response;
                    if (!object.contains("idcard")) {
                        qDebug() << "idcard not found";
                    } else {
                        cardID = object.value("idcard").toString();
                        qDebug() << "idcard found: " << cardID;
                    }
                }
                getCardInfoFromDb();
                reply->deleteLater();
            });
    manager->get(request);
}

//tämä funktio hakee haetun cardID:n perusteella relevantit tiedot
void DLLPinCode::getCardInfoFromDb()
{
    QString site_url = DLLPinCode::getBaseUrl() + "/card/" + cardID;
    QNetworkRequest request((site_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QByteArray authHeader = QString("Bearer %1").arg(token).toLatin1();
    request.setRawHeader("Authorization", authHeader);
    QUrl url = request.url();
    qDebug() << "card info URL: " << url.toString();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, [=](QNetworkReply *reply) {
                if (reply->error()) {
                    qDebug() << reply->errorString();
                }
                else {
                    QByteArray response = reply->readAll();
                    qDebug() << "Raw response:" << response;
                    QJsonDocument document = QJsonDocument::fromJson(response);
                    QJsonObject object = document.object();
                    // Check if the response contains the cardhexcode key
                    cardhexcodeSQL = object.value("cardhexcode").toString();
                    SQLPin = object.value("fourdigitpin").toString();
                    wrongAttempts = object.value("wrongAttempts").toInt();
                    ui->labelAttempts->setText(QString::number(wrongAttempts) + " yritystä jäljellä");
                    qDebug() << "wrongAttemptsMäärä" << wrongAttempts;
                    ui->labeljee->setText(cardhexcodeSQL);
                    ui->cardhexcodeLabel->setText(cardHexCode);
                    ui->labelpin->setText(SQLPin);
                    if(wrongAttempts <= 0)
                    {
                        accountFreezed();
                    }
                }
                reply->deleteLater();
            });
    manager->get(request);
}

void DLLPinCode::updateWrongAttemptsInCard(const QString& cardID, int newWrongAttempts, const QString& token)
{
    qDebug() << "Updating wrongAttempts in card " << cardID << " to " << newWrongAttempts;
    QString site_url = DLLPinCode::getBaseUrl() + "/card/" + cardID;
    QNetworkRequest request((site_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QByteArray authHeader = QString("Bearer %1").arg(token).toLatin1();
    request.setRawHeader("Authorization", authHeader);
    QJsonObject requestBody;
    requestBody.insert("wrongAttempts", newWrongAttempts);
    QJsonDocument requestBodyDoc(requestBody);
    QByteArray requestBodyData = requestBodyDoc.toJson();
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error()) {
            qDebug() << "Failed to update wrongAttempts in card " << cardID << ", remaining attempts: " << reply->errorString();
        }
        else {
            QByteArray response = reply->readAll();
            qDebug() << "Updated wrongAttempts to idcard" << cardID << ", remaining attempts: " << response;
        }
        reply->deleteLater();
    });
    manager->put(request, requestBodyData);
}

void DLLPinCode::enterClickHandler()
{ 
    timer->stop();
    ui->buttonEnter->setFlat(true);
    ui->buttonEnter->setDisabled(true);

    while (cardhexcodeSQL.isEmpty() || SQLPin.isEmpty()) {
        QCoreApplication::processEvents();
    }

    CheckPin = ui->lineEdit->text();
    qDebug() << "Pin joka on syötetty:" << CheckPin;
    qDebug() << "Tietokannasta haettu PIN:" << SQLPin;
    qDebug() << "cardHexCode (luettu):" << cardHexCode;
    qDebug() << "cardhexcodeSQL (haettu):" << cardhexcodeSQL;
    if (cardhexcodeSQL == cardHexCode && CheckPin == SQLPin && wrongAttempts > 0)
    {
        updateWrongAttemptsInCard(cardID, 3, token);
        emit LoginSuccess(cardID.toInt());
        done(Accepted);
    }
    else
    {
        wrongAttempts--;
        if(wrongAttempts <= 0)
        {
            accountFreezed();
        }
        if(languageGlobal == "fi")
        {
            ui->label->setText("Väärin, syötä tunnusluku uudestaan.");
            ui->labelAttempts->setText(QString::number(wrongAttempts) + " yritys(tä) jäljellä!");
        }
        else
        {
            ui->label->setText("Wrong PIN, please try again again.");
            ui->labelAttempts->setText(QString::number(wrongAttempts) + " attempt(s) left!");
        }

        timer->start(30000);
        updateWrongAttemptsInCard(cardID,wrongAttempts,token);

        ui->labelAttempts->setVisible(true);
        clearClickHandler();
    }
    ui->buttonEnter->setFlat(false);
    ui->buttonEnter->setDisabled(false);
}


void DLLPinCode::numberClickHandler()
{
    QPushButton *numberButton = qobject_cast<QPushButton *>(sender());
        QString clickedValue = numberButton->text();
        timer->stop();
        timer->start(30000);

        InsertingPin += clickedValue;
        ui->lineEdit->setText(InsertingPin);
        CheckPin = clickedValue;
}


void DLLPinCode::clearClickHandler()
{
      InsertingPin = "";
      ui->lineEdit->clear();
      timer->stop();
      timer->start(30000);
}

void DLLPinCode::stopClickHandler()
{
      timer->stop();
      timer->start(5000);
      emit LoginSuccess(0);
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
      connect(timer, &QTimer::timeout, this, &QDialog::reject);
}

void DLLPinCode::accountFreezed()
{
      timer->stop();
      timer->start(5000);
      emit LoginSuccess(0);
      ui->labelFreezed1->setVisible(true);
      ui->labelFreezed2->setVisible(true);
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
      connect(timer, &QTimer::timeout, this, &QDialog::reject);
}

void DLLPinCode::finnish()
{
      ui->label->setText("Näppäile PIN-koodi");
      ui->labelAttempts->setText(" yritys(tä) jäljellä");
      ui->labelFreezed1->setText("Tili jäädytetty liian monen yrityksen takia.");
      ui->labelFreezed2->setText("Ota yhteys asiakaspalvelumme numeroon: +358 10 1234567");
      ui->labelInterrupt->setText("Tapahtuma keskeytetty");
      ui->label_3->setText("ja paina ENTER");
}

void DLLPinCode::english()
{
      ui->label->setText("Enter 4-digit PIN code");
      ui->labelAttempts->setText(" attempt(s) left");
      ui->labelFreezed1->setText("Account frozen due to excessive amount of attempts.");
      ui->labelFreezed2->setText("Please contact our customer service number +358 10 1234567");
      ui->labelInterrupt->setText("Login aborted");
      ui->label_3->setText("and press ENTER");
}
