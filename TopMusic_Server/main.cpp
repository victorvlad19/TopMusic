#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sqlite3.h>
#include <string>
#include "sqlite_orm.h"
#include <iostream>
#include <fcntl.h>

using namespace std;
using namespace sqlite_orm;
/* portul folosit */

#define PORT 2728

extern int errno;		/* eroarea returnata de unele apeluri */

struct User{
    int id;
    string name;
    string user;
    string pass;
    string admin;
    int vote;
};

struct Music{
    string name;
    string descriere;
    string gen;
    string link;
    string vote_num;
    string comments;
};


auto storage = make_storage ("topmusic.sqlite",make_table("users",
make_column("id", &User::id, autoincrement(), primary_key()),
make_column("name", &User::name),
make_column("user", &User::user),
make_column("pass", &User::pass),
make_column("type", &User::admin),
make_column("vote", &User::vote)
));

auto storage_music = make_storage ("topmusic.sqlite",make_table("music",
make_column("name", &Music::name),
make_column("descriere", &Music::descriere),
make_column("gen", &Music::gen),
make_column("link", &Music::link),
make_column("vote_num", &Music::vote_num),
make_column("comments", &Music::comments)));

/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address) {
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy (str, inet_ntoa (address.sin_addr));
    /* portul utilizat de client */
    bzero (port, 7);
    sprintf (port, ":%d", ntohs (address.sin_port));
    strcat (str, port);
    return (str);
}
int procesare(int fd);
void add_row_register (char register_name[50], char register_username[50], char register_pass[50], char admin_state[50] ){

    string add_name(register_name);
    string add_username(register_username);
    string add_pass (register_pass);
    string admin_st (admin_state);

    User user{-1, add_name, add_username, add_pass, admin_st , 1 };
    auto id_inserat = storage.insert(user);
    cout << "[BD] Inserat.. id = " << id_inserat << endl;

}

void add_row_music (char name_music[50], char desc_music[50], char gen_music[50], char link_music[50]){

    string add_name(name_music);
    string add_desc(desc_music);
    string add_gen(gen_music);
    string add_link(link_music);

    Music music{add_name, add_desc, add_gen, add_link , "0","" };
    auto id_inserat = storage_music.insert(music);
    cout << "[BD][M] Inserat.. id = " << id_inserat << endl;

}

bool check_login (char login_username[50], char login_pass[50]){

    string login_username_ck(login_username);
    string login_pass_ck (login_pass);

    auto login_usr = storage.get_all<User>(where((c(&User::user) == login_username_ck) and (c(&User::pass) == login_pass_ck)));
    cout << "login count = " << login_usr.size() << endl;
    return login_usr.size() >= 1;

}

// Varaibile
// Register
char register_name[50];
char register_username[50];
char register_pass[50];
char admin_state[50];
// Login
char login_username[50];
char login_pass[50];
// Music Add
char music_name[50];
char music_desc[50];
char music_gen[50];
char music_link[50];
// Vote Add
char vote_name[50];


/* programul */
int main ()
{
    struct sockaddr_in server;	/* structurile pentru server si clienti */
    struct sockaddr_in from;
    fd_set readfds;		/* multimea descriptorilor de citire */
    fd_set actfds;		/* multimea descriptorilor activi */
    struct timeval tv;		/* structura de timp pentru select() */
    int sd, client;		/* descriptori de socket */
    int optval=1; 			/* optiune folosita pentru setsockopt()*/
    int fd;			/* descriptor folosit pentru
				   parcurgerea listelor de descriptori */
    int nfds;			/* numarul maxim de descriptori */
    socklen_t len;			/* lungimea structurii sockaddr_in */

    // Baza de date
    printf("Baza de date: %s\n", sqlite3_libversion()); // Verifica vers.
    storage.sync_schema();
    storage_music.sync_schema();

    // Server
    /* creare socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server] Eroare la socket().\n");
        return errno;
    }

    /*setam pentru socket optiunea SO_REUSEADDR */
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

    /* pregatim structurile de date */
    bzero (&server, sizeof (server));

    /* umplem structura folosita de server */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server] Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 5) == -1)
    {
        perror ("[server] Eroare la listen().\n");
        return errno;
    }

    /* completam multimea de descriptori de citire */
    FD_ZERO (&actfds);		/* initial, multimea este vida */
    FD_SET (sd, &actfds);		/* includem in multime socketul creat */

    tv.tv_sec = 1;		/* se va astepta un timp de 1 sec. */
    tv.tv_usec = 0;

    /* valoarea maxima a descriptorilor folositi */
    nfds = sd;

    printf ("[server] Asteptam la portul %d...\n", PORT);
    fflush (stdout);

    /* servim in mod concurent clientii... */
    while (1)
    {
        /* ajustam multimea descriptorilor activi (efectiv utilizati) */
        bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

        /* apelul select() */
        if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
        {
            perror ("[server] Eroare la select().\n");
            return errno;
        }
        /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
        if (FD_ISSET (sd, &readfds))
        {
            /* pregatirea structurii client */
            len = sizeof (from);
            bzero (&from, sizeof (from));

            /* a venit un client, acceptam conexiunea */
            client = accept (sd, (struct sockaddr *) &from, &len);

            /* eroare la acceptarea conexiunii de la un client */
            if (client < 0)
            {
                perror ("[server] Eroare la accept().\n");
                continue;
            }

            if (nfds < client) /* ajusteaza valoarea maximului */
                nfds = client;

            /* includem in lista de descriptori activi si acest socket */
            FD_SET (client, &actfds);

            printf("[server] S-a [conectat] clientul cu descp. %d, de la adresa %s.\n",client, conv_addr (from));
            fflush (stdout);
        }

        //////////////////////////////
        /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
        for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
        {
            /* este un socket de citire pregatit? */
            if (fd != sd && FD_ISSET (fd, &readfds))
            {
                if (procesare(fd) == 10) {
                    printf("[server] S-a [deconectat] clientul cu descp. %d.\n", fd);
                    fflush(stdout);
                    close(fd);        /* inchidem conexiunea cu clientul */
                    FD_CLR (fd, &actfds);/* scoatem si din multime */
                }
            }
        }			/* for */
    }				/* while */
}				/* main */

