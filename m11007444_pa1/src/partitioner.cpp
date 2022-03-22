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

void Partitioner::calcCutSize()
{
    _cutSize = 0;
    for (int i = 0; i < _netArray.size(); i++)
    {
        if (_netArray[i]->getPartCount(0) > 0 && _netArray[i]->getPartCount(1) > 0)
        {
            _cutSize++;
        }
    }
}

void Partitioner::firstCut()
{
    // divide the cells first.
    int halfCell = _cellArray.size() / 2; 
    for (int i = 0; i < _cellArray.size(); i++)
    {
        if (i < halfCell)
        {
            _cellArray[i]->setPart(0);
            _partSize[0]++;
            // Go over each Cell's connected Nets;
            vector<int> tmpNetList = _cellArray[i]->getNetList();
            for (int j = 0; j < tmpNetList.size(); j++)
            {
                _netArray[tmpNetList[j]]->incPartCount(0); // part A of this net need to increment its partCount; 
            }
            
        }
        else
        {
            _cellArray[i]->setPart(1);
            _partSize[1]++;
            // Go over each Cell's connected Nets;
            vector<int> tmpNetList = _cellArray[i]->getNetList();
            for (int j = 0; j < tmpNetList.size(); j++)
            {
                _netArray[tmpNetList[j]]->incPartCount(1); // part B of this net need to increment its partCount; 
            }
        }
        if (_cellArray[i]->getPinNum() > _maxPinNum)
        {
            _maxPinNum = _cellArray[i]->getPinNum();
        }
    }
    // traverse every net to calculate the initial cutsize.
    calcCutSize();
    cout << "The max pin number is :" << _maxPinNum << endl;
    cout << "After the first cut, cutsize = " << _cutSize << endl;
}


void Partitioner::initialCellGains()
{
    // inititial the cell gains.
    for (int i = 0; i < getCellNum(); i++)
    {
        _cellArray[i]->setGain(0); // initial the gain of all cells to zero. g(i) = 0;
    }
    //  for eachnet n on cell i .
    for (int i = 0; i < _netArray.size(); i++)
    {
        vector<int> tmpCellList = _netArray[i]->getCellList();
        for (int j = 0; j < tmpCellList.size(); j++)
        {
            int F = 0, T = 0;
            if (_cellArray[tmpCellList[j]]->getPart() == 0)
            {
                F = _netArray[i]->getPartCount(0);
                T = _netArray[i]->getPartCount(1);
            }
            else
            {
                F = _netArray[i]->getPartCount(1);
                T = _netArray[i]->getPartCount(0);
            }
            if (F == 1)
            {
                _cellArray[tmpCellList[j]]->incGain();
            }
            else if (T == 0)
            {
                _cellArray[tmpCellList[j]]->decGain();
            }
        }
    }

    constructBucketList();    // Insert THE CELL into the BucketList


}

void Partitioner::constructBucketList()
{
    // initial the bucket list.
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        _bList[0].insert(pair<int, Node*>(i, NULL));
        _bList[1].insert(pair<int, Node*>(i, NULL));
    }

    // put them into the bucket list.
    for (int i = 0; i < _cellNum; i++)
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
        if(_cellArray[i]->getLock() == false)
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

    reportCellDistribution();


    cout <<endl<< "===========THE CUTSIZE NOW IS " << _cutSize - _accGain << "===========" << endl << endl;

}

void Partitioner::FM()
{
    bool first = true;
    _iterNum = 0;
    do
    {
        if (!first)
        {
            initialPartitionParams();
        }
        else
        {
            first = false;
        }
        Cell* candidate = selectCell();    // choose the first cell with maximum gain.
        while (candidate != NULL)   // first iteration;
        {
            UpdateGain(candidate);
            //reportBucketList();
            candidate = selectCell();
        }
        backToBest();
        cout <<"max acc gain:" <<_maxAccGain << endl;
        _iterNum++;
    } while (_maxAccGain > 0);
}

void Partitioner::backToBest()
{

    // show the best movement.
    cout << "The bestMoveNum is " << _bestMoveNum << ", with gain: " << _maxAccGain << endl;
#if 0
    cout << "The best movement of this iteration is:";
    for (int i = 0; i < _bestMoveNum; i++)
    {
        cout << _cellArray[_moveStack[i]]->getName() << " ";
    }
    cout << endl << endl;
#endif
    for (int i = _bestMoveNum; i < _moveNum; i++) 
    {
        int throwCellId = _moveStack[i];
        Cell* throwCell = _cellArray[throwCellId];
        _partSize[throwCell->getPart()]--;
        _partSize[!throwCell->getPart()]++;
        throwCell->move();
    }
    _cutSize = _cutSize - _maxAccGain;

    // free every cell
    for (int i = 0; i < _cellNum; i++)
    {
        _cellArray[i]->unlock();
        _cellArray[i]->setGain(0);
    }
}

