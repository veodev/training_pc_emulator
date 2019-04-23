#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtEndian>

const int PING_INTERVAL_MS = 500;
const int WATCHDOG_INTERVAL_MS = 3000;

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("TRAINING PC");

    _rcTcpServer = new QTcpServer(this);
    connect(_rcTcpServer, &QTcpServer::newConnection, this, &Widget::rcTcpServerNewConnection);
    _rcTcpServer->listen(QHostAddress::Any, 50002);

    _cduTcpServer = new QTcpServer(this);
    connect(_cduTcpServer, &QTcpServer::newConnection, this, &Widget::cduTcpServerNewConnection);
    _cduTcpServer->listen(QHostAddress::Any, 50003);

    _umuTcpServer = new QTcpServer(this);
    connect(_umuTcpServer, &QTcpServer::newConnection, this, &Widget::umuTcpServerNewConnection);
    _umuTcpServer->listen(QHostAddress::Any, 50004);

    connect(&_rcWatchdog, &QTimer::timeout, this, &Widget::rcWatchdogTimeout);
    _rcWatchdog.setInterval(WATCHDOG_INTERVAL_MS);

    connect(&_cduWatchdog, &QTimer::timeout, this, &Widget::cduWatchdogTimeout);
    _cduWatchdog.setInterval(WATCHDOG_INTERVAL_MS);

    connect(&_umuWatchdog, &QTimer::timeout, this, &Widget::umuWatchdogTimeout);
    _umuWatchdog.setInterval(WATCHDOG_INTERVAL_MS);

    connect(&_rcPingTimer, &QTimer::timeout, this, &Widget::rcPingTimerTimeout);
    _rcPingTimer.setInterval(PING_INTERVAL_MS);

    connect(&_cduPingTimer, &QTimer::timeout, this, &Widget::cduPingTimerTimeout);
    _cduPingTimer.setInterval(PING_INTERVAL_MS);

    connect(&_umuPingTimer, &QTimer::timeout, this, &Widget::umuPingTimerTimeout);
    _umuPingTimer.setInterval(PING_INTERVAL_MS);

    connect(&_systemTimeTimer, &QTimer::timeout, this, &Widget::systemTimeTimerTimeout);
    _systemTimeTimer.setInterval(1);
    _systemTimeTimer.start();
    _systemTime.start();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::rcTcpServerNewConnection()
{
    if (_rcTcpSocket != nullptr) {
        disconnect(_rcTcpSocket, &QTcpSocket::readyRead, this, &Widget::rcTcpSocketReadyRead);
        disconnect(_rcTcpSocket, &QTcpSocket::disconnected, this, &Widget::rcTcpSocketDisconnected);
        _rcTcpSocket->disconnectFromHost();
        _rcTcpSocket->deleteLater();
        _rcTcpSocket = nullptr;
    }
    _rcTcpSocket = _rcTcpServer->nextPendingConnection();
    connect(_rcTcpSocket, &QTcpSocket::readyRead, this, &Widget::rcTcpSocketReadyRead);
    connect(_rcTcpSocket, &QTcpSocket::disconnected, this, &Widget::rcTcpSocketDisconnected);
    setRcConnectionLabelConnected(true);
    _rcPingTimer.start();
    qDebug() << "New RC connection";
}

void Widget::cduTcpServerNewConnection()
{
    if (_cduTcpSocket != nullptr) {
        disconnect(_cduTcpSocket, &QTcpSocket::readyRead, this, &Widget::cduTcpSocketReadyRead);
        disconnect(_cduTcpSocket, &QTcpSocket::disconnected, this, &Widget::cduTcpSocketDisconnected);
        _cduTcpSocket->disconnectFromHost();
        _cduTcpSocket->deleteLater();
        _cduTcpSocket = nullptr;
    }
    _cduTcpSocket = _cduTcpServer->nextPendingConnection();
    connect(_cduTcpSocket, &QTcpSocket::readyRead, this, &Widget::cduTcpSocketReadyRead);
    connect(_cduTcpSocket, &QTcpSocket::disconnected, this, &Widget::cduTcpSocketDisconnected);
    setCduConnectionLabelConnected(true);
    _cduPingTimer.start();
}

