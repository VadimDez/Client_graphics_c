#include "client.h"
#include "ui_client.h"
#include <iostream>
#include <QCryptographicHash>
#include <QMessageBox>
client::client(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::client)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    //scene->setSceneRect(0,0,0,0);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->rotate(-180);
    ui->graphicsView->scale(-1,1);
    pen.setColor(Qt::black);
    pen.setWidth(1);
    maxY = ui->graphicsView->height()/2;
    maxX = ui->graphicsView->width()/2;
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
    ui->listWidget->hide(); // per nascondere i punti
    quadrato = 10;
}

client::~client()
{
    delete ui;
}

void client::Connect()
{
    socket = new QTcpSocket(this);
    socket->connectToHost(ip,port); // indirizzo
    if(socket->waitForConnected(1000))
    { // se conesso:
        ui->stato->setText("Connesso");
        // faccio sparire ip, port e il bottone 'connect'
        ui->ip->setVisible(false);
        ui->port->setVisible(false);
        ui->ConnectButton->setVisible(false);
        ui->labelPort->setVisible(false);
        ui->labelIp->setVisible(false);
    }
    else
    {
        ui->stato->setText("Non connesso");
    }
}
void client::on_ConnectButton_clicked()
{
    ip = ui->ip->text();
    Connect();
}

void client::on_port_valueChanged(int arg1)
{
    port = arg1;
}

void client::on_drawButton_clicked()
{
    QPen graf;
    graf.setColor(Qt::red);
    graf.setWidth(3);
    ui->listWidget->clear();
    double y;
    QByteArray info;
    string equazione;
    bool exit = false;
    bool isX = false;
    first = true;
    maxY = ui->graphicsView->height()/2;
    maxX = ui->graphicsView->width()/2;
    drawLines();
    equazione = ui->equazione->text().toStdString();
    if(equazione != "") // controllo se la stringa non e' vuota
    {
        // in caso se non c'e' la X nella equazione deve prendere la X come 0
        // altrimenti se c'e' la X devo dare un valore a X
        unsigned int m=0;
        x = 0;
        do
        {
            if(equazione[m] == 'x')
            {
                graf.setWidth(1);
                exit = true;
                x = (int)-maxX/2;
                isX = true;
            }
            m++;
            if(m==equazione.size())
            {
                exit = true;
            }
        }while(!exit);
        exit = false;
        do // ciclo per mandare X al server e ottenere la Y
        {
            packet(equazione,info); // implemento packetto per mandare al server
            socket->write(info); // mando packetto
            socket->waitForBytesWritten(1000);
            info.clear();
            socket->waitForReadyRead(100);
            int size = socket->bytesAvailable();
            info = socket->read(size);
            if(info[0] == 'y' && info[1] == '=') // controllo per vedere se non ritorna errore
            { // in caso se non c'e' errore ritorna il valore di Y
                QByteArray temporary;
                for(int i=2;i<info.size();i++) // copio valore di Y
                {
                    temporary += info[i];
                }
                y = temporary.toDouble();
                // Per disegnare vuole la coordinata precedentemente disegnata,
                // ma visto che e' il primo punto non ha ancora disegnato nulla
                // quindi salvo come punti precedenti quelli appena trovati
                // e in questo caso non disegnera una retta ma solo un punto
                if(first) // se e' primo punto:
                {
                    oldX = x;
                    oldY = y;
                    first = false;
                }
                // per disegnare la retta dal punto precedentemente disegnato al nuovo punto
                // metto "*quadrato" perche quadrato e' la grandezza du un quadratino
                scene->addLine(oldX*quadrato,oldY*quadrato,x*quadrato,y*quadrato,graf);
                oldX = x; // salvo la x come punto gia' disegnato
                oldY = y; // e la y
            }
            else
            { // in caso se c'e' errore utente vede un MSGBOX con stcritto errore;
                QMessageBox msg;
                msg.setText("ERRORE!");
                msg.exec();
                exit = true;
            }
            // aggiungo nella lista i punti:
            ui->listWidget->addItem("x="+QString::number(x)+" y=" + QString::number(y));
            if(isX) // isX controllo che dice se c'era la x nell'equazione
            {
                x++;
                if(x == (int)(maxX/2))
                {
                    exit = true;
                }
            }
            else
            {
                exit = true;
            }

        }while(!exit);
        scene->update();
    }
}

void client::on_zoomIn_clicked() // ZoomIn
{
    quadrato += 10;
    on_drawButton_clicked();
    if(quadrato > 10) // faccio disponibile tasto per zoom out
    {
        ui->zoomOut->setEnabled(true);
    }
}
void client::on_zoomOut_clicked() // ZoomOut
{
    quadrato -= 10;
    // controllo per sapere se devo far non disponibile il tasto di ZoomOut
    if(quadrato >= 10)
    {
        on_drawButton_clicked();
        if(quadrato == 10)
        {
            ui->zoomOut->setEnabled(false);
        }
    }
}

// per far nascondere o far vedere i punti di X e Y
void client::on_checkBox_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->listWidget->hide();
    }
    else
    {
        ui->listWidget->show();
    }
}

void client::drawLines()
{
    scene->clear();
    QPen pen;
    pen.setColor(Qt::blue);
    scene->addLine(-maxX,0,maxX,0,pen);
    scene->addLine(0,maxY,0,-maxY,pen);
    // qui faccio quadratini per lo sfondo
    pen.setColor(Qt::black);
    pen.setStyle(Qt::DotLine);
    for(int quadr=quadrato;quadr<=maxX;quadr+=quadrato)
    {
        scene->addLine(-maxX,quadr,maxX,quadr,pen); // fa le linee orizzontale y > 0(sopra asse x)
        scene->addLine(-maxX,-quadr,maxX,-quadr,pen);//fa le linee orizzontale y < 0(sotto asse x)
    }
    for(int quadr=quadrato;quadr<=maxY;quadr+=quadrato)
    {
        scene->addLine(quadr,maxY,quadr,-maxY,pen); // le linee vert. x > 0
        scene->addLine(-quadr,maxY,-quadr,-maxY,pen); // vert. x < 0
    }
}
// procedura che crea packetto da mandare in modo che viene:
// ST&x=valoredellaX&y=equazione&cs=checksumInMd5&ET
void client::packet(string str, QByteArray &pack)
{
    pack.clear();
    pack += "ST"; // Inizio trasmissione;
    pack += "&x=";
    //info += (char)x; // valore della X
    pack += QString::number(x).toLocal8Bit();
    pack += "&y=";
    pack += str.c_str(); // aggiungo equazione
    pack += "&cs="; // per sapere dove inizia check sum;
    QCryptographicHash cryp(QCryptographicHash::Md5);
    cryp.addData(pack);
    pack += cryp.result().toHex().data(); // Checksum della equazione;
    pack += "&ET"; // Fine trasmissione;
}