void Partitioner::initialPartitionParams()
{
    // initial the parameters.
    _accGain = 0;                    // accumulative gain
    _maxAccGain = 0;                 // maximum accumulative gain
    _moveNum = 0;                    // number of cell movements
    _bestMoveNum = 0;                // store best number of movements
    _moveStack.erase(_moveStack.begin(), _moveStack.end()); // remove all elements in the move stack.
    
    for (int i = _maxPinNum; i >= -1 * _maxPinNum; i--)
    {
        _bList[0][i] = NULL;
        _bList[1][i] = NULL;
    }
    #if 1
    for(int i = 0 ; i < _cellNum ; i++)
    {
        _cellArray[i]->getNode()->setPrev(NULL);
        _cellArray[i]->getNode()->setNext(NULL);
        _cellArray[i]->setGain(0);
    }
    #endif

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
//    cout << "The moving cell is " << baseCell->getName() << ", with gain = " << baseCell->getGain() << endl << endl;
    bool FromSide = baseCell->getPart();
    bool ToSide = !baseCell->getPart();
    // reflect the move
    _partSize[FromSide]--; // from side -1;
    _partSize[ToSide]++; // to side +1;
    // Lock the base cell and complement its block.
    baseCell->lock();
    baseCell->move(); // move the basecell !!

    vector<int> tmpNetList = baseCell->getNetList();
    for (int n = 0; n < tmpNetList.size(); n++)
    {
        // calculate F and T.
        int F = _netArray[tmpNetList[n]]->getPartCount(FromSide);
        int T = _netArray[tmpNetList[n]]->getPartCount(ToSide);
        vector<int> tmpCellList = _netArray[tmpNetList[n]]->getCellList();
        if (T == 0)
        {
            for (int i = 0; i < tmpCellList.size(); i++) // increment gains of all free cells on n;
            {
                if (_cellArray[tmpCellList[i]]->getLock() == false)
                {
                    UpdateBucketList(_cellArray[tmpCellList[i]], _cellArray[tmpCellList[i]]->getGain() + 1);
                    _cellArray[tmpCellList[i]]->incGain();
                }
            }
        }
        else if (T == 1) // decrement gain of the only T cell on n, if it is free.
        {
            for (int i = 0; i < tmpCellList.size(); i++)
            {
                if (_cellArray[tmpCellList[i]]->getPart() == ToSide && _cellArray[tmpCellList[i]]->getLock() == false)
                {
                    UpdateBucketList(_cellArray[tmpCellList[i]], _cellArray[tmpCellList[i]]->getGain() - 1);
                    _cellArray[tmpCellList[i]]->decGain();
                }
            }
        }
        // F(n) = F(n) - 1, T(n) = T(n) + 1;
        _netArray[tmpNetList[n]]->decPartCount(FromSide);
        _netArray[tmpNetList[n]]->incPartCount(ToSide);
        // update F and T;
        F = _netArray[tmpNetList[n]]->getPartCount(FromSide);
        T = _netArray[tmpNetList[n]]->getPartCount(ToSide);

        if (F == 0)
        {
            for (int i = 0; i < tmpCellList.size(); i++) // decrement gains of all free cells on n;
            {
                if (_cellArray[tmpCellList[i]]->getLock() == false)
                {
                    UpdateBucketList(_cellArray[tmpCellList[i]], _cellArray[tmpCellList[i]]->getGain() - 1);
                    _cellArray[tmpCellList[i]]->decGain();
                }
            }
        }
        else if (F == 1)
        {
            for (int i = 0; i < tmpCellList.size(); i++) // increment gain of the only F cell on n, if it is free.
            {
                if (_cellArray[tmpCellList[i]]->getPart() == FromSide && _cellArray[tmpCellList[i]]->getLock() == false)
                {
                    UpdateBucketList(_cellArray[tmpCellList[i]], _cellArray[tmpCellList[i]]->getGain() + 1);
                    _cellArray[tmpCellList[i]]->incGain();
                }
            }
        }
    }
   

    UpdatePartitionParams(baseCell);
}

void Partitioner::UpdatePartitionParams(Cell* baseCell)
{
    // modify the parameters.
    _accGain = _accGain + baseCell->getGain();
    _moveNum = _moveNum + 1;
    if (_accGain > _maxAccGain)
    {
        _maxAccGain = _accGain;
        _bestMoveNum = _moveNum;
    }
    _moveStack.push_back(_cellName2Id[baseCell->getName()]);
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
#if 0
        if(a_selected != NULL)
            cout << "The candidate of a is " << a_selected->getName() << endl;
        if(b_selected != NULL)
            cout << "The candidate of b is " << b_selected->getName() << endl;
#endif
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

        // cout << "part size of a is " << _partSize[0] << endl;
        // cout << "part size of b is " << _partSize[1] << endl;


        if (_partSize[candidate->getPart()] - 1 >= min_cell_nums && _partSize[!candidate->getPart()] + 1 <= max_cell_nums)
        {
            //cout << "Moving " << candidate->getName() << " is balanced !!!" << endl;
            return true;
        }
        else
        {
            //cout << "Moving " << candidate->getName() << " is not balanced !!!" << endl;
            return false;
        }
    }
}


void Partitioner::partition()
{
    const clock_t begin_time = clock();
    firstCut();
    initialCellGains();    // Computing the Cell Gains.
//    reportBucketList();
    FM();
    const clock_t end_time = clock();
    cout << "time elapsed: " <<  ( (double)(end_time-begin_time) / CLOCKS_PER_SEC ) << endl;
}

void Partitioner::reportCellDistribution()
{
    cout << "A part ("<<_partSize[0] << ") :"<< endl;
    for (int i = 0; i < _cellArray.size(); i++)
    {
        if (_cellArray[i]->getPart() == 0)
        {
            cout << _cellArray[i]->getName() << " ";
        }
    }
    cout << endl;
    cout << "B part (" << _partSize[1] << ") :" << endl;
    for (int i = 0; i < _cellArray.size(); i++)
    {
        if (_cellArray[i]->getPart() == 1)
        {
            cout << _cellArray[i]->getName() << ' ';
        }
    }
    cout << endl << endl;
}

void Partitioner::printSummary() const
{
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
