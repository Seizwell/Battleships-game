#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <string>
#include <ctime>
#include <fstream>
#include "plansza.h"

using namespace std;

void Plansza::ZERUJ()
{
    for(int i=0; i<maks; i++){
        for(int j=0; j<maks; j++){
            T[i][j]=0;
            P[i][j]=0;
        }
    }
}


void Plansza::piszT()
{
    for(int i=1; i<11; i++){
        for(int j=1; j<11; j++){
            switch(T[i][j]){
                case 0:
                    cout.width(2);
                    cout<<".";//nic
                    break;
                case 1:
                    cout.width(2);
                    cout<<"x";//statek
                    break;
                case 2:
                    cout.width(2);
                    cout<<"x";
                    break;
                case 3:
                    cout.width(2);
                    cout<<"x";
                    break;
                case 4:
                    cout.width(2);
                    cout<<"x";
                    break;
                case 5:
                    cout.width(2);
                    cout<<".";
                    break;
                case 7:
                    cout.width(2);
                    cout<<"~";//pudlo
                    break;
                case 8:
                    cout.width(2);
                    cout<<"o";//trafiony
                    break;
            }
        }
        cout<<"\n";
    }
    cout<<"\n\n";
}

void Plansza::piszP()
{
    for(int i=1; i<11; i++){
        for(int j=1; j<11; j++){
            switch(P[i][j]){
                case 0:
                    cout.width(2);
                    cout<<".";//nic
                    break;
                case 1:
                    cout.width(2);
                    cout<<"x";//statek
                    break;
                case 2:
                    cout.width(2);
                    cout<<"x";
                    break;
                case 3:
                    cout.width(2);
                    cout<<"x";
                    break;
                case 4:
                    cout.width(2);
                    cout<<"x";
                    break;
                case 5:
                    cout.width(2);
                    cout<<".";
                    break;
                case 7:
                    cout.width(2);
                    cout<<"~";//pudlo
                    break;
                case 8:
                    cout.width(2);
                    cout<<"o";//trafiony
                    break;
            }
        }
        cout<<"\n";
    }
    cout<<"\n\n";
}

void Plansza::los4()
{
    int k=0+rand()%(1-0+1);
    int x=0;
    int y=0;
    if(k==0){
        x=1+rand()%(7-1+1);
        y=1+rand()%(10-1+1);
        for(int i=x-1; i<=x+4; i++){
            for(int j=y-1; j<=y+1; j++){
                T[i][j]=5;
            }
        }
        for(int i=0; i<4; i++){
            T[x+i][y]=4;
        }
    }
    else{
        x=1+rand()%(10-1+1);
        y=1+rand()%(7-1+1);
        for(int i=x-1; i<=x+1; i++){
            for(int j=y-1; j<=y+4; j++){
                T[i][j]=5;
            }
        }
        for(int i=0; i<4; i++){
            T[x][y+i]=4;
        }
    }
}

void Plansza::los3()
{
    int k=0+rand()%(1-0+1);
    int a;
    if(k==0){
        do{
            int x=1+rand()%(8-1+1);
            int y=1+rand()%(10-1+1);
            for(int i=x-1; i<=x+3; i++){
                for(int j=y-1; j<=y+1; j++){
                    if(!(T[x][y]==5)&!(T[x+1][y]==5)&!(T[x+1][y]==4)&!(T[x+2][y]==5)&!(T[x+2][y]==4)&!(T[i][j]==4)&!(T[i][j]==3)){
                        a=1;
                    }
                    else{
                        a=0;
                        j=y+5;
                    }
                }
            }
            if(a==1){
                for(int i=x-1; i<=x+3; i++){
                    for(int j=y-1; j<=y+1; j++){
                        T[i][j]=5;
                    }
                }
                for(int i=0; i<3; i++){
                    T[x+i][y]=3;
                }
            }
        }while(a==0);
    }
    else{
        do{
        int x=1+rand()%(10-1+1);
        int y=1+rand()%(8-1+1);
        for(int i=x-1; i<=x+1; i++){
            for(int j=y-1; j<=y+3; j++){
                if(!(T[x][y]==5)&!(T[x][y+1]==5)&!(T[x][y+1]==4)&!(T[x][y+2]==5)&!(T[x][y+2]==4)&!(T[i][j]==4)&!(T[i][j]==3)){
                        a=1;
                    }
                    else{
                        a=0;
                        j=y+5;
                    }
                }
            }
            if(a==1){
                for(int i=x-1; i<=x+1; i++){
                    for(int j=y-1; j<=y+3; j++){
                        T[i][j]=5;
                    }
                }
                for(int i=0; i<3; i++){
                    T[x][y+i]=3;
                }
            }
        }while(a==0);
    }
}

