#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QtCore>
#include <QtGui>
using namespace std;
namespace Ui {
    class client;
}

class client : public QDialog
{
    Q_OBJECT

public:
    explicit client(QWidget *parent = 0);
    ~client();
    QString ip;
    int port;
    double maxX,maxY;
    bool first;
    double x;
    double oldX,oldY;
    double *vetX,*vetY;
    int quadrato;
    QPen pen;
    void Connect();
    void drawGraphic();
    void drawLines();
    void packet(string str, QByteArray &pack);

private slots:
    void on_ConnectButton_clicked();
    void on_port_valueChanged(int arg1);
    void on_drawButton_clicked();
    void on_zoomIn_clicked();
    void on_checkBox_stateChanged(int arg1);
    void on_zoomOut_clicked();

private:
    Ui::client *ui;
    QTcpSocket *socket;
    QGraphicsScene *scene;
};

#endif // CLIENT_H
