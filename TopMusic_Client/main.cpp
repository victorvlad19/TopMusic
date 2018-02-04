#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include "user.h"
#include "ui_user.h"
#include "admin.h"
#include "ui_admin.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

// Variabile Globale
char salut[50]="Salut ";
string lista[1000][1000];
int i_matrice = 0;
int j_matrice = 0;
int numar_coloane = 0;
char user_session[50];

// Dimensiunea ferestrei
int latime = 620;
int inaltime = 420;
int x, y;

// Conectare
int sd;			// descriptorul de socket
struct sockaddr_in server;	// structura folosita pentru conectare
char msg[100];		// mesajul trimis
char adresa[] = "127.0.0.1"; /* stabilim adresa de contectare */
int port = 2728; /* stabilim portul */

void close_conection () {
    char msg[100] = "quit";

    /* trimiterea mesajului la server de [deconectare] */
    if (write(sd, msg, 100) <= 0) {
    QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !");
                m.setWindowTitle("EROARE !"); }

    /* inchidem conexiunea, am terminat */
    close (sd);}

void User::list_comments(){
    char comment_name[50];

    if (write(sd, "list_comments", 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    strcpy (comment_name, ui->comboBox_comm->currentText().toLatin1().data());

    if (write(sd, comment_name, 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    ui->listWidget_comm->clear();

    read(sd, msg, 300);
    char * parti;
    int count = 0;

    parti = strtok (msg,",");
    while (parti != NULL) {
        ui->listWidget_comm->addItem(parti);
        count ++;
        parti = strtok (NULL, ",");
    }
    cout << count;

}
void User::list_music_table(){
    // Afisare
    for (int i=0; i<numar_coloane; i++){
        QPushButton* button_commentarii = new QPushButton();

        button_commentarii->setText("Voteaza"); button_commentarii->setStyleSheet("QPushButton {" "color: blue;"  "background-color: rgb(240, 240, 240);" "}" );
        ui ->tableWidget->setCellWidget(i,5,button_commentarii);
        connect(button_commentarii, &QPushButton::clicked, this,&User::on_pushButton_voteaza);


        for (int j=0; j<5; j++){
            QTableWidgetItem* celula = new QTableWidgetItem(lista[i][j].c_str());
            ui->tableWidget->setItem(i,j,celula);
            if (j == 4) celula->setTextAlignment(Qt::AlignCenter);
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QDesktopWidget *fereastra = QApplication::desktop();

    // META INTERFATA
    x = (fereastra->width() - latime) / 2;
    y = (fereastra->height() - inaltime) / 2;
    w.resize(latime, inaltime);
    w.move( x, y );
    w.setWindowTitle("Top Music");


    // CONECTAREA
    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        QMessageBox m; m.setText("Eroare la socket(). Nu s-a putut realiza conexiunea !");
                m.setWindowTitle("EROARE !"); m.exec();; return errno; }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    server.sin_family = AF_INET; /* familia socket-ului */
    server.sin_addr.s_addr = inet_addr(adresa); /* adresa IP a serverului */
    server.sin_port = htons (port); /* portul de conectare */

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    { QMessageBox m; m.setText("Eroare la connect(). Nu s-a putut realiza conexiunea !");
        m.setWindowTitle("EROARE !"); m.exec(); return errno; }

    w.show();

    return a.exec();
}

// Logare Button Click
void MainWindow::on_pushButton_clicked()
{
    char username_login[50];
    char pass_login [50];
    char login_database_user[50] = "login_database_user ";
    char login_database_pass[50] = "login_database_pass ";
    bool login_ck = false;

    strcpy (username_login, ui ->lineEdit->text().toLatin1().data());
    strcpy (pass_login, ui ->lineEdit_2->text().toLatin1().data());

    // Verificam daca campurile sunt nule
    if (strlen(username_login) == 0) { ui->label_7->setText("  [EROARE] Campul username nu este completat !"); }
    else if (strlen(pass_login) == 0) { ui->label_7->setText("  [EROARE] Campul parola nu este completat !"); }
    else {
        ui->label_7->setText("  Incarcare .. ");

        // Trimitem la server
        // Contruim mesajul pentru server
        strcat(login_database_user,username_login);
        strcat(login_database_pass,pass_login);

        /* trimiterea mesajului la server */
        if (write(sd, login_database_user, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, login_database_pass, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

        /* Citim mesajului de la server */

        /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
        read(sd, msg, 100);

        char * parti;
        char login_name_show[50];
        parti = strtok (msg," ");
        while (parti != NULL)
        {
            if (strcmp (msg,"login_ok") == 0) login_ck = true;
            if (login_ck) strcpy(login_name_show, parti);
            parti = strtok (NULL, " ");
        }


        if (login_ck) {
            char exclamare[50] = " !";
            login_name_show[0] = toupper(login_name_show[0]);
            strcat(salut ,login_name_show);
            strcat(salut,exclamare);

            strcpy(user_session,username_login);
            /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
            read(sd, msg, 100);

           if (strcmp (msg,"user") == 0) {
              ui->label_7->setText("  V-ati logat cu succes !");
              hide (); User u; u.setModal(true); u.setWindowTitle("Top Music"); u.exec();
           }
           if (strcmp (msg,"admin") == 0) {
              ui->label_7->setText("  V-ati logat cu succes !");
              hide (); Admin a; a.setModal(true); a.setWindowTitle("Top Music"); a.exec();
           }

            // Curatam
            login_ck = false;
        } else {  login_ck = false; ui->label_7->setText("  [EROARE] Utilizatorul nu exista !"); }

        // Curatam
        ui->lineEdit->clear();
        ui->lineEdit_2->clear();
    }

}

// Inregistrare Button Click
void MainWindow::on_pushButton_2_clicked()
{
    char name_register[50];
    char username_register[50];
    char pass_register[50];
    char check_register [50];
    char add_database_name[50] = "add_database_name ";
    char add_database_user[50] = "add_database_user ";
    char add_database_pass[50] = "add_database_pass ";
    char add_database_admin[50] = "add_database_admin ";

    strcpy (name_register, ui->lineEdit_3->text().toLatin1().data());
    strcpy (username_register, ui->lineEdit_4->text().toLatin1().data());
    strcpy (pass_register, ui->lineEdit_5->text().toLatin1().data());

    // Verificam daca campurile sunt nule
    if (strlen(name_register) == 0) { ui->label_7->setText("  [EROARE] Campul nume nu este completat !"); }
    else if (strlen(username_register) == 0) { ui->label_7->setText("  [EROARE] Campul username nu este completat !"); }
    else if (strlen (pass_register) == 0) { ui->label_7->setText("  [EROARE] Campul parola nu este completat !"); }
    else {
        ui->label_7->setText("  Incarcare .. ");

        if (ui->checkBox->checkState()) { strcpy(check_register,"admin");
        } else strcpy(check_register,"user");

        // Trimitem la server
        // Contruim mesajul pentru server
        strcat(add_database_name,name_register);
        strcat(add_database_user, username_register);
        strcat(add_database_pass, pass_register);
        strcat(add_database_admin, check_register);

        /* trimiterea mesajului la server */
        if (write(sd, add_database_name, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, add_database_user, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, add_database_pass, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, add_database_admin, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

        /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
        read(sd, msg, 100);
        if (strcmp (msg,"register_ok") == 0) ui->label_7->setText("  V-ati inregistrat cu succes !");

        // Curatam
        ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
        ui->lineEdit_5->clear();
    }
}

// Inchidem conexiunea la apsarea butonului close
void MainWindow::closeEvent (QCloseEvent *event) { close_conection(); }
void User::closeEvent(QCloseEvent *bar) { close_conection();}
void Admin::closeEvent(QCloseEvent *bar) { close_conection();}

const string stringLiber = "";
string extrage_string( string initial, string start, string final )
{
     size_t pozitiaStart = initial.find( start );

     if( pozitiaStart == string::npos ){ return stringLiber;}

     pozitiaStart += start.length();
     string::size_type index_final = initial.find( final, pozitiaStart );
     return initial.substr( pozitiaStart, index_final - pozitiaStart );
}
bool terminator_final(const string& str, const string& sufix){
    return str.size() >= sufix.size() && str.rfind(sufix) == (str.size()-sufix.size());
}

void populare_tabel (){

    // Cere Numar total din lista
    if (write(sd, "list_music" , 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    // Primeste
    read(sd, msg, 100);
    long n;
    n = atol(msg);

    // Citim coloanele
    i_matrice = 0; j_matrice = 0;
    for (int i=0; i<n; i++){
       read(sd, msg, 300);
       char * parte;
         parte = strtok (msg," ");
         while (parte != NULL)
         {
           if (terminator_final(parte,",")){
               string extracted = extrage_string( parte, "'", "'" );
               lista[i_matrice][j_matrice++] = extracted;
           }
           parte = strtok (NULL, " ");
         }
         i_matrice++;
         j_matrice = 0;
    }
    numar_coloane = i_matrice;

}

// Music Adaugare Button Click
void User::on_pushButton_clicked(){
    char music_name[50];
    char music_desc[50];
    char music_gen[50];
    char music_link[50];
    char add_music_name[50] = "add_music_name ";
    char add_music_desc[50] = "add_music_desc ";
    char add_music_gen[50] = "add_music_gen ";
    char add_music_link[50] = "add_music_link ";

    strcpy (music_name, ui->m_name->text().toLatin1().data());
    strcpy (music_desc, ui->m_desc->text().toLatin1().data());
    strcpy (music_link, ui->m_link->text().toLatin1().data());
    strcpy (music_gen, ui->comboBox->currentText().toLatin1().data());

    // Verificam daca campurile sunt nule
    if (strlen(music_name) == 0) { ui->m_label_7->setText("  [EROARE] Campul nume nu este completat !"); }
    else if (strlen(music_desc) == 0) { ui->m_label_7->setText("  [EROARE] Campul descriere nu este completat !"); }
    else if (strlen(music_link) == 0) { ui->m_label_7->setText("  [EROARE] Campul link nu este completat !"); }
    else {
        ui->m_label_7->setText("  Incarcare .. ");

        // Trimitem la server
        // Contruim mesajul pentru server
        strcat(add_music_name,music_name);
        strcat(add_music_desc, music_desc);
        strcat(add_music_gen, music_gen);
        strcat(add_music_link, music_link);

         /* trimiterea mesajului la server */
        if (write(sd, add_music_name, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, add_music_desc, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, add_music_gen, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
        if (write(sd, add_music_link, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

        /* citirea raspunsului dat de server
                   (apel blocant pina cind serverul raspunde) */
                if (read(sd, msg, 100) < 0) {
                    //perror("[client]Eroare la read() de la server.\n");
                }
        if (strcmp (msg,"music_add_ok") == 0) ui->m_label_7->setText("  Melodia a fost adaugata cu succes !");

        // Adaugam in tabel
        memset (lista,0,1000);
        ui->tableWidget->clear();
        populare_tabel();

        ui->tableWidget->setRowCount(numar_coloane);
        ui->tableWidget->setSortingEnabled(false);

        for( int i = 0; i < ui->tableWidget->rowCount(); ++i ) {
            ui->tableWidget->setRowHidden( i, false );
        }
        // Numarul de melodii
        ui->m_total->setText(("Total: " + to_string(numar_coloane)).c_str());

        list_music_table();

        QModelIndex index = ui->tableWidget->model()->index(0,2);
        ui->tableWidget->setCurrentIndex(index);

        // Rescriem capul de tabel
        ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Nume"));
        ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Descriere"));
        ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Gen"));
        ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Link"));
        ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Nr. Voturi"));
         ui->tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
        ui->tableWidget->setSortingEnabled(true);

        // Curatam
        ui->m_name->clear();
        ui->m_desc->clear();
        ui->m_link->clear();

    }
}

// Button de Filtrare
void User::on_pushButton_3_clicked()
{
    long nr = 0;
    QString filtreaza = ui->comboBox_2->currentText();
    for( int i = 0; i < ui->tableWidget->rowCount(); ++i )
    {
        bool ok = false;
        for( int j = 0; j < ui->tableWidget->rowCount(); ++j )
        {
            QTableWidgetItem *item = ui->tableWidget->item( i, 2 );
            if( item->text().contains(filtreaza) )
            {
                ok = true;
                break;
            }
        }
        ui->tableWidget->setRowHidden( i, !ok );
        if (ok) { nr ++; }
    }

    // Numarul de melodii
    ui->m_total->setText(("Total: " + to_string(nr)).c_str());
    nr = 0;
}

void User::on_pushButton_f_ex_clicked()
{
    // Numarul de melodii
    ui->m_total->setText(("Total: " + to_string(numar_coloane)).c_str());

    for( int i = 0; i < ui->tableWidget->rowCount(); ++i ) {
        ui->tableWidget->setRowHidden( i, false );
    }
}


User::User(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::User) {
    ui->setupUi(this);
    // Textul Salut
    ui->label_salut->setText(salut);
    // Populare ComboBox
    ui->comboBox->addItem("Hip-Hop");
    ui->comboBox->addItem("Clasica");
    ui->comboBox->addItem("Pop");
    ui->comboBox->addItem("Dubstep");
    ui->comboBox->addItem("Manele");
    ui->comboBox->addItem("Rock");
    ui->comboBox->addItem("Metal");
    ui->comboBox->addItem("Electronica");
    ui->comboBox->addItem("Trap");
    ui->comboBox_2->addItem("Hip-Hop");
    ui->comboBox_2->addItem("Clasica");
    ui->comboBox_2->addItem("Pop");
    ui->comboBox_2->addItem("Dubstep");
    ui->comboBox_2->addItem("Manele");
    ui->comboBox_2->addItem("Rock");
    ui->comboBox_2->addItem("Metal");
    ui->comboBox_2->addItem("Electronica");
    ui->comboBox_2->addItem("Trap");

    // Populare Tabel
    populare_tabel();
    // Setare Tabel
    ui->tableWidget->horizontalHeader()->sortIndicatorOrder();
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->setRowCount(numar_coloane);
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Nume"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Descriere"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Gen"));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Link"));
    ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Nr. Voturi"));
    ui->tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Numarul de melodii
    ui->m_total->setText(("Total: " + to_string(numar_coloane)).c_str());

    list_music_table();

    QModelIndex index = ui->tableWidget->model()->index(0,2);
    ui->tableWidget->setCurrentIndex(index);

    // COMMENTARII Populare

    // NUME MELODII
    // Cerem numarul total de melodii din lista
    if (write(sd, "list_music_name" , 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    // Primeste
    read(sd, msg, 100);
    long n;
    n = atol(msg);

    // Adauga in combo-box
    for (int i=0; i<n; i++){
        read(sd, msg, 300);
        ui->comboBox_comm->addItem(msg);
    }

    // Populare commentari
    list_comments();
}

// Button de votare
void User::on_pushButton_voteaza()
{
    int row = ui->tableWidget->currentRow();
    char add_vote[50] = "add_vote ";
    char vote_name[50];

    if (row != -1) {

        ui->m_label_7->setText(ui -> tableWidget ->item(row,0)->text());
        strcpy (vote_name, ui -> tableWidget ->item(row,0)->text().toLatin1());

        // Trimitem la server
        // Contruim mesajul pentru server
        strcat(add_vote, vote_name);

        if (write(sd, add_vote, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

        if (write(sd, user_session, 100) <= 0)
        { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

        read(sd, msg, 100);

        if (strcmp (msg,"vote_add_ok") == 0) {
            ui->m_label_7->setText("  Ati votat cu succes !");

            // Rescriem tabelul
            memset (lista,0,1000);
            ui->tableWidget->clear();
            populare_tabel();

            ui->tableWidget->setRowCount(numar_coloane);
            ui->tableWidget->setSortingEnabled(false);

            for( int i = 0; i < ui->tableWidget->rowCount(); ++i ) {
                ui->tableWidget->setRowHidden( i, false );
            }
            // Numarul de melodii
            ui->m_total->setText(("Total: " + to_string(numar_coloane)).c_str());

            list_music_table();

            QModelIndex index = ui->tableWidget->model()->index(0,2);
            ui->tableWidget->setCurrentIndex(index);

            // Rescriem capul de tabel
            ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Nume"));
            ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Descriere"));
            ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Gen"));
            ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Link"));
            ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Nr. Voturi"));
            ui->tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
            ui->tableWidget->setSortingEnabled(true);

        }
        if (strcmp (msg,"vote_add_false") == 0){ ui->m_label_7->setText("  Aveti dreptul de votare restrictionat !"); }

    }
}

// Comment Button
void User::on_comm_push_clicked() {

    char comment_data [50];
    char comment_name [50];

    ui->m_label_7->setText("  Incarcare...");

    if (write(sd, "add_comment", 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    // Pregatim mesajul pentru server
    strcpy (comment_data, ui->lineEdit_comm->text().toLatin1().data());
    strcpy (comment_name, ui->comboBox_comm->currentText().toLatin1().data());

    // Trimitem serverului
    if (write(sd, comment_data, 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
    if (write(sd, comment_name, 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    // Clear
    ui->lineEdit_comm->clear();
     ui->m_label_7->setText("  Commentariu adaugat cu succes !");

    // Repopulate list
    list_comments();

}

// Change channel click
void User::on_channel_button_clicked() { list_comments(); }

// Admin
Admin::Admin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Admin)
{
    ui->setupUi(this);
    
    // NUME MELODII
    // Cerem numarul total de melodii din lista
    if (write(sd, "list_music_name" , 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
    
    // Primeste
    read(sd, msg, 100);
    long n;
    n = atol(msg);

    // Adauga in combo-box
    for (int i=0; i<n; i++){
        read(sd, msg, 300);
        ui->adm_cmb_sterge->addItem(msg);
    }

    // NUME UTILIZATORI
    // Cerem numarul total de utilizatoir din lista
    if (write(sd, "list_user_name" , 100) <= 0)
    { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

    // Primeste
    read(sd, msg, 100);
    long n_user;
    n_user = atol(msg);

    // Adauga in combo-box
    for (int i=0; i<n_user; i++){
        read(sd, msg, 300);
        ui->adm_cmb_res->addItem(msg);
    }

}

void Admin::on_adm_sterge_clicked() {
    char delete_music[50] = "delete_music";

    /* trimiterea mesajului la server */
   if (write(sd, delete_music, 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
   if (write(sd, ui->adm_cmb_sterge->currentText().toLatin1().data(), 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

   ui->adm_cmb_sterge->clear();

   // Cerem numarul total de melodii din lista
   if (write(sd, "list_music_name" , 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

   // Primeste
   read(sd, msg, 100);
   long n;
   n = atol(msg);

   // Adauga in combo-box
   for (int i=0; i<n; i++){
       read(sd, msg, 300);
       ui->adm_cmb_sterge->addItem(msg);
   }

   ui->adm_status->setText("User sters !");

}

// Restrictioneaza voatarea
void Admin::on_adm_res_clicked() {
    /* trimiterea mesajului la server */
   if (write(sd, "restrict_user", 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
   if (write(sd, ui->adm_cmb_res->currentText().toLatin1().data(), 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

   ui->adm_status->setText("User restrictionat !");

}

// Scoate restrictionarea
void Admin::on_adm_res_2_clicked() {
    /* trimiterea mesajului la server */
   if (write(sd, "un-restrict_user", 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }
   if (write(sd, ui->adm_cmb_res->currentText().toLatin1().data(), 100) <= 0)
   { QMessageBox m; m.setText("Eroare la write(). Nu s-a putut scrie mesaj serverului !"); m.setWindowTitle("EROARE !"); m.show(); }

   ui->adm_status->setText("Restrictionare scoasa !");

}

