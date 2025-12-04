#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <string>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include <term.h>
#include <time.h>

#include "plansza.h"
#include "gra.h"

int main(){
    char o,p,n;
    int a,b;

    Plansza T1;
    Plansza T2;

    Gra G1;

    cout<<"Chcesz kontynuowac poprzednia gre czy zaczac nowa? Wpisz P dla poprzedniej lub N dla nowej: ";
    p=toupper(cin.get());
    cout<<"\n";
    if(p=='P'){
        G1.wczytajT(T1.T,T2.T,T1.P,T2.P);
    }
    else{
        T1.ZERUJ();
        T2.ZERUJ();
        srand(time(NULL));
        T1.los4(T1.T);
        T2.los4(T2.T);
        for(int i=0; i<2; i++){
            T1.los3(T1.T);
            T2.los3(T2.T);
        }
        for(int i=0; i<3; i++){
            T1.los2(T1.T);
            T2.los2(T2.T);
        }
        for(int i=0; i<4; i++){
            T1.los1(T1.T);
            T2.los1(T2.T);
        }
        G1.ID();
    }

        cout << "\033[2J\033[1;1H";
        cin.ignore();
        cout<< "Niech przygotuje sie gracz1"<<endl;
        cin.get();

        o='T';
    do{
        cout<<"Tura Gracza: ";
        G1.Nazwa1();
        cout<<endl<<"Plansza strzalow:"<<endl;
        T1.piszT(T1.P);
        cout<<endl<<"Plansza statkow:"<<endl;
        T1.piszT(T1.T);
        G1.Strzal(T2.T,T1.P);

        cin.ignore();
        cout<< "Niech przygotuje sie kolejny gracz"<<endl;
        cin.get();
        cout << "\033[2J\033[1;1H";

        cout<<"Tura Gracza: ";
        G1.Nazwa2();
        cout<<endl<<"Plansza strzalow:"<<endl;
        T2.piszT(T2.P);
        cout<<endl<<"Plansza statkow:"<<endl;
        T2.piszT(T2.T);
        G1.Strzal(T1.T,T2.P);

        a=T1.SprawdzT(T1.T);
        b=T2.SprawdzT(T2.T);
        if(a==1&b==2){
            cout<<"Wygral "; G1.Nazwa1();
            o='N';
        }
        else if(a==2&b==1){
            cout<<"Wygral "; G1.Nazwa2();
            o='N';
        }

        G1.zapiszT(T1.T,T2.T,T1.P,T2.P);
        cout<<"+++Autosave+++"<<endl;

        cin.ignore();
        cout<< "Niech przygotuje sie kolejny gracz"<<endl;
        cin.get();
        cout << "\033[2J\033[1;1H";


    }while(o=='T');
return 0;
}