/* realizeaza primirea si retrimiterea unui mesaj unui client */
int procesare(int fd)
{
    char buffer[100];		/* mesajul */
    int bytes;			/* numarul de octeti cititi/scrisi */
    char msg[100];		//mesajul primit de la client
    char msgrasp[100]=" ";        //mesaj de raspuns pentru client

    bytes = read (fd, msg, sizeof (buffer));
    if (bytes < 0)  {
        perror ("Eroare la read() de la client.\n");
        return 0;
    }

    // Conditia de iesire
    if (strcmp(msg,"quit") == 0) return 10;
    else printf ("[server]Mesajul a fost receptionat...%s\n", msg);

    // Inregistrare
    char * parti;
    bool name_ck = false,user_ck = false,pass_ck = false,
            user_login_ck = false, pass_login_ck = false,
            admin_ck = false;
    // Music
    bool music_name_ck = false, music_desc_ck = false,
            music_gen_ck = false, music_link_ck = false;

    // Vote
    bool vote_ck = false;

    char msg_aux[50];
    strcpy(msg_aux,msg);
    parti = strtok (msg_aux," ");
    while (parti != NULL) {
        // Register
        if (strcmp(parti,"add_database_name") == 0) name_ck = true;
        if (strcmp(parti,"add_database_user") == 0) user_ck = true;
        if (strcmp(parti,"add_database_pass") == 0) pass_ck = true;
        if (strcmp(parti, "add_database_admin") == 0) admin_ck = true;
        // Login
        if (strcmp(parti,"login_database_user") == 0) user_login_ck = true;
        if (strcmp(parti,"login_database_pass") == 0) pass_login_ck = true;
        // Music Add
        if (strcmp(parti,"add_music_name") == 0) music_name_ck = true;
        if (strcmp(parti,"add_music_desc") == 0) music_desc_ck = true;
        if (strcmp(parti,"add_music_gen") == 0) music_gen_ck = true;
        if (strcmp(parti,"add_music_link") == 0) music_link_ck = true;
        // Vote Add
        if (strcmp (parti,"add_vote") == 0) vote_ck = true;

        // Adaugam continutul in variabile
        // Register
        if (name_ck) strcpy(register_name, parti);
        if (user_ck) strcpy(register_username,parti);
        if (pass_ck) strcpy(register_pass,parti);
        if (admin_ck) strcpy(admin_state,parti);
        // Login
        if (user_login_ck) strcpy(login_username, parti);
        if (pass_login_ck) strcpy(login_pass, parti);
        // Music Add
        if (music_name_ck) strcpy(music_name, parti);
        if (music_desc_ck) strcpy(music_desc, parti);
        if (music_gen_ck) strcpy(music_gen, parti);
        if (music_link_ck) strcpy(music_link, parti);
        // Vote Add
        if (vote_ck) strcpy(vote_name,parti);

        parti = strtok (NULL, " ");

    } if (admin_ck) { // REGISTER
        printf("[register] Utilizator nou ! \n");
        printf("Numele este: %s\n", register_name);
        printf("Userul este: %s\n", register_username);
        printf("Parola este: %s\n", register_pass);

        /*adaugam la baza de date */
        add_row_register(register_name,register_username,register_pass,admin_state);

        /*curatam  */
        pass_ck = false; user_ck = false; name_ck = false; admin_ck = false;

        /*pregatim mesajul de raspuns */
        bzero(msgrasp,100);
        strcpy(msgrasp,"register_ok");
        printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
        if (bytes && write (fd, msgrasp, bytes) < 0) {
            perror ("[server] Eroare la write() catre client.\n");
            return 0;
        }
    } if (pass_login_ck) { // LOGIN
        printf("[login] Logare .. ! \n");
        printf("Userul este: %s\n", login_username);
        printf("Parola este: %s\n", login_pass);

        if (check_login (login_username,login_pass)){

            string login_username_ck(login_username);
            string login_pass_ck (login_pass);
            char client_name[50];
            char client_type[50];

            auto name_srv = storage.select(&User::name, where((c(&User::user) == login_username_ck) and (c(&User::pass) == login_pass_ck)));
            cout << name_srv.size() << endl;
            for(auto &i : name_srv) {
                strcpy(client_name,i.c_str()); }

            auto admin = storage.select(&User::admin, where((c(&User::user) == login_username_ck) and (c(&User::pass) == login_pass_ck)));
            for(auto &i1 : admin) {
                strcpy(client_type,i1.c_str());
                cout << "Type-ul este: " << i1 <<" ";
            }
            cout << endl;

            /*pregatim mesajul de raspuns */
            bzero(msgrasp,100);
            strcpy(msgrasp,"login_ok ");
            strcat(msgrasp,client_name);
            cout << msgrasp;
            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
            if (bytes && write (fd, msgrasp, bytes) < 0) {
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }

            /*pregatim mesajul de raspuns */
            bzero(msgrasp,100);
            strcpy(msgrasp,client_type);
            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
            if (bytes && write (fd, msgrasp, bytes) < 0) {
                perror ("[server] Eroare la write() catre client.\n");
                return 0; }

            /*curatam  */
            user_login_ck = false; pass_login_ck = false;

        } else {
            /*curatam  */
            user_login_ck = false; pass_login_ck = false;

            /*pregatim mesajul de raspuns */
            bzero(msgrasp,100);
            strcpy(msgrasp,"register_fail");
            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
            if (bytes && write (fd, msgrasp, bytes) < 0) {
                perror ("[server] Eroare la write() catre client.\n");
                return 0; }}
    } if (music_link_ck){ // MUSIC ADD
        printf("[M] Numele este: %s\n", music_name);
        printf("[M] Desc. este: %s\n", music_desc);
        printf("[M] Genul este: %s\n", music_gen);
        printf("[M] Link-ul este: %s\n", music_link);

        // Adaugam la baza de date
        add_row_music(music_name,music_desc,music_gen,music_link);

        /*pregatim mesajul de raspuns */
        bzero(msgrasp,100);
        strcpy(msgrasp,"music_add_ok");
        printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
        if (bytes && write (fd, msgrasp, bytes) < 0) {
            perror ("[server] Eroare la write() catre client.\n");
            return 0;
        }

        /*curatam  */
        music_name_ck = false; music_desc_ck = false; music_gen_ck = false; music_link_ck = false;
    } if (strcmp(msg,"list_music") == 0) { // LISTAM TABELUL
        /*pregatim mesajul de raspuns */
        bzero(msgrasp,100);

        auto music_count = storage_music.get_all<Music>();
        long n = music_count.size();
        // Scriem buffer-ul
        int aux = snprintf(buffer, sizeof(buffer), "%ld", n);
        char * num_string = buffer;
        strcpy(msgrasp,num_string);

        // Trimitem clientului
        printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
        if (bytes && write (fd, msgrasp, bytes) < 0) {
            perror ("[server] Eroare la write() catre client.\n");
            return 0;
        }

        // Trimitem coloanele
        for(auto &music : music_count) {
            if (bytes && write (fd, storage_music.dump(music).c_str(), 300) < 0) {
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
        }
        printf("TOTUL OK !\n");
    } if (vote_ck){ // ADD VOTE

        char msg_1[50];
        int ps=0;
        read (fd, msg_1,100);

        string user_session(msg_1);
        auto vote_ck_1 = storage.select(&User::vote, where(c(&User::user) == user_session));
        for(auto &l : vote_ck_1) { ps =  (int)l; }
        if (ps == 1) {

            // Modificam in baza de date
            string vote_name_string(vote_name);
            auto vote = storage_music.select(&Music::vote_num, where(c(&Music::name) == vote_name_string));
            int vote_number = 0;
            for (auto &j : vote) {
                vote_number = stoi(j);
            }

            vote_number++;
            cout << vote_number;
            string s = to_string(vote_number);

            storage_music.update_all(sqlite_orm::set(&Music::vote_num, s), where(c(&Music::name) == vote_name_string));

            /*pregatim mesajul de raspuns */
            bzero(msgrasp, 100);
            strcpy(msgrasp, "vote_add_ok");
            printf("[server]Trimitem mesajul inapoi...%s\n", msgrasp);
            if (bytes && write(fd, msgrasp, bytes) < 0) {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }
        } else {
            /*pregatim mesajul de raspuns */
            bzero(msgrasp, 100);
            strcpy(msgrasp, "vote_add_false");
            printf("[server]Trimitem mesajul inapoi...%s\n", msgrasp);
            if (bytes && write(fd, msgrasp, bytes) < 0) {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }
        }

        // Curatam
        vote_ck = false;
    } if (strcmp(msg,"add_comment") == 0) { // ADD COMMENT
        char comment_data[50];
        char comment_name[50];
        char comm_chunk_data_temp[50];
        string comm_chunk_data;

        read (fd, comment_data,100);
        read (fd, comment_name,100);

        strcat(comment_data,",");

        string comm_name(comment_name);
        auto comm = storage_music.select(&Music::comments, where(c(&Music::name) == comm_name));
        for(auto &k : comm) { comm_chunk_data =  k; }

        strcpy(comm_chunk_data_temp,comm_chunk_data.c_str());
        strcat(comm_chunk_data_temp,comment_data);

        string comm_data_full(comm_chunk_data_temp);
        storage_music.update_all( sqlite_orm::set(&Music::comments, comm_data_full) , where(c(&Music::name) == comm_name));

        cout <<"Este: " <<comm_chunk_data_temp << " " << comment_name;

    }if (strcmp(msg,"list_comments") == 0){ // LISTEAZA COMENTARIILE

        char comment_name[50];
        char comm_chunk_data_temp[50];
        string comm_chunk_data;
        read (fd, comment_name,100);

        string comm_name(comment_name);
        auto comm_2 = storage_music.select(&Music::comments, where(c(&Music::name) == comm_name));
        for(auto &p : comm_2) { comm_chunk_data =  p; }

        strcpy(comm_chunk_data_temp,comm_chunk_data.c_str());

        printf("[server]Trimitem mesajul inapoi...%s\n", comm_chunk_data_temp);
        if (bytes && write(fd, comm_chunk_data_temp, 300) < 0) {
            perror("[server] Eroare la write() catre client.\n");
            return 0;
        }

    }if (strcmp(msg,"list_music_name") == 0){ // ADMIN MUSIC LIST
        // Construim numarul de melodii
        auto music_count = storage_music.get_all<Music>();
        long n = music_count.size();
        int ret = snprintf(buffer, sizeof(buffer), "%ld", n);
        char * num_string = buffer;
        strcpy(msgrasp,num_string);

        // Trimitem clientului numarul de melodii
        printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
        if (bytes && write (fd, msgrasp, bytes) < 0) {
            perror ("[server] Eroare la write() catre client.\n");
            return 0;
        }

        // Trimitem numele melodiilor
        auto music = storage_music.select(&Music::name);
        for(auto &id : music) {
            if (bytes && write (fd, id.c_str(), 300) < 0) {
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
        }
    } if (strcmp(msg,"delete_music") == 0) { // ADMIN DELETE
        read (fd, msg, 100);
        string music_name_d(msg);
        storage_music.remove_all<Music>(where(c(&Music::name) == music_name_d));

    } if (strcmp(msg,"list_user_name") == 0) {
        auto music_count = storage.select(&User::user, where(c(&User::admin) == "user") );
        long n = music_count.size();
        int ret = snprintf(buffer, sizeof(buffer), "%ld", n);
        char * num_string = buffer;
        strcpy(msgrasp,num_string);

        // Trimitem clientului numarul de melodii
        printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
        if (bytes && write (fd, msgrasp, bytes) < 0) {
            perror ("[server] Eroare la write() catre client.\n");
            return 0;
        }

        // Trimitem numele utilizatoriulor
        auto user = storage.select(&User::user, where(c(&User::admin) == "user") );
        for(auto &id : user) {
            if (bytes && write (fd, id.c_str(), 300) < 0) {
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
        }
    } if (strcmp(msg,"restrict_user") == 0) { // ADMIN RESTRICT
        read (fd, msg, 100);
        string user_r(msg);
        storage.update_all( sqlite_orm::set(&User::vote, "0") , where(c(&User::user) == user_r));
    } if (strcmp(msg,"un-restrict_user") == 0) {
        read (fd, msg, 100);
        string user_r(msg);
        storage.update_all( sqlite_orm::set(&User::vote, "1") , where(c(&User::user) == user_r));
    }

    fflush(stdout);

    return bytes;
}