void Widget::umuTcpServerNewConnection()
{
    if (_umuTcpSocket != nullptr) {
        disconnect(_umuTcpSocket, &QTcpSocket::readyRead, this, &Widget::umuTcpSocketReadyRead);
        disconnect(_umuTcpSocket, &QTcpSocket::disconnected, this, &Widget::umuTcpSocketDisconnected);
        _umuTcpSocket->disconnectFromHost();
        _umuTcpSocket->deleteLater();
        _umuTcpSocket = nullptr;
    }
    _umuTcpSocket = _umuTcpServer->nextPendingConnection();
    connect(_umuTcpSocket, &QTcpSocket::readyRead, this, &Widget::umuTcpSocketReadyRead);
    connect(_umuTcpSocket, &QTcpSocket::disconnected, this, &Widget::umuTcpSocketDisconnected);
    setUmuConnectionLabelConnected(true);
    _umuPingTimer.start();
}

void Widget::rcTcpSocketReadyRead()
{
    while (_rcTcpSocket->bytesAvailable()) {
        _rcMessageBuffer.append(_rcTcpSocket->readAll());
    }
    parseRcMessages();
}

void Widget::rcTcpSocketDisconnected()
{
    ui->rcConnectionLabel->setStyleSheet("background-color: red");
    _rcTcpSocket->deleteLater();
    _rcTcpSocket = nullptr;
    qDebug() << "RC disconnected!";
}

void Widget::cduTcpSocketReadyRead()
{
    while (_cduTcpSocket->bytesAvailable()) {
        _cduMessageBuffer.append(_cduTcpSocket->readAll());
    }
    parseCduMessages();
}

void Widget::cduTcpSocketDisconnected()
{
    ui->cduConnectionLabel->setStyleSheet("background-color: red");
    _cduTcpSocket->deleteLater();
    _cduTcpSocket = nullptr;
}

void Widget::umuTcpSocketReadyRead()
{
    while (_umuTcpSocket->bytesAvailable()) {
        _umuMessageBuffer.append(_umuTcpSocket->readAll());
    }
    parseUmuMessages();
}

void Widget::umuTcpSocketDisconnected()
{
    ui->umuConnectionLabel->setStyleSheet("background-color: red");
    _umuTcpSocket->deleteLater();
    _umuTcpSocket = nullptr;
}

void Widget::setRcConnectionLabelConnected(bool isConnected)
{
    isConnected ? ui->rcConnectionLabel->setStyleSheet("background-color: green") : ui->rcConnectionLabel->setStyleSheet("background-color: red");
}

void Widget::setCduConnectionLabelConnected(bool isConnected)
{
    isConnected ? ui->cduConnectionLabel->setStyleSheet("background-color: green") : ui->cduConnectionLabel->setStyleSheet("background-color: red");
}

void Widget::setUmuConnectionLabelConnected(bool isConnected)
{
    isConnected ? ui->umuConnectionLabel->setStyleSheet("background-color: green") : ui->umuConnectionLabel->setStyleSheet("background-color: red");
}

void Widget::rcWatchdogTimeout()
{
    _rcPingTimer.stop();
    _isRcConnected = false;
    ui->rcConnectionLabel->setStyleSheet("background-color: red");
}

void Widget::cduWatchdogTimeout()
{
    _cduPingTimer.stop();
    _isCduConnected = false;
    ui->cduConnectionLabel->setStyleSheet("background-color: red");
}

void Widget::umuWatchdogTimeout()
{
    _umuPingTimer.stop();
    _isUmuConnected = false;
    ui->umuConnectionLabel->setStyleSheet("background-color: red");
}

void Widget::rcPingTimerTimeout()
{
    if (_rcTcpSocket != nullptr) {
        MessageHeader header;
        header.Id = static_cast<unsigned char>(MessageId::PingId);
        header.Reserved1 = 0;
        header.Size = 0;
        _rcTcpSocket->write(reinterpret_cast<char*>(&header), sizeof(header));
        _rcTcpSocket->flush();
    }
}

void Widget::cduPingTimerTimeout()
{
    if (_cduTcpSocket != nullptr) {
        MessageHeader header;
        header.Id = static_cast<unsigned char>(MessageId::PingId);
        header.Reserved1 = 0;
        header.Size = 0;
        _cduTcpSocket->write(reinterpret_cast<char*>(&header), sizeof(header));
        _cduTcpSocket->flush();
    }
}

void Widget::umuPingTimerTimeout()
{
    if (_umuTcpSocket != nullptr) {
        MessageHeader header;
        header.Id = static_cast<unsigned char>(MessageId::PingId);
        header.Reserved1 = 0;
        header.Size = 0;
        _umuTcpSocket->write(reinterpret_cast<char*>(&header), sizeof(header));
        _umuTcpSocket->flush();
    }
}

