#include <iostream>
#include<bits/stdc++.h>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include <time.h>
#include<vector>
using namespace std;


ifstream fin("input_phase2.txt");
ofstream fout("outputPhase2.txt");

char M[300][4];
char buffer[40];
char IR[4];
char R[4];
int IC;
int C;
int SI;
int PI;
int TI;
int PTR;
bool breakFlag;
vector<int> check_rand;


struct PCB{
    int job_id;
    int TTL;
    int TLL;
    int TTC;
    int LLC;

    void setPCB(int id, int ttl, int tll){
        job_id = id;
        TTL = ttl;
        TLL = tll;
        TTC = 0;
        LLC = 0;
    }
};

PCB pcb;


string error[9] = {"No Error", "Out of Data", "Line Limit Exceeded", "Time Limit Exceeded",
    "Operation Code Error", "Operand Error", "Invalid Page Fault","Time Limit Exceeded with opcode error","Time Limit Exceeded with operand error"};


void INIT();
void READ(int RA);
void WRITE(int RA);
int ADDRESSMAP(int VA);
void EXECUTE_USER_PROGRAM();
void STARTEXECUTION();
int ALLOCATE();
void LOAD();
void SIMULATION();

void INIT(){
    memset(M, '\0', 1200);
    memset(IR, '\0', 4);
    memset(R, '\0', 4);
    C = 0;
    SI = 0;
    PI = 0;
    TI = 0;
    breakFlag = false;
}

void TERMINATE(int EM){
     fout << endl;
    if(EM == 0){
        fout << "Job id -> " << pcb.job_id << " terminated normally. " << error[EM];
    }
    else{
        fout << "Job id -> " << pcb.job_id << " terminated abnormally due to " << error[EM] << endl;
        fout << "IC="<<IC<<", IR="<<IR<<", TTC="<<pcb.TTC<<", LLC="<<pcb.LLC;
    }
    fout<<endl<<endl<<endl<<endl;
}

void READ(int RA){
    fin.getline(buffer, 41);

    char temp[5];
    memset(temp, '\0', 5);
    memcpy(temp,buffer,4);

    if(!strcmp(temp, "$END")){
        TERMINATE(1);
        breakFlag = true;
    }
    else{
        strcpy(M[RA],buffer);
    }
}

void WRITE(int RA){
    if(pcb.LLC+1 > pcb.TLL){
        TERMINATE(2);
        breakFlag = true;
    }
    else{
        string str;
        //char str[40];
        int k = 0;
        for(int i=RA; i<(RA+10); i++){
            for(int j=0; j<4; j++)
                str += M[i][j];
        }
        fout << str<<endl ;
        pcb.LLC++;
    }
}

int MOS(int RA = 0){
    if(TI == 0){
        if(SI != 0){
            switch(SI){
                case 1:
                    READ(RA);
                    break;
                case 2:
                    WRITE(RA);
                    break;
                case 3:
                    TERMINATE(0);
                    breakFlag = true;
                    break;
                default:
                    cout<<"Error with SI."<<endl;
            }
            SI = 0;
        }
        else if(PI != 0){
            switch(PI){
                case 1:
                    TERMINATE(4);
                    breakFlag = true;
                    break;
                case 2:
                    TERMINATE(5);
                    breakFlag = true;
                    break;
                case 3:
                    PI = 0;
                    //Page Fault checking
                    char temp[3];
                    memset(temp,'\0',3);
                    memcpy(temp, IR, 2);

                    if(!strcmp(temp,"GD") || !strcmp(temp,"SR")){
                        int m;
                        do{
                            m = ALLOCATE();
                        }while(M[m*10][0]!='\0');

                        int currPTR = PTR;
                        while(M[currPTR][0]!='*')
                            currPTR++;
                        itoa(m, M[currPTR], 10);

                        cout << "Valid Page Fault, page frame = " << m << endl;
                        cout << "PTR = " << PTR << endl;
                        cout<<endl;

                        if(pcb.TTC+1 > pcb.TTL){
                            TI = 2;
                            PI = 3;
                            MOS();
                            break;
                        }
                        return 1;
                    }
                    else if(!strcmp(temp,"PD") || !strcmp(temp,"LR") || !strcmp(temp,"H") || !strcmp(temp,"CR") || !strcmp(temp,"BT")){
                        TERMINATE(6);
                        breakFlag = true;

                        if(pcb.TTC+1 > pcb.TTL){
                            TI = 2;
                            PI = 3;
                            MOS();
                            break;
                        }
                    }
                    else{
                        PI = 1;
                        MOS();
                    }
                    return 0;
                default:
                    cout<<"Error with PI."<<endl;
            }
            PI = 0;
        }
    }
    else{
        if(SI != 0){
            switch(SI){
                case 1:
                    TERMINATE(3);
                    breakFlag = true;
                    break;
                case 2:
                    WRITE(RA);
                    if(!breakFlag) TERMINATE(3);
                    break;
                case 3:
                    TERMINATE(0);
                    breakFlag = true;
                    break;
                default:
                    cout<<"Error with SI."<<endl;
            }
            SI = 0;
        }
        else if(PI != 0){
            switch(PI){
                case 1:
                    TERMINATE(7);
                    breakFlag = true;
                    break;
                case 2:
                    TERMINATE(8);
                    breakFlag = true;
                    break;
                case 3:
                    TERMINATE(3);
                    breakFlag = true;
                    break;
                default:
                    cout<<"Error with PI."<<endl;
            }
            PI = 0;
        }
    }

    return 0;
}

