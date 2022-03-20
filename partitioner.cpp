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

void Partitioner::parseInput(fstream& inFile)
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
    for (int i = 0; i < getCellNum(); i++)
    {
        _cellArray[i]->setGain(0); // initial the gain of all cells.
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
    }
    constructBucketList();
    // Insert into the BucketList

}

void Partitioner::constructBucketList()
{
    
    // initial the bucket list for every pin Node.
    for (int i = 0; i < getCellNum(); i++)
    {
        if (_cellArray[i]->getPinNum() > _maxPinNum)
        {
            _maxPinNum = _cellArray[i]->getPinNum();
        }
        //cout << "The pinNode of " << _cellArray[i]->getName() << " is : " << _cellArray[i]->getPinNum() << ", with gain : " << _cellArray[i]->getGain() << endl;
    }
    cout << "The max pin number is :" << _maxPinNum << endl;

    

    // initial the bucket list.
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        _bList[0].insert(pair<int, Node*>(i, NULL));
        _bList[1].insert(pair<int, Node*>(i, NULL));
    }

    // put them into the bucket list.
    for (int i = 0; i < getCellNum(); i++)
    {
        if (_bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()] == NULL)
        {
            _bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()] = _cellArray[i]->getNode();
        }
        else 
        {
            Node* tmpPtr = _bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()];
            while (tmpPtr->getNext() != NULL)
            {
                tmpPtr = tmpPtr->getNext();
            }
            // doubly linked list.
            tmpPtr->setNext(_cellArray[i]->getNode());
            tmpPtr->getNext()->setPrev(tmpPtr);
            tmpPtr->getNext()->setNext(NULL);
        }
    }


}

void Partitioner::reportGainList()
{
    for (int i = 0; i < getCellNum(); i++)
    {
        cout << "The gain of " << _cellArray[i]->getName() << " is : " << _cellArray[i]->getGain() << ", and its part is "<< _cellArray[i]->getPart() << endl;
    }
    cout << endl << endl;
}

void Partitioner::reportBucketList()
{

    reportGainList();

    cout << "IN the bList[0]: " << endl;
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        if (_bList[0][i] != NULL)
        {
            cout << "gain = " << i << " : ";
            Node* tmpPtr = _bList[0][i];
            while (tmpPtr->getNext() != NULL)
            {
                cout << _cellArray[tmpPtr->getId()]->getName() << " ";
                tmpPtr = tmpPtr->getNext();
            }
            cout << _cellArray[tmpPtr->getId()]->getName() << endl;
        }
        else
        {
            cout << "gain = " << i << " : Nothing" << endl;
        }
    }

    cout << endl << endl;
    // check the bucket list.
    cout << "IN the bList[1]: " << endl;
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        if (_bList[1][i] != NULL)
        {
            cout << "gain = " << i << " : ";
            Node* tmpPtr = _bList[1][i];
            while (tmpPtr->getNext() != NULL)
            {
                cout << _cellArray[tmpPtr->getId()]->getName() << " ";
                tmpPtr = tmpPtr->getNext();
            }
            cout << _cellArray[tmpPtr->getId()]->getName() << endl;
        }
        else
        {
            cout << "gain = " << i << " : Nothing" << endl;
        }
    }

    cout << "THE CUTSIZE NOW IS " << _cutSize << endl << endl;

}

void Partitioner::FM()
{
    initialCellGains();    // Computing the Cell Gains.
    // choose the first cell with maximum gain.
    reportBucketList();
    Cell* candidate = selectCell();
    while (candidate != NULL)
    {
        UpdateGain(candidate);
        reportBucketList();
        candidate = selectCell();
    }


}

void Partitioner::UpdateBucketList(Cell* baseCell,int newGain)
{
    if (baseCell == NULL)
    {
        return;
    }
    else
    {
        // update origin gain
        if (baseCell->getNode()->getPrev() == NULL)
        {
            _bList[baseCell->getPart()][baseCell->getGain()] = baseCell->getNode()->getNext();
            if(_bList[baseCell->getPart()][baseCell->getGain()] != NULL)
                _bList[baseCell->getPart()][baseCell->getGain()]->setPrev(NULL);
        }
        else
        {
            baseCell->getNode()->getPrev()->setNext(baseCell->getNode()->getNext());
            if(baseCell->getNode()->getNext() != NULL)  
                baseCell->getNode()->getNext()->setPrev(baseCell->getNode()->getPrev());
        }
        // update new Gain
        Node* tmpPtr = _bList[baseCell->getPart()][newGain];
        if (tmpPtr == NULL)
        {
            _bList[baseCell->getPart()][newGain] = baseCell->getNode();
            baseCell->getNode()->setNext(NULL);
            baseCell->getNode()->setPrev(NULL);
        }
        else
        {
            while (tmpPtr->getNext() != NULL)
            {
                tmpPtr = tmpPtr->getNext();
            }
            tmpPtr->setNext(baseCell->getNode());
            baseCell->getNode()->setPrev(tmpPtr);
            baseCell->getNode()->setNext(NULL);
        }
    }

}