void Widget::parseRcMessages()
{
    MessageHeader header;
    while (true) {
        if (_rcMessageBuffer.size() >= static_cast<int>(sizeof(header))) {
            header.Id = static_cast<uchar>(_rcMessageBuffer.at(0));
            header.Size = qFromLittleEndian<ushort>(reinterpret_cast<const uchar*>(_rcMessageBuffer.mid(2, sizeof(ushort)).data()));
            _rcMessageBuffer.remove(0, sizeof(header));
            if (_rcMessageBuffer.size() >= header.Size) {
                switch (static_cast<MessageId>(header.Id)) {
                case MessageId::PingId:
                    _rcWatchdog.start();
                    setRcConnectionLabelConnected(true);
                    //                    printMessageFromRc("PING");
                    break;
                case MessageId::AnswerToQuestionId: {
                    char answer = _rcMessageBuffer.at(0);
                    _rcMessageBuffer.remove(0, 1);
                    printAnswerQuestion(answer);
                    break;
                }
                case MessageId::OperatorActionId: {
                    char action = _rcMessageBuffer.at(0);
                    _rcMessageBuffer.remove(0, 1);
                    printOperatorAction(action);
                    break;
                }
                default:
                    break;
                }
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }
}

void Widget::parseCduMessages()
{
    MessageHeader header;
    while (true) {
        if (_cduMessageBuffer.size() >= static_cast<int>(sizeof(header))) {
            header.Id = static_cast<uchar>(_cduMessageBuffer.at(0));
            header.Size = qFromLittleEndian<ushort>(reinterpret_cast<const uchar*>(_cduMessageBuffer.mid(2, sizeof(ushort)).data()));
            _cduMessageBuffer.remove(0, sizeof(header));
            if (_cduMessageBuffer.size() >= header.Size) {
                switch (static_cast<MessageId>(header.Id)) {
                case MessageId::PingId:
                    _cduWatchdog.start();
                    //                    printMessageFromCdu("PING");
                    break;
                case MessageId::BoltJointOnId:
                    printMessageFromCdu("BOLT JOINT ON");
                    break;
                case MessageId::BoltJointOffId:
                    printMessageFromCdu("BOLT JOINT OFF");
                    break;
                case MessageId::RailTypeId:
                    printMessageFromCdu("Rail Type");
                    break;
                case MessageId::OperatorTrackCoordinateId: {
                    char km = _cduMessageBuffer.at(0);
                    char pk = _cduMessageBuffer.at(1);
                    printMessageFromCdu(QString("TRACK MARK: %1km %2pk").arg(int(km)).arg(int(pk)));
                    _cduMessageBuffer.remove(0, 2);
                    break;
                }
                case MessageId::RailroadSwitchMarkId: {
                    char number = _cduMessageBuffer.at(0);
                    printMessageFromCdu(QString("RAILROAD SWITCH: %1").arg(int(number)));
                    _cduMessageBuffer.remove(0, 1);
                    break;
                }
                case MessageId::DefectMarkId: {
                    char size = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    QString defectCode = QString::fromUtf16(reinterpret_cast<const ushort*>(_cduMessageBuffer.data()), size);
                    _cduMessageBuffer.remove(0, size * 2);
                    char side = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    printMessageFromCdu("DEFECT: " + defectCode + " SIDE: " + QString::number(int(side)));
                    break;
                }
                case MessageId::ChangeCduModeId:
                    printCduMode(static_cast<CduMode>(_cduMessageBuffer.at(0)));
                    _cduMessageBuffer.remove(0, 1);
                    break;
                case MessageId::RegistrationOnId: {
                    qDebug() << _cduMessageBuffer.left(header.Size).toHex(' ');

                    char operatorNameSize = _cduMessageBuffer.at(0);
                    qDebug() << "OPERATOR NAME LENGTH: " << operatorNameSize;
                    _cduMessageBuffer.remove(0, 1);
                    QString operatorName = QString::fromUtf16(reinterpret_cast<const ushort*>(_cduMessageBuffer.data()), operatorNameSize);
                    _cduMessageBuffer.remove(0, operatorNameSize * 2);
                    char railroadPathSize = _cduMessageBuffer.at(0);
                    qDebug() << "RAILROAD PATH NAME LENGTH: " << railroadPathSize;
                    _cduMessageBuffer.remove(0, 1);
                    QString railroadPath = QString::fromUtf16(reinterpret_cast<const ushort*>(_cduMessageBuffer.data()), railroadPathSize);
                    _cduMessageBuffer.remove(0, railroadPathSize * 2);
                    char pathNumber = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    char direction = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    char km = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    char pk = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    char m = _cduMessageBuffer.at(0);
                    _cduMessageBuffer.remove(0, 1);
                    printMessageFromCdu("REGISTRATION ON");
                    printMessageFromCdu("Operator: " + operatorName);
                    printMessageFromCdu("Path: " + railroadPath);
                    printMessageFromCdu("Path number: " + QString::number(int(pathNumber)));
                    printMessageFromCdu("Direction: " + QString::number(int(direction)));
                    printMessageFromCdu(QString("%1 km %2 pk %3 m").arg(int(km)).arg(int(pk)).arg(int(m)));
                    break;
                }
                case MessageId::RegistrationOffId:
                    printMessageFromCdu("REGISTRATION OFF");
                    break;
                default:
                    break;
                }
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }
}

void Widget::parseUmuMessages()
{
    MessageHeader header;
    while (true) {
        if (_umuMessageBuffer.size() >= static_cast<int>(sizeof(header))) {
            header.Id = static_cast<uchar>(_umuMessageBuffer.at(0));
            header.Size = qFromLittleEndian<ushort>(reinterpret_cast<const uchar*>(_umuMessageBuffer.mid(2, sizeof(ushort)).data()));
            _umuMessageBuffer.remove(0, sizeof(header));
            if (_umuMessageBuffer.size() >= header.Size) {
                switch (static_cast<MessageId>(header.Id)) {
                case MessageId::PingId:
                    _umuWatchdog.start();
                    printMessageFromUmu("PING");
                    break;
                default:
                    break;
                }
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }
}

void Widget::printMessageFromRc(QString message)
{
    ui->rcPlainTextEdit->appendPlainText(message);
}

void Widget::printMessageFromCdu(QString message)
{
    ui->cduPlainTextEdit->appendPlainText(message);
}

void Widget::printMessageFromUmu(QString message)
{
    ui->umuPlainTextEdit->appendPlainText(message);
}

void Widget::printOperatorAction(char action)
{
    switch (static_cast<OperatorAction>(action)) {
    case OperatorAction::PutFlagAction:
        printMessageFromRc("OperatorAction::PutFlagAction");
        break;
    case OperatorAction::PutSwitchLocker:
        printMessageFromRc("OperatorAction::PutSwitchLocker");
        break;
    }
}

void Widget::printAnswerQuestion(char answer)
{
    printMessageFromRc(QString("Answer: %1").arg(static_cast<int>(answer)));
}

void Widget::printCduMode(CduMode mode)
{
    switch (mode) {
    case CduMode::HandMode:
        printMessageFromCdu("HAND MODE");
        break;
    case CduMode::BScanMode:
        printMessageFromCdu("BSCAN MODE");
        break;
    case CduMode::PauseMode:
        printMessageFromCdu("PAUSE MODE");
        break;
    case CduMode::EvaluationMode:
        printMessageFromCdu("EVALUATION MODE");
        break;
    case CduMode::CalibrationMode:
        printMessageFromCdu("CALIBRATION MODE");
        break;
    default:
        break;
    }
}

void Widget::systemTimeTimerTimeout()
{
    ui->systemTimeLabel->setText(QTime::fromMSecsSinceStartOfDay(_systemTime.elapsed()).toString("hh:mm:ss.zzz"));
}

void Widget::on_rcMainModeButton_released()
{
    if (_rcTcpSocket != nullptr) {
        MessageHeader header;
        header.Id = static_cast<unsigned char>(MessageId::ChangeRcModeId);
        header.Reserved1 = 0;
        header.Size = 0;
        RcMode mode = RcMode::MainMode;
        _rcTcpSocket->write(reinterpret_cast<char*>(&header), sizeof(header));
        _rcTcpSocket->write(reinterpret_cast<char*>(&mode), sizeof(RcMode));
        _rcTcpSocket->flush();
    }
}

void Widget::on_rcAnswerModeButton_released()
{
    if (_rcTcpSocket != nullptr) {
        MessageHeader header;
        header.Id = static_cast<unsigned char>(MessageId::ChangeRcModeId);
        header.Reserved1 = 0;
        header.Size = 0;
        RcMode mode = RcMode::AnswerMode;
        _rcTcpSocket->write(reinterpret_cast<char*>(&header), sizeof(header));
        _rcTcpSocket->write(reinterpret_cast<char*>(&mode), sizeof(RcMode));
        _rcTcpSocket->flush();
    }
}