void Plansza::los2()
{
    int k=0+rand()%(1-0+1);
    int a;
    if(k==0){
        do{
            int x=1+rand()%(9-1+1);
            int y=1+rand()%(10-1+1);
            for(int i=x-1; i<=x+2; i++){
                for(int j=y-1; j<=y+1; j++){
                    if(!(T[x][y]==5)&!(T[x+1][y]==5)&!(T[x+1][y]==4)&!(T[x+2][y]==5)&!(T[x+2][y]==4)&!(T[i][j]==4)&!(T[i][j]==3)&!(T[i+1][j]==3)&!(T[i+2][j]==3)&!(T[i][j]==2)){
                        a=1;
                    }
                    else{
                        a=0;
                        j=y+5;
                    }
                }
            }
            if(a==1){
                for(int i=x-1; i<=x+2; i++){
                    for(int j=y-1; j<=y+1; j++){
                        T[i][j]=5;
                    }
                }
                for(int i=0; i<2; i++){
                    T[x+i][y]=2;
                }
            }
        }while(a==0);
    }
    else{
        do{
        int x=1+rand()%(10-1+1);
        int y=1+rand()%(9-1+1);
        for(int i=x-1; i<=x+1; i++){
            for(int j=y-1; j<=y+2; j++){
                if(!(T[x][y]==5)&!(T[x][y+1]==5)&!(T[x][y+1]==4)&!(T[x][y+2]==5)&!(T[x][y+2]==4)&!(T[i][j]==4)&!(T[i][j]==3)&!(T[i][j+1]==3)&!(T[i][j+2]==3)&!(T[i][j]==2)){
                        a=1;
                    }
                    else{
                        a=0;
                        j=y+5;
                    }
                }
            }
            if(a==1){
                for(int i=x-1; i<=x+1; i++){
                    for(int j=y-1; j<=y+2; j++){
                        T[i][j]=5;
                    }
                }
                for(int i=0; i<2; i++){
                    T[x][y+i]=2;
                }
            }
        }while(a==0);
    }
}

void Plansza::los1()
{
    int k=0+rand()%(1-0+1);
    int a;
    if(k==0){
        do{
            int x=1+rand()%(10-1+1);
            int y=1+rand()%(10-1+1);
            for(int i=x-1; i<=x+1; i++){
                for(int j=y-1; j<=y+1; j++){
                    if(!(T[x][y]==5)&!(T[x+1][y]==5)&!(T[x+1][y]==4)&!(T[x+2][y]==5)&!(T[x+2][y]==4)&!(T[i][j]==4)&!(T[i][j]==3)&!(T[i+1][j]==3)&!(T[i+2][j]==3)&!(T[i][j]==2)&!(T[i+1][j]==2)&!(T[i+2][j]==2)&!(T[i][j]==1)){
                        a=1;
                    }
                    else{
                        a=0;
                        j=y+5;
                    }
                }
            }
            if(a==1){
                for(int i=x-1; i<=x+1; i++){
                    for(int j=y-1; j<=y+1; j++){
                        T[i][j]=5;
                    }
                }
                for(int i=0; i<1; i++){
                    T[x+i][y]=1;
                }
            }
        }while(a==0);
    }
    else{
        do{
        int x=1+rand()%(10-1+1);
        int y=1+rand()%(10-1+1);
        for(int i=x-1; i<=x+1; i++){
            for(int j=y-1; j<=y+1; j++){
                if(!(T[x][y]==5)&!(T[x][y+1]==5)&!(T[x][y+1]==4)&!(T[x][y+2]==5)&!(T[x][y+2]==4)&!(T[i][j]==4)&!(T[i][j]==3)&!(T[i][j+1]==3)&!(T[i][j+2]==3)&!(T[i][j]==2)&!(T[i][j+1]==2)&!(T[i][j+2]==2)&!(T[i][j]==1)){
                        a=1;
                    }
                    else{
                        a=0;
                        j=y+5;
                    }
                }
            }
            if(a==1){
                for(int i=x-1; i<=x+1; i++){
                    for(int j=y-1; j<=y+1; j++){
                        T[i][j]=5;
                    }
                }
                for(int i=0; i<1; i++){
                    T[x][y+i]=1;
                }
            }
        }while(a==0);
    }
}

bool Plansza::SprawdzT()
{
    for(int x=0; x<maks; x++){
        for(int y=0; y<maks; y++){
            if(T[x][y]==1||T[x][y]==2||T[x][y]==3||T[x][y]==4){
                return false;
            }
        }
    }
    return true;
}
