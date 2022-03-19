#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;

void Partitioner::parseInput(fstream &inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);
    // Set up whole circuit
    while (inFile >> str)
    {
        if (str == "NET")
        {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName));
            _netName2Id[netName] = netId;
            while (inFile >> cellName)
            {
                if (cellName == ";")
                {
                    tmpCellName = "";
                    break;
                }
                else
                {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0)
                    {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                    }
                    // an existed cell
                    else
                    {
                        if (cellName != tmpCellName)
                        {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

void Partitioner::firstCut()
{
    bool side = 0; // 0 for A, 1 for B;
    // divide the cells first.
    for (int i = 0; i < _cellArray.size(); i++)
    {
        _cellArray[i]->setPart(side);
        _partSize[side]++;
        side = !side;
    }
    // traverse every net to calculate the initial cutsize.
    for (int i = 0; i < _netArray.size(); i++)
    {
        vector<int> cellList = _netArray[i]->getCellList();
        int a_count = 0, b_count = 0;
        for (int j = 0; j < cellList.size(); j++)
        {
            if (_cellArray[cellList[j]]->getPart() == 0)
            {
                a_count++;
            }
            else
            {
                b_count++;
            }
        }
        if (a_count > 0 && b_count > 0)
        {
            _cutSize++;
        }
    }
}

// calculate the number of cells in the From side or To side.
int Partitioner::From(Net* n, Cell* c)
{
    vector<int> cellList = n->getCellList(); // cell List on n;
    int a_count = 0, b_count = 0;

    for (int i = 0; i < cellList.size(); i++)
    {
        if (_cellArray[cellList[i]] != c)
        {
            //cout <<"IN F: " << _cellArray[cellList[i]]->getName() << endl;
            if (_cellArray[cellList[i]]->getPart() == 0)
            {
                a_count++;
            }
            else
            {
                b_count++;
            }
        }
    }

    if (c->getPart() == 0)
    {
        // mark 0 as from side;
        return a_count;
    }
    else
    {
        // mark b as from side.
        return b_count;
    }
}


int Partitioner::To(Net* n, Cell* c)
{
    vector<int> cellList = n->getCellList(); // cell List on n;
    int a_count = 0, b_count = 0;

    for (int i = 0; i < cellList.size(); i++)
    {
        if (_cellArray[cellList[i]] != c)
        {
            //cout <<"IN F: " << _cellArray[cellList[i]]->getName() << endl;
            if (_cellArray[cellList[i]]->getPart() == 0)
            {
                a_count++;
            }
            else
            {
                b_count++;
            }
        }
    }

    if (c->getPart() == 0)
    {
        // mark 0 as from side;
        return b_count;
    }
    else
    {
        // mark b as from side.
        return a_count;
    }
}


void Partitioner::initialCellGains()
{
    // inititial the cell gains.
    for (int i = 0; i < _cellArray.size(); i++)
    {
        _cellArray[i]->setGain(0); // initial the gain of all cells.
        cout << _cellArray[i]->getName() << endl;
        vector<int> netList = _cellArray[i]->getNetList();
        //  for each net on cell.
        for (int j = 0; j < netList.size(); j++)
        {
            //cout << _netArray[netList[j]]->getName() << " ";
            if (From(_netArray[netList[j]], _cellArray[i]) == 1)
            {
                _cellArray[i]->incGain();
            }
            if (To(_netArray[netList[j]], _cellArray[i]) == 0)
            {
                _cellArray[i]->decGain();
            }
        }
        cout << endl;
    }
}

void Partitioner::FM()
{
    initialCellGains();    // Computing the Cell Gains.
    // choose the first cell with maximum gain.
    int maximum_gain = 0;
    int maximum_index = 0;
    vector<bool> balanced_array(_cellNum);

    for (int i = 0; i < _cellNum; i++)
    {
        if (i == 0)
        {
            maximum_gain = _cellArray[0]->getGain();
            maximum_index = i;
        }
        else if (_cellArray[i]->getGain() > maximum_gain)
        {
            maximum_gain = _cellArray[i]->getGain();
            maximum_index = i;
        }
    }

}

bool Partitioner::balanced(Cell* c)
{
    int min_cell_nums = ((1 - _bFactor) / 2.0) * _cellNum;
    int max_cell_nums = ((1 + _bFactor) / 2.0) * _cellNum;

    return 0;
}


void Partitioner::partition()
{
    firstCut();
    FM();
}

void Partitioner::printSummary() const
{

    cout << "A part:" << endl;
    for (int i = 0; i < _cellArray.size(); i++)
    {
        if (_cellArray[i]->getPart() == 0)
        {
            cout << _cellArray[i]->getName() << " ";
        }
    }
    cout << endl;
    cout << "B part:" << endl;
    for (int i = 0; i < _cellArray.size(); i++)
    {
        if (_cellArray[i]->getPart() == 1)
        {
            cout << _cellArray[i]->getName() << ' ';
        }
    }

    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i)
    {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j)
        {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i)
    {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j)
        {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream &outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i)
    {
        if (_cellArray[i]->getPart() == 0)
        {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i)
    {
        if (_cellArray[i]->getPart() == 1)
        {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i)
    {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i)
    {
        delete _netArray[i];
    }
    return;
}
