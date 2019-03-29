#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QTime>

#include "enums.h"

namespace Ui
{
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget();

private slots:
    void on_rcMainModeButton_released();
    void on_rcAnswerModeButton_released();

private:
    void rcTcpServerNewConnection();
    void cduTcpServerNewConnection();
    void umuTcpServerNewConnection();

    void rcTcpSocketReadyRead();
    void rcTcpSocketDisconnected();

    void cduTcpSocketReadyRead();
    void cduTcpSocketDisconnected();

    void umuTcpSocketReadyRead();
    void umuTcpSocketDisconnected();

    void setRcConnectionLabelConnected(bool isConnected);
    void setCduConnectionLabelConnected(bool isConnected);
    void setUmuConnectionLabelConnected(bool isConnected);

    void rcWatchdogTimeout();
    void cduWatchdogTimeout();
    void umuWatchdogTimeout();

    void rcPingTimerTimeout();
    void cduPingTimerTimeout();
    void umuPingTimerTimeout();

    void parseRcMessages();
    void parseCduMessages();
    void parseUmuMessages();

    void printMessageFromRc(QString message);
    void printMessageFromCdu(QString message);
    void printMessageFromUmu(QString message);

    void printOperatorAction(char action);
    void printAnswerQuestion(char answer);
    void printCduMode(CduMode mode);

    void systemTimeTimerTimeout();

private:
    Ui::Widget* ui;
    QTcpServer* _rcTcpServer = nullptr;
    QTcpServer* _cduTcpServer = nullptr;
    QTcpServer* _umuTcpServer = nullptr;
    QTcpSocket* _rcTcpSocket = nullptr;
    QTcpSocket* _cduTcpSocket = nullptr;
    QTcpSocket* _umuTcpSocket = nullptr;

    bool _isRcConnected = false;
    bool _isCduConnected = false;
    bool _isUmuConnected = false;

    QTimer _rcWatchdog;
    QTimer _cduWatchdog;
    QTimer _umuWatchdog;

    QTimer _rcPingTimer;
    QTimer _cduPingTimer;
    QTimer _umuPingTimer;

    QByteArray _rcMessageBuffer;
    QByteArray _cduMessageBuffer;
    QByteArray _umuMessageBuffer;

    QTime _systemTime;
    QTimer _systemTimeTimer;
};

#endif  // WIDGET_H
