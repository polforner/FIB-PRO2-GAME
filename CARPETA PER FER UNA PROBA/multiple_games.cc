#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;


/*
Programa para ejecutar varias partidas del Pandemic concurrentemente. Hecho por Llinot#9521 y Rubikvng#1349
Para usarlo, compiladlo y ejecutadlo de la siguiente forma: 
    ./multiple_games.exe Player1 Player2 Player3 Player 4
El programa despues preguntara cuantas partidas se quieren ejecutar y empezara a funcionar automaticamente.
Cuando acabe, mostrara por pantalla el numero de partidas ganadas de cada jugador y su winrate

Nota: si se ejecuta el juego con el mismo Player varias veces, se sumaran todas sus victorias y se mostraran en una sola linea
*/
int main (int argc, char *argv[]) {
    cout << "numero de partides: ";
    int n;
    cin >> n;
    cout << endl;

    cout << "starting games" << endl;

    for (int i=0; i<n; ++i) {
        int pid = fork();
        if (pid == 0) {
            cout << "playing game " << i << endl;
            char buf[10];
            srand(getpid());
            sprintf(buf, "%i",rand()%1000000 + i);
            char buf2[10];
            sprintf(buf2, "%i.txt", i);
            close(2);
            open(buf2, O_WRONLY | O_TRUNC | O_CREAT, 0666);
            execlp("./Game", "./Game", argv[1], argv[2], argv[3], argv[4], "-s", buf, "-i", "default.cnf", "-o", "default.out", NULL);            
            exit(0);
        }
    }
    
    vector<int> wins (4, 0);
    int conta = 1;
    while(waitpid(-1,NULL,0) > 0) {cout << conta << " games have finished" << endl; ++conta;}
    cout << endl;
    for (int i = 0; i < n; ++i) {
        char buf2[10];
        sprintf(buf2, "%i.txt", i);
        cout << "analizing game " << i << endl;
        int fdpipe1[2];
        pipe(fdpipe1);
        int cat = fork();
        if (cat == 0) {
            close(fdpipe1[0]);
            dup2(fdpipe1[1], 1);
            execlp("cat", "cat", buf2, NULL);
        }       
        int fdpipe2[2];
        pipe(fdpipe2);
        int grep = fork();
        if (grep == 0) {
            dup2(fdpipe1[0], 0);
            dup2(fdpipe2[1], 1);
            close(fdpipe2[0]);    
            close(fdpipe1[1]);
            execlp("grep", "grep", "top", NULL);
        }
        close(fdpipe1[0]);
        close(fdpipe1[1]);
        close(fdpipe2[1]);
        char basura[16];
        read(fdpipe2[0], basura, 16);
        char c; 
        string s;
        while(read(fdpipe2[0], &c, 1) > 0 and c != ' ') s.push_back(c);
        if (s == argv[1]) ++wins[0];
        else if (s == argv[2]) ++wins[1];
        else if (s == argv[3]) ++wins[2];
        else if (s == argv[4]) ++wins[3];
        remove(buf2);
    }
    while (waitpid(-1,NULL,0) > 0);
    cout << endl;
    for (int i = 0; i < 4; ++i) {
        printf("%s: %i wins\n", argv[i + 1], wins[i]);
    }
    cout << endl;
    for (int i=0; i<4; ++i) {
        cout << argv[i+1] << " has won " << double(100.0*wins[i]/n) << "% of the games" << endl;
    }
}