#pragma once
#include <math.h>
#include <string>
#include <map>
#include "module.h"
#include <fstream>
using namespace std;

class BstarTree
{
public:
	BstarTree() // constructor of the b star tree.
	{
		_root = NULL;
	}

	Node* getRoot() { return _root; }
	void setRoot(Node* root) { _root = root; }

	void printBT(const std::string& prefix, Node* node, bool isLeft)
	{
		if (node != nullptr)
		{
			std::cout << prefix;

			std::cout << (isLeft ? "├──" : "└──");

			// print the value of the node
			std::cout << node->getBlock()->getName() << std::endl;

			// enter the next tree level - left and right branch
			printBT(prefix + (isLeft ? "│   " : "    "), node->getLeftChild(), true);
			printBT(prefix + (isLeft ? "│   " : "    "), node->getRightChild(), false);
		}
	}
private:
	Node* _root;
};

class Contour // the contour is a doubly linked list.
{
public:
	Contour()
	{
		_xStart = 0;
		_xEnd = 0;
		_y = 0;
		_next = NULL;
		_prev = NULL;
	}
	Contour(int xStart, int xEnd, int y) : _xStart(xStart), _xEnd(xEnd), _y(y), _next(NULL), _prev(NULL) {} // initial list.
	~Contour() {}
	// Basic Access Methods.
	int getStartX() { return _xStart; }
	int getEndX() { return _xEnd; }
	int getY() { return _y; }
	Contour* getNext() { return _next; } // get the next contour.
	Contour* getPrev() { return _prev; } // get the previous contour.
	// set methods
	void setStartX(int x) { _xStart = x; }
	void setEndX(int x) { _xEnd = x; }
	void setY(int y) { _y = y; }
	void setNext(Contour* next) { _next = next; }
	void setPrev(Contour* prev) { _prev = prev; }

private:
	int _xStart; // x start
	int _xEnd; // x end
	int _y;
	Contour* _next;
	Contour* _prev;
};

class Floorplanner
{
public:
	Floorplanner(fstream& input_blk, fstream& input_Net, fstream& output, double alpha) : _alpha(alpha)
	{
		parse_blk(input_blk);
		parse_net(input_Net);
	}
	void floorplan(fstream& output);
  void outputReport(fstream& output);
	// tree related;
	void treePacking(BstarTree* bstartree);
	void insertNode2BstarTree(BstarTree* tree, Block* blk);
	void tree2floorplan(BstarTree* tree);
	void packingNode2Floorplan(Contour* dummy, Node* current, BstarTree* tree);
	void tidyContour(Contour* dummy);

	// calculate Methods
	void calcFloorplanArea();
	void calculateTreeCost(BstarTree* tree);
	void calcWireLength();
  void calculateNormalize(BstarTree* bstartree);
	// Basic Access Methods.
	double getArea() { return _area; }
	double getCost() { return _cost; }
	double getChipWidth() { return _chipWidth; }
	double getChipHeight() { return _chipHeight; }
	double getWireLength() { return _wireLength; }
	int getOutlineWidth() { return _outlineWidth; }
	int getOutlineHeight() { return _outlineHeight; }

	double getStartTime() { return _startTime; }
	void setStartTime(double startTime) { _startTime = startTime; }
	// set functions
	void setCost(double cost) { _cost = cost; }
	void setArea(double area) { _area = area; }
	void setChipWidth(double chipWidth) { _chipWidth = chipWidth; }
	void setChipHeight(double chipHeight) { _chipHeight = chipHeight; }
	void setWireLength(double wirelength) { _wireLength = wirelength; }
	// report methods.
	void printSummary(BstarTree* tree);
	void reportContour(Contour* dummy);
	void reportBlockPos(Block* blk);

	// SA pertubation
	void rotateBlock(Block* blk);
	void swap2Nodes(BstarTree* tree, Node* node1, Node* node2);
	void deleteAndInsert(BstarTree* tree, Node* deleteNode, Node* insertNode, bool side, bool insertSide);	// delete in position 1 and insert in position 2.
	void swap2Subtrees(Node* target);
	Node* setOriginNode(Node* node1, bool& originSide, bool& insertSide);

	void SA(BstarTree* tree);

	double getCostSA();
	void saveCurrent2Optimum();
	bool cmpWithLocalOptimum();
private:
	double _alpha;
	void parse_blk(fstream& input_blk); // parse the input of the blocks
	void parse_net(fstream& input_Net); // parse the input of the nets
	int _numBlocks;						// number of blocks
	int _numTerms;						// number of terminals
	int _numNets;						// number of nets
	int _outlineWidth, _outlineHeight;
	double _cost;
	double _area;
	double _chipWidth;
	double _chipHeight;
	double _wireLength;
	double _heightStar;
	double _widthStar;
	double _aspectRatio;
	double _ratio;
	double _costSA;
	bool _passOutline;

	double _Anorm; // for calculate normalize.
	double _Wnorm;
	
	double _optCost;
	double _optCostSA;
	double _optArea;
	double _optChipWidth;
	double _optChipHeight;
	double _optWireLength;
	double _startTime;
	bool _optPassOutline;

	Contour* dummy = new Contour(-1, -1, -1);	// the head of the contour.
	vector<Block*> _blkList;					// the list of blocks
	vector<Block*> _optBlkList;					// the list of optimum blocks
	vector<Terminal*> _termList;				// the list of terminals
	vector<Net*> _netList;						// the list of nets
	vector<Node*> _nodeList;					// the list of nodes
	map<string, Block*> _name2Blk;				// mapping from block name to id
	map<string, Terminal*> _name2Term;			// mapping from terminal name to id

	void clearContours();
};
