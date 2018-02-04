# TopMusic

# Descriere
Proiectul isi propune realizarea unui sistem de management
al unui top muzical format cu ajutorul contributiei utilizatorilor si administratorilor.
Topul contine functii de filtrare, adaugare si comentarii
pe baza melodiilor curente.

# Tehnologiile utilizate
TCP - Este unul dintre principalele protocoale din suita de protocoale Internet.
Este folosit in implementarea initiala a retelei si este completat de Internet Protocol
(IP). TCP ofera informatii fiabile, ordonate, si de livrare verificat-eroare a
unui flux de octeti intre aplicatiile care ruleaza pe gazde care comunica printr-o
retea IP.

IP - Protocolul este folosit pentru a furniza o adresa unica si de a identifica
in mod unic legatura dintre client si server. Versiunea folosita in cadrul proiectului
este IPv4, adresa IP fiind reprezentata pe 32 de biti.

Socket-uri - Clientul intodeauna creaza un socket pentru a initia conversatia iar
pentru aceasta trebuie sa cunoasca serverul caruia adreseaza cererea. Serverul
trebuies sa fie pregatit pentru a receptiona cererile. In momentul receptionarii
mesajului creaza un socket pe partea serverului care faciliteaza deservirea clientului.
Atat pe partea de server cat si pe parte de client se utilizeaza un obiect
de tip Socket pentru comunicare.

# Server - Structura serverului
Arhitectura interna este data de urmatoarea secventa de primitve socket() →
bind() → listen() → Loop → recv() → send() → close() → close()

# Client - Descrierea tehnologiilor
Qt - Pentru a furniza o interfata grafica cat mai bine dezvoltata am ales aceasta
librarie. Ofera o biblioteca cu elemente grafice pentru interfata dorita, care pot
rula pe mai multe sisteme de operare si sisteme hardwere. Va fi folosita in conturarea
sistemului de logare, pentru creearea panoului de inregistrare, desenarea
listei cu topuri si a butoanelor de sortare si adaugare.

# Arhitectura aplicatiei
Diagrama UML reprezentativa arhitecturii aplicatiei pe partea de client:

![alt text](https://i.imgur.com/Z0CWGF4.png)

# Bibliografie
1. Atelier de programare in retele de calculatoare S. Buraga, G. Ciobanu. Polirom,
Iasi, 2001
2. Programare C si C++ pentru Linux D. Acostchioaie Polirom, Iasi, 2002
3. Securitatea sistemelor Linux D. Acostchioaie Polirom, Iasi, 2003

# Preview
![alt text](https://i.imgur.com/eVciVRV.png)
![alt text](https://i.imgur.com/I1Dy6wC.png)

