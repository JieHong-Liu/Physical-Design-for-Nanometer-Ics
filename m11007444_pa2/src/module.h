#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <cassert>
using namespace std;

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name, size_t x, size_t y) : _name(name), _x1(x), _y1(y), _x2(x), _y2(y) {}
    ~Terminal() {}

    // basic access methods
    const string getName() { return _name; }
    const size_t getX1() { return _x1; }
    const size_t getX2() { return _x2; }
    const size_t getY1() { return _y1; }
    const size_t getY2() { return _y2; }
    pair<double, double> getCenter()
    {
        double centerX = (double)(_x1 + _x2) / 2.0;
        double centerY = (double)(_y1 + _y2) / 2.0;
        //  cout << "The Terminal: " << this->getName() << ", the center x = " << centerX << ", the center y = " << centerY << endl;
        return make_pair(centerX, centerY);
    } // return pair
    // set functions
    void setName(string& name) { _name = name; }
    void setPos(size_t x1, size_t y1, size_t x2, size_t y2)
    {
        _x1 = x1;
        _y1 = y1;
        _x2 = x2;
        _y2 = y2;
    }

protected:
    string _name; // module name
    size_t _x1;   // min x coordinate of the terminal
    size_t _y1;   // min y coordinate of the terminal
    size_t _x2;   // max x coordinate of the terminal
    size_t _y2;   // max y coordinate of the terminal
};

class Block : public Terminal
{
public:
    // constructor and destructor
    Block(string& name, size_t w, size_t h) : Terminal(name, 0, 0), _w(w), _h(h), _isRotated(0) {}
    ~Block() {}

    // basic access methods
    const size_t getWidth() { return _isRotated ? _h : _w; }
    const size_t getHeight() { return _isRotated ? _w : _h; }
    const size_t getArea() { return _h * _w; }
    const bool getRotated() { return _isRotated; }
    // set functions
    void setWidth(size_t w) { _w = w; }
    void setHeight(size_t h) { _h = h; }
    void setRotated(bool rotated) { _isRotated = rotated; }
private:
    size_t _w;           // width of the block
    size_t _h;           // height of the block
    bool _isRotated;

};

class Node // the node of the tree.
{
public:
    Node(Block* b)
    {
        _blk = b;
        _leftChild = NULL;
        _rightChild = NULL;
        _parent = NULL;
    };
    ~Node() {};


    bool isLeftChild() // return true if the node is left child of its parent.
    {
        if(this->_parent == NULL) { cout << "this node did not have a parent" << endl; assert(false);}
        else if (this == this->_parent->_leftChild) {return true;}
        else return false;
    }

    bool hasTwoChilds() // return true if the node has two children
    {
        if(this->_leftChild != NULL && this->_rightChild != NULL) {return true;}
        else {return false;}
    }

    // get method
    Block* getBlock() { return _blk; }
    Node* getLeftChild() { return _leftChild; }
    Node* getRightChild() { return _rightChild; }
    Node* getParent() { return _parent; }
    Node* getChild(bool side)
    {
        if (side == 0)
        {
            return _leftChild;
        }
        else
        {
            return _rightChild;
        }
    }
    // set method
    void setLeftChild(Node* newNode) { _leftChild = newNode; }
    void setRightChild(Node* newNode) { _rightChild = newNode; }
    void setParent(Node* newNode) { _parent = newNode; }
    void setChild(bool side, Node* newNode) 
    {
        if (side == false) { _leftChild = newNode; } 
        else if (side == true) { _rightChild = newNode; }
    }

 
private:
    Block* _blk;
    Node* _leftChild;
    Node* _rightChild;
    Node* _parent;
};

class Net
{
public:
    // constructor and destructor
    Net() { _netDegree = 0; }
    Net(int degree) : _netDegree(degree) {}
    ~Net() {}

    // basic access methods
    const vector<Terminal*> getTermList() { return _termList; }

    // modify methods
    void addTerm(Terminal* term) { _termList.push_back(term); }

    // other member functions
    double calcHPWL()
    {
        double HPWL = 0;
        double min_x = 0, max_x = 0, min_y = 0, max_y = 0;
        pair<double, double> p;
        for (int i = 0; i < _netDegree; i++)
        {
            p = _termList[i]->getCenter();
            if (i == 0) { min_x = p.first; max_x = p.first; min_y = p.second; max_y = p.second; }
            else
            {
                if (p.first < min_x) { min_x = p.first; }
                else if (p.first > max_x) { max_x = p.first; }
                if (p.second < min_y) { min_y = p.second; }
                else if (p.second > max_y) { max_y = p.second; }
            }
        }
        HPWL = max_x - min_x + max_y - min_y;
        return HPWL;
    }

private:
    int _netDegree;
    vector<Terminal*> _termList; // list of terminals the net is connected to
};

#endif // MODULE_H
