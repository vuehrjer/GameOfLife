//
//  main.cpp
//  Game of Life
//
//  Created by Marius Führer on 16.09.19.
//  Copyright © 2019 Marius Führer. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <omp.h>

using namespace std;

void translateArea(vector<vector<char> > vectorRaw, vector<vector<int> >& vectorNew)
{
    for(int i = 0; i < vectorRaw.size(); i++){
        vector<int> v;
        for(int j = 0; j < vectorRaw[i].size(); j++){
            if(vectorRaw[i][j] == 'x'){
                v.push_back(1);
            }
            else {
                v.push_back(0);
            }
            
        }
        vectorNew.push_back(v);
    }
}

void convertStringToVector(string sline, vector<char>& vec)
{
    for(int i = 0; i < sline.length()-1; i++){
        vec.push_back(sline.at(i));
    }
}
void revertArea(vector<vector<char> >& vectorNew, vector<vector<int> > vectorRaw)
{
    for(int i = 0; i < vectorRaw.size(); i++){
        vector<char> v;
        for(int j = 0; j < vectorRaw[i].size(); j++){
            if(vectorRaw[i][j] == 1){
                v.push_back('x');
            }
            else {
                v.push_back('.');
            }
            
        }
        vectorNew.push_back(v);
    }
}

vector<vector<char> > readFile(string filename, string& arg, string& sline, int& iarg, int& isline)
{
    
    vector<vector<char> > c;
    vector<char> v;
    ifstream myfile(filename);
    
    if(myfile.is_open())
    {
        //string sline;
        //string arg;
        getline(myfile, sline);
        string::size_type pos = sline.find(',');
        if(sline.npos != pos)
        {
            arg = sline.substr(pos + 1);
            sline = sline.substr(0, pos);
            iarg = stoi(arg);
            isline = stoi(sline);
            cout << arg << endl;
            cout << sline << endl;
            
            while(getline(myfile,sline))
            {
                vector<char> vector;
                
                convertStringToVector(sline, vector);
                c.push_back(vector);
            }
            
        }
        myfile.close();
    }
    return c;
}
template <class T>
void printVectorOfVectors(vector<vector<T> >& c)
{
    for(int i = 0; i < c.size(); i++){
        for(int j = 0; j < c[i].size(); j++){
            cout << c[i][j];
        }
        cout << endl;
    }
}

vector<vector<int> > nextGeneration(vector<vector<int> > firstGeneration, int iarg, int isline)
{
    vector<vector<int> > nextGeneration;
    for(int i = 0; i < firstGeneration.size(); i++){
        vector<int> v;
        for(int j = 0; j < firstGeneration[i].size(); j++)
        {
            v.push_back(0);
        }
        nextGeneration.push_back(v);
    }
    
    omp_set_dynamic(0);
    omp_set_num_threads(8);

    #pragma omp parallel for
    for(int i = 0; i < firstGeneration.size(); i++)
    {
        for(int j=0; j < firstGeneration[i].size(); j++)
        {
            int aliveNeighbors = 0;
            for(int a = -1; a <= 1; a++){
                for(int b = -1; b <= 1; b++){
                    
                    aliveNeighbors += firstGeneration[(i+a+iarg) % iarg][(j+b+isline)%isline];
                    
                }
            }
            aliveNeighbors -= firstGeneration[i][j];
            if((firstGeneration[i][j] == 1) && (aliveNeighbors < 2))
            {
                nextGeneration[i][j] = 0;
            }
            else if((firstGeneration[i][j] == 1) && (aliveNeighbors > 3))
            {
                nextGeneration[i][j] = 0;
            }
            else if((firstGeneration[i][j] == 0) && (aliveNeighbors == 3))
            {
                nextGeneration[i][j] = 1;
            }
            else{
                nextGeneration[i][j] = firstGeneration[i][j];
            }
        }
        
    }
    return nextGeneration;
}

string formatTime(std::chrono::steady_clock::duration time)
{
    std::chrono::hours h = std::chrono::duration_cast<std::chrono::hours>(time);
    time -= h;
    std::chrono::minutes m = std::chrono::duration_cast<std::chrono::minutes>(time);
    time -= m;
    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(time);
    time -= s;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(time);
    std::stringstream ss;
    ss << setfill('0') << setw(2) << h.count() << ":" << setw(2) << m.count() << ":" << setw(2) << s.count() << "." << setw(3) << ms.count();
    return ss.str();
}

vector<vector<int> > circleOfLife(vector<vector<int> >& gameBoard, int arg, int sline, int generations)
{
    
    {
        for(int i= 0; i < generations; i++)
        {
            gameBoard = nextGeneration(gameBoard, arg, sline);
        }
        return gameBoard;
        }
}

void writeBoardToFile(vector<vector<char> > vector, string filename)
{
    ofstream outfile(filename, ios::out | ios::trunc);
    if (outfile.is_open()) {
        for(int i = 0; i < vector.size(); i++)
        {
            for(int j = 0; j < vector[i].size() ; j++)
            {
                outfile << vector[i][j];
            }
            //outfile << endl;
        }
        outfile.close();

    }
    else{
        cout << "Error: File not open";
    }
}

int main(int argc, const char * argv[]) {
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now();
    if (argc == 1)
    {
        std::cout << "\nCommand Line Options missing" << '\n';
        return 0;
    }
    
    int generations = 0;
    bool measure = false;
    string outputfile;
    string sourcefile;
    string mode;
    
    if (argc > 2)
    {
        for (int i = 0; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--load")
            {
                if (i < argc) sourcefile = argv[++i];
            }
            else if (arg == "--save")
            {
                if (i < argc) outputfile = argv[++i];
            }
            else if (arg == "--generations")
            {
                if (i < argc) generations = stoi(argv[++i]);
            }
            else if (arg == "--measure")
            {
                measure = true;
            }
            else if (arg == "--mode")
            {
            }
        }
    }
    for(int i = 1; i < argc; i++)
    {
        cout << argv[i] << endl;
    }

    string arg;
    string sline;
    int iarg = 0;
    int isline = 0;
    vector<vector<int> > initialStateInNumbers;
    vector<vector<int> > nextGen;
    vector<vector<char> > endState;
    vector<vector<char> > initialState = readFile(sourcefile, arg, sline, iarg, isline);
    translateArea(initialState, initialStateInNumbers);
    auto t2 = Clock::now();
    //nextGen = nextGeneration(initialStateInNumbers, iarg, isline);
    //printVectorOfVectors<int>(initialStateInNumbers);
    vector<vector<int> > endStateInNumbers = circleOfLife(initialStateInNumbers, iarg, isline, generations);
    auto t3 = Clock::now();
    revertArea(endState, endStateInNumbers);
    //printVectorOfVectors<int>(endStateInNumbers);
    writeBoardToFile(endState, outputfile);
    //printVectorOfVectors<int>(nextGen);
    auto t4 = Clock::now();
    string timeInit = formatTime(t2-t1);
    string timeKernel = formatTime(t3-t2);
    string timeEnd = formatTime(t4-t3);
    if(measure == true)
    {
        cout << timeInit << "; " << timeKernel << "; " << timeEnd << ";" << endl;
    }
    return 0;
}