void SIMULATION(){
    pcb.TTC++;
    if(pcb.TTC+1> pcb.TTL){
            TI = 2;
        }
}

int ADDRESSMAP(int VA){
    cout<<"va-> "<<VA<<endl;
    if(0 <= VA && VA < 100){
        int pte = PTR + VA/10;
        cout<<"pte-> "<<pte<<endl;
        if(M[pte][0] == '*'){
            PI = 3;
            return 0;
        }
        cout << "In ADDRESSMAP, VA = " << VA << ", pte = " << pte << ", M[pte] = " << M[pte] << endl;
        return atoi(M[pte])*10 + VA%10;
    }
    PI = 2;
    return 0;
}

void EXECUTE_USER_PROGRAM(){
    char temp[3], operand[2];
    int locIR, RA;

    while(true){
        if(breakFlag) break;

        RA = ADDRESSMAP(IC);
        if(PI != 0){
            if(MOS()){
                continue;
            }
            break;
        }
        cout << "IC = " << IC << ", RA = " << RA << endl;

        memcpy(IR, M[RA], 4);
        cout<<"IR1"<<IR;
        IC += 1;

        memset(temp,'\0',3);
        memcpy(temp,IR,2);
        for(int i=0; i<2; i++){
            if(!((47 < IR[i+2] && IR[i+2] < 58) || IR[i+2] == 0)){
                PI = 2;
                break;
            }
            operand[i] = IR[i+2];
        }

        if(PI != 0){
            MOS();
            break;
        }

        locIR = atoi(operand);

        RA = ADDRESSMAP(locIR);
        if(PI != 0){
            if(MOS()){
                IC--;
                continue;
            }
            break;
        }

        cout << "IC = " << IC << ", RA = " << RA << ", IR = " << IR << endl;


        if(!strcmp(temp,"LR")){
            SIMULATION();
            cout<<endl;
            for(int i=0; i<4; i++){
                R[i] = M[RA][i];
            }
          

        }
        else if(!strcmp(temp,"SR")){
                SIMULATION();
                for(int i=0; i<4; i++){
                M[RA][i] = R[i];
            }
          

        }
        else if(!strcmp(temp,"CR")){
            SIMULATION();
            if(!strcmp(R,M[RA]))
                C = 1;
            else
                C = 0;

        }
        else if(!strcmp(temp,"BT")){
            SIMULATION();
            if(C == 1)
                IC = RA;

        }
        else if(!strcmp(temp,"GD")){
            SI = 1;
            SIMULATION();
            MOS(RA);

        }
        else if(!strcmp(temp,"PD")){
            SI = 2;
            SIMULATION();
            MOS(RA);

        }
        else if(!strcmp(temp,"H")){
            SI = 3;
            SIMULATION();
            MOS();

            break;
        }
        else{
            PI = 1;
            MOS();
            break;
        }
        memset(IR, '\0', 4);
    }
    for(int i=0; i<300; i++){
                    cout<<i<<" ";
                for(int j=0 ; j<4; j++){
                    cout<<M[i][j]<<" ";
                }
                cout<<endl;
            }
}

void STARTEXECUTION(){
    IC = 0;
    EXECUTE_USER_PROGRAM();
}

int ALLOCATE(){
    int random=rand() % 30;
    if(check_rand.size() == 0){
        check_rand.push_back(random);
        return check_rand[0];
    }
    for(int i=0;i<check_rand.size();i++){
            if(random==check_rand[i]){
                return ALLOCATE();
            }
    }
    check_rand.push_back(random);

    return check_rand[check_rand.size()-1];

}

void LOAD(){
    int m;
    int currPTR;
    char temp[5];
    memset(buffer, '\0', 40);

    while(!fin.eof()){
        fin.getline(buffer,41);
        memset(temp, '\0', 5);
        memcpy(temp,buffer,4);

        if(!strcmp(temp,"$AMJ")){  
            INIT();

            int jobId, TTL, TLL;
            memcpy(temp, buffer+4, 4);
            jobId = atoi(temp);
            memcpy(temp, buffer+8, 4);
            TTL = atoi(temp);
            memcpy(temp, buffer+12, 4);
            TLL = atoi(temp);
            pcb.setPCB(jobId, TTL, TLL);

            PTR = ALLOCATE()*10;
            memset(M[PTR], '*', 40);
            currPTR = PTR;
        }
        else if(!strcmp(temp,"$DTA")){
                cout<<"Now dta starts";
            STARTEXECUTION();
        }
        else if(!strcmp(temp,"$END")){
            continue;
        }
        else{
            if(breakFlag) continue;

            do{
                m = ALLOCATE();
                cout<<"m"<<m;

            }while(M[m*10][0]!='\0');
            itoa(m, M[currPTR], 10);
            currPTR++;

            strcpy(M[m*10],buffer);

            cout << " PTR = " << PTR << endl;

            cout<<endl;
        }
    }
}

int main(){
    LOAD();
    fin.close();
    fout.close();
    return 0;
}
