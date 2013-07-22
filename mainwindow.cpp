#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));

    tailleMessage = 0;
}

void MainWindow::on_boutonConnexion_clicked() {
    // On annonce sur la fenêtre qu'on est en train de se connecter
    ui->listeMessages->append(tr("<em>Tentative de connexion en cours...</em>"));
    ui->boutonConnexion->setEnabled(false);

    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(ui->serveurIP->text(), ui->serveurPort->value()); // On se connecte au serveur demandé
}

void MainWindow::on_boutonEnvoyer_clicked() {
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    // On prépare le paquet à envoyer
    QString messageAEnvoyer = tr("<strong>") + ui->pseudo->text() +tr("</strong> : ") + ui->message->text();

    out << (quint16) 0;
    out << messageAEnvoyer;
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet

    ui->message->clear(); // On vide la zone d'écriture du message
    ui->message->setFocus(); // Et on remet le curseur à l'intérieur
}

// Appuyer sur la touche Entrée a le même effet que cliquer sur le bouton "Envoyer"
void MainWindow::on_message_returnPressed() {
    on_boutonEnvoyer_clicked();
}

void MainWindow::donneesRecues() {
    /* Même principe que lorsque le serveur reçoit un paquet :
    On essaie de récupérer la taille du message
    Une fois qu'on l'a, on attend d'avoir reçu le message entier (en se basant sur la taille annoncée tailleMessage)
    */
    QDataStream in(socket);

    if (tailleMessage == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
             return;

        in >> tailleMessage;
    }

    if (socket->bytesAvailable() < tailleMessage)
        return;


    // Si on arrive jusqu'à cette ligne, on peut récupérer le message entier
    QString messageRecu;
    in >> messageRecu;

    // On affiche le message sur la zone de Chat
    ui->listeMessages->append(messageRecu);

    // On remet la taille du message à 0 pour pouvoir recevoir de futurs messages
    tailleMessage = 0;
}

void MainWindow::connecte() {
    ui->listeMessages->append(tr("<em>Connexion réussie !</em>"));
    ui->boutonConnexion->setEnabled(true);
}

void MainWindow::deconnecte() {
    ui->listeMessages->append(tr("<em>Déconnecté du serveur</em>"));
}

void MainWindow::erreurSocket(QAbstractSocket::SocketError erreur) {
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
        case QAbstractSocket::HostNotFoundError:
            ui->listeMessages->append(tr("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            ui->listeMessages->append(tr("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>"));
            break;
        case QAbstractSocket::RemoteHostClosedError:
            ui->listeMessages->append(tr("<em>ERREUR : le serveur a coupé la connexion.</em>"));
            break;
        default:
            ui->listeMessages->append(tr("<em>ERREUR : ") + socket->errorString() + tr("</em>"));
    }

    ui->boutonConnexion->setEnabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}