void Partitioner::UpdateGain(Cell* baseCell)
{
    // Lock the base cell and complement its block.
    baseCell->lock();

    bool FromSide = baseCell->getPart();
    bool ToSide = !baseCell->getPart();

    cout << "The moving cell is " << baseCell->getName() << endl;


    // reflect the move
    _partSize[FromSide]--; // from side -1;
    _partSize[ToSide]++; // to side +1;

    baseCell->move(); // move it!!
    _cutSize = _cutSize - baseCell->getGain();

    vector<int> netList = baseCell->getNetList();
    for (int n = 0; n < netList.size(); n++)
    {
        vector<int> cellList = _netArray[netList[n]]->getCellList();

        if (To(_netArray[netList[n]], baseCell) == 0)
        {
            // increment gains of all free cells on n.
            for (int c = 0; c < cellList.size(); c++) 
            {
                if (_cellArray[cellList[c]]->getLock() == 0) // free cell
                {
                    UpdateBucketList(_cellArray[cellList[c]],_cellArray[cellList[c]]->getGain()+1);
                    _cellArray[cellList[c]]->incGain();
                }
            }
        }
        else if (To(_netArray[netList[n]], baseCell) == 1)
        {
            // decrement gain of the only T cell on n;
            for (int c = 0; c < cellList.size(); c++) 
            {
                if (_cellArray[cellList[c]]->getLock() == 0 && _cellArray[cellList[c]]->getPart() == ToSide ) // free cell
                {
                    UpdateBucketList(_cellArray[cellList[c]], _cellArray[cellList[c]]->getGain() - 1);
                    _cellArray[cellList[c]]->decGain();
                }
            }
        }


        if (From(_netArray[netList[n]], baseCell)-1 == 0)
        {
            // decrement gains of all free cells on n.
            for (int c = 0; c < cellList.size(); c++)
            {
                if (_cellArray[cellList[c]]->getLock() == 0) // free cel
                {
                    UpdateBucketList(_cellArray[cellList[c]], _cellArray[cellList[c]]->getGain() - 1);
                    _cellArray[cellList[c]]->decGain();
                }
            }
        }
        else if (From(_netArray[netList[n]], baseCell) - 1 == 1)
        {
            // increment gains of only F cells on n.
            for (int c = 0; c < cellList.size(); c++)
            {
                if (_cellArray[cellList[c]]->getPart() == FromSide && _cellArray[cellList[c]]->getLock() == 0) // free cell
                {
                    UpdateBucketList(_cellArray[cellList[c]], _cellArray[cellList[c]]->getGain() + 1);
                    _cellArray[cellList[c]]->incGain();
                }
            }
        }
    }
 
}



Cell* Partitioner::selectCell()
{
    Cell* a_selected = NULL;
    Cell* b_selected = NULL;
    // select the maximum gain for two part.
    bool selectA = false;
    bool selectB = false;


    // find a part;
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        if (_bList[0][i] != NULL)
        {
            a_selected = _cellArray[_bList[0][i]->getId()]; // delete after
            break;
        }
    }

    // find b part;
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        if (_bList[1][i] != NULL)
        {
            b_selected = _cellArray[_bList[1][i]->getId()]; // delete after
            break;
        }
    }
   

    if (a_selected == NULL && b_selected == NULL)
    {
        cout << "all cells are locked!!!" << endl;
        return NULL;
    }
    else
    {
        if(a_selected != NULL)
            cout << "The candidate of a is " << a_selected->getName() << endl;
        if(b_selected != NULL)
            cout << "The candidate of b is " << b_selected->getName() << endl;

        bool a_VAL = balanced(a_selected);
        bool b_VAL = balanced(b_selected);

        if (a_VAL && b_VAL) // both side are balance.
        {
            if (a_selected->getGain() >= b_selected->getGain())
            {
                selectA = true;
                selectB = false;
            }
            else
            {
                selectA = false;
                selectB = true;
            }
        }
        else if (a_VAL && !b_VAL) // part a is balance, part b is not.
        {
            selectA = true;
            selectB = false;
        }
        else if (!a_VAL && b_VAL) // part a is not balance, part b is.
        {
            selectA = false;
            selectB = true;
        }
        else // both part are unbalanced.
        {
            cout << "No cell can be selected" << endl;
            selectA = false;
            selectB = false;
        }
    }
    if (selectA == true)
    {
        // remove the node of the bucket list.
        if (_bList[0][a_selected->getGain()] != NULL)
        {
            _bList[0][a_selected->getGain()] = _bList[0][a_selected->getGain()]->getNext();
            if(_bList[0][a_selected->getGain()] != NULL)
                _bList[0][a_selected->getGain()]->setPrev(NULL);
        }

        return a_selected;
    }
    else if (selectB == true)// select B is true.
    {
        // remove the node of the bucket list.
        if (_bList[1][b_selected->getGain()] != NULL)
        {
            _bList[1][b_selected->getGain()] = _bList[1][b_selected->getGain()]->getNext();
            if (_bList[1][b_selected->getGain()] != NULL)
                _bList[1][b_selected->getGain()]->setPrev(NULL);
        }
        return b_selected;
    }
    else
    {
        return NULL;
    }
}

bool Partitioner::balanced(Cell* candidate)
{
    if (candidate == NULL)
        return false;
    else
    {
        double min_cell_nums = ((1 - getBFactor()) / 2.0) * _cellNum;
        double max_cell_nums = ((1 + getBFactor()) / 2.0) * _cellNum;

        cout << "part size of a is " << _partSize[0] << endl;
        cout << "part size of b is " << _partSize[1] << endl;


        if (_partSize[candidate->getPart()] - 1 >= min_cell_nums && _partSize[!candidate->getPart()] + 1 <= max_cell_nums)
        {
            cout << "Moving " << candidate->getName() << " is balanced !!!" << endl;
            return true;
        }
        else
        {
            cout << "Moving " << candidate->getName() << " is not balanced !!!" << endl;
            return false;
        }
    }
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

void Partitioner::writeResult(fstream& outFile)
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
