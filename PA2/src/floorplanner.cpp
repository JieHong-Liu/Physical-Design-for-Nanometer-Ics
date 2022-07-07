#include "floorplanner.h"
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string>
using namespace std;
#define OUTLINEINF 99999
void Floorplanner::floorplan(fstream& output)
{
	srand(time(NULL));
	const clock_t begin_time = clock();
	setStartTime(begin_time);
	// construct the bstar tree.
	BstarTree* bstartree = new BstarTree;
	treePacking(bstartree); // initialize the complete binary tree.
  calculateNormalize(bstartree);
  SA(bstartree);
  
  
  // Applied Fast Simulated Annealing.
	printSummary(bstartree);
  outputReport(output);
}

void Floorplanner::calculateNormalize(BstarTree* bstartree)
{
  double _accumArea = 0;
  double _accumWireLength = 0;
 	for (int i = 0; i < 100; i++) // do 1000 times iteration.
		{
			int operation = rand() % 4;
			switch (operation)
			{
  			case 0: // rotate a block.
  			{
  				// cout << "rotation!!!" << endl;
  				int randBlk = rand() % _blkList.size();
  				rotateBlock(_blkList[randBlk]);
  				tree2floorplan(bstartree);
  				calculateTreeCost(bstartree);
  				break;
  			}
  
  			case 1: // swap two nodes
  			{
  				// cout << "swap nodes!!!" << endl;
  				int randNode1 = rand() % _nodeList.size();
  				int randNode2 = rand() % _nodeList.size();
  				if (randNode1 == randNode2) break;
  				swap2Nodes(bstartree, _nodeList[randNode1], _nodeList[randNode2]);
  				tree2floorplan(bstartree);
  				calculateTreeCost(bstartree);
          break;
  			}
  			case 2: // delete and insert (move the node.)
  			{
  				// cout << "delete and insert" << endl;
  				int randNode1 = rand() % _nodeList.size();
  				int randNode2 = rand() % _nodeList.size();
  				if (randNode1 == randNode2) break;
  				bool side = rand() % 2;
  				bool insertSide = rand() % 2;
  
  				deleteAndInsert(bstartree, _nodeList[randNode1], _nodeList[randNode2], side, insertSide);
  				tree2floorplan(bstartree);
  				calculateTreeCost(bstartree);
  
  				break;
  			}
  			case 3: // swap two subtrees
  			{
  				// cout << "Swap two subtress !!!" << endl;
  				int randNode = rand() % _nodeList.size();
  				swap2Subtrees(_nodeList[randNode]);
  				tree2floorplan(bstartree);
  				calculateTreeCost(bstartree);
  
  				break;
  
  			}
  			default: //
  				assert(false);
			}
    	  _accumWireLength += getWireLength();
	      _accumArea += getArea();
    }
    _Anorm = _accumArea / 100.0;
	  _Wnorm = _accumWireLength / 100.0;
}


double Floorplanner::getCostSA()
{
	double costSA = 0;
	calcWireLength();
	calcFloorplanArea();

	_aspectRatio = _outlineHeight / _outlineWidth;
	_ratio = getChipHeight() / getChipWidth();
	costSA =( _alpha * (getArea()/_Anorm) + (1-_alpha) * (getWireLength()/_Wnorm) )+  (_aspectRatio - _ratio) * (_aspectRatio - _ratio);
	return costSA;
}

void Floorplanner::saveCurrent2Optimum()
{
	bool pass = false;
	if (getChipWidth() <= getOutlineWidth() && getChipHeight() <= getOutlineHeight())
	{
		pass = true;
	}
	else
	{
		pass = false;
	}
	_optPassOutline = pass;
	_optArea = getArea();
	_optCost = getCost();
	_optCostSA = getCostSA();
	_optChipWidth = getChipWidth();
	_optChipHeight = getChipHeight();
	_optWireLength = getWireLength();
	// copy the block data into opt blk list.
	_optBlkList.clear(); // clear the list.
	for (int i = 0; i < _blkList.size(); i++)
	{
		string str = _blkList[i]->getName();
		Block* blk = new Block(str, _blkList[i]->getWidth(), _blkList[i]->getHeight());
		blk->setPos(_blkList[i]->getX1(), _blkList[i]->getY1(), _blkList[i]->getX2(), _blkList[i]->getY2());
		_optBlkList.push_back(blk);
	}
}

void Floorplanner::SA(BstarTree* bstartree)
{
	tree2floorplan(bstartree);
	calculateTreeCost(bstartree);
	saveCurrent2Optimum();
#if 1
	for (double temperature = 1000; temperature >= 1; temperature = temperature * 0.95)		// while not yet frozen
	{
		double P = 0;
		if (temperature > 100) P = 1000;
		else if (temperature > 50) P = 1000;
		else P = 1000;

		for (int i = 0; i < P; i++) // do P times iteration.
		{
			tree2floorplan(bstartree);
			calculateTreeCost(bstartree);
			double originCost = getCostSA();
			double costDiff = 0;
			int operation = rand() % 4;
			switch (operation)
			{
      case -1:
         break;
			case 0: // rotate a block.
			{
				// cout << "rotation!!!" << endl;
				int randBlk = rand() % _blkList.size();
				rotateBlock(_blkList[randBlk]);
				tree2floorplan(bstartree);
				calculateTreeCost(bstartree);
				double newCost = getCostSA();
				costDiff = newCost - originCost;
				if (costDiff > 0)
				{
					double prob2 = exp(-1 * (costDiff / temperature));
					double acceptProb = (double)rand() / (RAND_MAX + 1.0);

					if (acceptProb >= prob2)
					{
						// we accept the new solution
					}
					else
					{
						rotateBlock(_blkList[randBlk]);// we don't accept the new solution
					}
				}
				break;
			}

			case 1: // swap two nodes
			{
				// cout << "swap nodes!!!" << endl;
				int randNode1 = rand() % _nodeList.size();
				int randNode2 = rand() % _nodeList.size();
				if (randNode1 == randNode2) break;
				swap2Nodes(bstartree, _nodeList[randNode1], _nodeList[randNode2]);
				tree2floorplan(bstartree);
				calculateTreeCost(bstartree);
				double newCost = getCostSA();
				costDiff = newCost - originCost;
				if (costDiff > 0)
				{
					double prob2 = exp(-1 * (costDiff / temperature));
					double acceptProb = (double)rand() / (RAND_MAX + 1.0);
					if (acceptProb >= prob2)
					{
						// we accept the new solution
					}
					else
					{
						swap2Nodes(bstartree, _nodeList[randNode1], _nodeList[randNode2]);	// we don't accept the new solution
					}
				}
				break;
			}
			case 2: // delete and insert (move the node.)
			{
				// cout << "delete and insert" << endl;
				int randNode1 = rand() % _nodeList.size();
				int randNode2 = rand() % _nodeList.size();
				if (randNode1 == randNode2) break;

				Node* originParent = NULL;
				bool originSide = false; // the origin side child of its parent
				bool originInsertSide = false; // the children side of the node 
				bool side = rand() % 2;
				bool insertSide = rand() % 2;
				originParent = setOriginNode(_nodeList[randNode1], originSide, originInsertSide);

				deleteAndInsert(bstartree, _nodeList[randNode1], _nodeList[randNode2], side, insertSide);
				tree2floorplan(bstartree);
				calculateTreeCost(bstartree);

				double newCost = getCostSA();
				costDiff = newCost - originCost;
				if (costDiff > 0)
				{
					double prob2 = exp(-1 * (costDiff / temperature));
					double acceptProb = (double)rand() / (RAND_MAX + 1.0);
					if (acceptProb >= prob2)
					{
						// we accept the new solution
						// cout << "accept the better sol" << endl;
					}
					else
					{
						// cout << "accept the uphill solution" << endl;
						deleteAndInsert(bstartree, _nodeList[randNode1], originParent, originSide, originInsertSide); // reset the delete node.
					}
				}

				break;
			}
			case 3: // swap two subtrees
			{
				// cout << "Swap two subtress !!!" << endl;
				int randNode = rand() % _nodeList.size();
				swap2Subtrees(_nodeList[randNode]);
				tree2floorplan(bstartree);
				calculateTreeCost(bstartree);

				double newCost = getCostSA();
				costDiff = newCost - originCost;
				if (costDiff > 0)
				{
					double prob2 = exp(-1 * (costDiff / temperature));
					double acceptProb = (double)rand() / (RAND_MAX + 1.0);
					if (acceptProb >= prob2)
					{
						// we accept the new solution
					}
					else
					{
						swap2Subtrees(_nodeList[randNode]); // reset the delete node.
					}
				}

				break;

			}
			default: //
				assert(false);
			}
			if (cmpWithLocalOptimum()) saveCurrent2Optimum();
		 // clearContours();
    }

	}
#endif


}

Node* Floorplanner::setOriginNode(Node* node1, bool& originSide, bool& originInsertSide)
{
	if (node1 == NULL) return NULL;
	Node* originParent = NULL;
	if (!node1->hasTwoChilds() && node1->getParent() != NULL)
	{
		originParent = node1->getParent();
		// cout<< "the origin parent is :"  << originParent->getBlock()->getName() << endl;
		if (node1->isLeftChild())
		{
			originSide = false; // origin side of its parent is left.
		}
		else
		{
			originSide = true;
		}

		if (node1->getLeftChild() != NULL)
		{
			originInsertSide = false;
		}
		else if (node1->getRightChild() != NULL)
		{
			originInsertSide = true;
		}
		else
		{
			// assert(false); // origin node did not have any chihldren
		}
	}
	else
	{
		// cout << "origin parent is NULL" << endl;
	}
	return originParent;
}

void Floorplanner::rotateBlock(Block* blk)
{
	blk->setRotated(!blk->getRotated());
}

void Floorplanner::swap2Subtrees(Node* target)
{
	Node* tmp = target->getRightChild();
	target->setRightChild(target->getLeftChild());
	target->setLeftChild(tmp);
}

void Floorplanner::deleteAndInsert(BstarTree* tree, Node* deleteNode, Node* insertNode, bool side, bool insertSide)
{
#if 0
	cout << "Enter delete and insert" << endl;
	cout << "deleteNode: " << deleteNode->getBlock()->getName() << endl;
	cout << "insertNode: " << insertNode->getBlock()->getName() << endl;
#endif
	if (deleteNode == NULL || insertNode == NULL) { return; }
	// deleteNode can not have relationship with insert node.
	else if (deleteNode == insertNode->getLeftChild() || deleteNode == insertNode->getRightChild() || deleteNode == insertNode->getParent())
	{
		return;
	}
	// This operation only delete the node with degree = 1 or degree = 0;
	else if (deleteNode->hasTwoChilds())
	{
		// the degree of this node is 2, don't do this operation.
		return;
	}
	else if (deleteNode == tree->getRoot())
	{
		return;
	}
	else if (deleteNode->getLeftChild() == NULL && deleteNode->getRightChild() == NULL)
	{
		if (deleteNode->getParent() != NULL)
		{
			// the degree of this node is 0.
			if (deleteNode == deleteNode->getParent()->getLeftChild()) // this node is left child of its parent;
			{
				deleteNode->getParent()->setLeftChild(NULL);
			}
			else if (deleteNode == deleteNode->getParent()->getRightChild())
			{
				deleteNode->getParent()->setRightChild(NULL);
			}
			else
			{
				assert(false);
			}
		}

	}
	else if (deleteNode->getLeftChild() != NULL && deleteNode->getRightChild() == NULL) // degree = 1 (only child is left child);
	{
		if (deleteNode->getParent() != NULL)
		{
			if (deleteNode == deleteNode->getParent()->getLeftChild()) // this node is left child of its parent;
			{
				deleteNode->getParent()->setLeftChild(deleteNode->getLeftChild());
				deleteNode->getLeftChild()->setParent(deleteNode->getParent());
			}
			else if (deleteNode == deleteNode->getParent()->getRightChild())
			{
				deleteNode->getParent()->setRightChild(deleteNode->getLeftChild());
				deleteNode->getLeftChild()->setParent(deleteNode->getParent());
			}
			else
			{
				assert(false);
			}
		}
	}

	else if (deleteNode->getLeftChild() == NULL && deleteNode->getRightChild() != NULL) // degree = 1 (only child is right child);
	{
		if (deleteNode->getParent() != NULL)
		{
			if (deleteNode == deleteNode->getParent()->getLeftChild()) // this node is left child of its parent;
			{
				deleteNode->getParent()->setLeftChild(deleteNode->getRightChild());
				deleteNode->getRightChild()->setParent(deleteNode->getParent());
			}
			else if (deleteNode == deleteNode->getParent()->getRightChild())
			{
				deleteNode->getParent()->setRightChild(deleteNode->getRightChild());
				deleteNode->getRightChild()->setParent(deleteNode->getParent());
			}
			else
			{
				assert(false);
			}
		}
	}
	// clear the deleteNode to NULL
	deleteNode->setParent(NULL);
	deleteNode->setLeftChild(NULL);
	deleteNode->setRightChild(NULL);

	// insert the deleteNode into the insertNode.
	deleteNode->setParent(insertNode);
	deleteNode->setChild(insertSide, insertNode->getChild(side));
	if (insertNode->getChild(side) != NULL)
	{
		insertNode->getChild(side)->setParent(deleteNode);
	}
	insertNode->setChild(side, deleteNode);

}

void Floorplanner::swap2Nodes(BstarTree* tree, Node* node1, Node* node2)
{
	if (node1 == node2) return;

	string str;
	Node* tmp = new Node(new Block(str, 0, 0));


	tmp->setParent(node1->getParent());
	tmp->setLeftChild(node1->getLeftChild());
	tmp->setRightChild(node1->getRightChild());

	if (node1 == tree->getRoot())
	{
		tree->setRoot(tmp);
	}
	else
	{
		if (node1 == tmp->getParent()->getLeftChild()) // left child of its parent.
		{
			tmp->getParent()->setLeftChild(tmp);
		}
		else if (node1 == tmp->getParent()->getRightChild()) // right child of its parent.
		{
			tmp->getParent()->setRightChild(tmp);
		}
		else
		{
			assert(false); // the program should never reach here.
		}
	}

	if (tmp->getLeftChild() != NULL)
		tmp->getLeftChild()->setParent(tmp);
	if (tmp->getRightChild() != NULL)
		tmp->getRightChild()->setParent(tmp);

	node1->setParent(node2->getParent());
	node1->setLeftChild(node2->getLeftChild());
	node1->setRightChild(node2->getRightChild());

	if (node2 == tree->getRoot())
	{
		tree->setRoot(node1);
	}
	else
	{
		if (node2 == node1->getParent()->getLeftChild()) // left child of its parent.
		{
			node1->getParent()->setLeftChild(node1);
		}
		else if (node2 == node1->getParent()->getRightChild()) // right child of its parent.
		{
			node1->getParent()->setRightChild(node1);
		}
		else
		{
			assert(false); // the program should never reach here
		}
	}

	if (node1->getLeftChild() != NULL)
		node1->getLeftChild()->setParent(node1);
	if (node1->getRightChild() != NULL)
		node1->getRightChild()->setParent(node1);

	node2->setParent(tmp->getParent());
	node2->setLeftChild(tmp->getLeftChild());
	node2->setRightChild(tmp->getRightChild());
	if (node2->getLeftChild() != NULL)
		node2->getLeftChild()->setParent(node2);
	if (node2->getRightChild() != NULL)
		node2->getRightChild()->setParent(node2);
	if (tmp == tree->getRoot())
	{
		tree->setRoot(node2);
	}
	else
	{
		if (tmp == node2->getParent()->getLeftChild()) // left child of its parent.
		{
			node2->getParent()->setLeftChild(node2);
		}
		else if (tmp == node2->getParent()->getRightChild()) // right child of its parent.
		{
			node2->getParent()->setRightChild(node2);
		}
		else
		{
			assert(false);
		}
	}
}

bool Floorplanner::cmpWithLocalOptimum()
{
	if (getChipWidth() <= getOutlineWidth() && getChipHeight() <= getOutlineHeight())
	{
		_passOutline = true;
	}
	else
	{
		_passOutline = false;
	}
	if (_passOutline == true &&  getCostSA() < _optCostSA)
	{
		return true;
	}
	else
	{
		return false;
	}
}



void Floorplanner::printSummary(BstarTree* tree)
{
	cout << "===============Summary (last solution )=============" << endl;
	tree->printBT("", tree->getRoot(), 0);
	cout << "The cost of the tree is :" << getCost() << endl;
	cout << "The total wireLength is :" << getWireLength() << endl;
	cout << "The area is " << getArea() << endl;
	cout << "Chip width: " << getChipWidth() << ", Chip Height: " << getChipHeight() << endl;
	if (getChipWidth() <= getOutlineWidth() && getChipHeight() <= getOutlineHeight())
	{
		cout << "PASS THE OUTLINE !!!" << endl;
	}
	else
	{
		cout << "Did not pass the outline QQ" << endl;
	}
	for (int i = 0; i < _blkList.size(); i++)
	{
		reportBlockPos(_blkList[i]);
	}
	reportContour(this->dummy);
	cout << "===================================" << endl << endl;


	cout << "===============Summary (Best solution )=============" << endl;
	// tree->printBT("", tree->getRoot(), 0);
	cout << "The cost of the tree is :" << _optCost << endl;
	cout << "The total wireLength is :" << _optWireLength << endl;
	cout << "The area is " << _optArea << endl;
	cout << "Chip width: " << _optChipWidth << ", Chip Height: " << _optChipHeight << endl;
	if (_optPassOutline)
	{
		cout << "PASS THE OUTLINE !!!" << endl;
	}
	else
	{
		cout << "Did not pass the outline QQ" << endl;
	}
	cout << "Time elapsed: " << double(clock() - _startTime) / CLOCKS_PER_SEC << endl;

	for (int i = 0; i < _optBlkList.size(); i++)
	{
		reportBlockPos(_optBlkList[i]);
	}
	cout << "===================================" << endl << endl;

}


void Floorplanner::outputReport(fstream& output)
{
	output <<  _optCost << endl;
	output <<  _optWireLength << endl;
	output <<  _optArea << endl;
	output <<  _optChipWidth << " " << _optChipHeight << endl;
	output << double(clock() - _startTime) / CLOCKS_PER_SEC << endl;
	for (int i = 0; i < _optBlkList.size(); i++)
	{
     	output << _optBlkList[i]->getName() << "\t" << _optBlkList[i]->getX1() << "\t" << _optBlkList[i]->getY1() << "\t" << _optBlkList[i]->getX2() << "\t" << _optBlkList[i]->getY2() << endl;
	}
}

void Floorplanner::calculateTreeCost(BstarTree* tree)
{
	double cost = 0;
	calcWireLength();
	calcFloorplanArea();
	cost = _alpha * getArea() + (1 - _alpha) * getWireLength();
	setCost(cost);
}

void Floorplanner::calcWireLength()
{
	// show the net HPWL.
	double sum = 0;
	for (int i = 0; i < _netList.size(); i++)
	{
		sum = sum + _netList[i]->calcHPWL();
		//		cout << "i = " << i << ", HPWL = " << _netList[i]->calcHPWL() << endl;
	}
	setWireLength(sum);
}

void Floorplanner::calcFloorplanArea()
{
	double Area = 0;
	Contour* tmpContour = this->dummy->getNext();
	int max_x = 0, max_y = 0;
	while (tmpContour != NULL)
	{
		if (tmpContour->getEndX() > max_x && tmpContour->getY() != 0)
		{
			max_x = tmpContour->getEndX();
		}
		if (tmpContour->getY() > max_y)
		{
			max_y = tmpContour->getY();
		}
		tmpContour = tmpContour->getNext();
	}
	Area = (double)max_x * (double)max_y;
	setChipWidth(max_x);
	setChipHeight(max_y);
	setArea(Area);
}
void Floorplanner::parse_blk(fstream& inputFile)
{
	string str;
	inputFile >> str; // parse outline
	if (str == "Outline:")
	{
		inputFile >> _outlineWidth >> _outlineHeight;
	}
	inputFile >> str;
	if (str == "NumBlocks:")
	{
		inputFile >> _numBlocks;
	}
	inputFile >> str;
	if (str == "NumTerminals:")
	{
		inputFile >> _numTerms;
	}

	for (int i = 0; i < _numBlocks; i++)
	{
		string blkName;
		int blk_width, blk_height;
		inputFile >> blkName; // block name
		inputFile >> blk_width >> blk_height;
		Block* tmpBlock = new Block(blkName, blk_width, blk_height);
		_blkList.push_back(tmpBlock);
		_name2Blk[blkName] = _blkList[i]; // index of the blkList.
	}

	for (int i = 0; i < _numTerms; i++)
	{
		string termName;
		int termX, termY;
		int tmpX, tmpY;
		inputFile >> termName >> str; // which str = "terminal"
		inputFile >> tmpX >> tmpY;	  // the x y of the term.
		Terminal* tmpTerm = new Terminal(termName, tmpX, tmpY);
		_termList.push_back(tmpTerm);
		_name2Term[termName] = _termList[i];
	}

	cout << "SHOW BLOCKS:" << endl;
	for (int i = 0; i < _numBlocks; i++)
	{
		cout << _blkList[i]->getName() << " " << _blkList[i]->getWidth() << " " << _blkList[i]->getHeight() << endl;
	}
}

void Floorplanner::parse_net(fstream& inputFile)
{
	string str;
	inputFile >> str;
	if (str == "NumNets:")
	{
		inputFile >> _numNets;
	}

	for (int i = 0; i < _numNets; i++)
	{
		int netDegree;
		inputFile >> str; // NetDegree
		inputFile >> netDegree;
		Net* tmpNet = new Net(netDegree);
		_netList.push_back(tmpNet);
		for (int j = 0; j < netDegree; j++)
		{
			string termName;
			inputFile >> termName;
			if (_name2Blk.count(termName) > 0) // this is a block.
			{
				tmpNet->addTerm(_name2Blk[termName]);
			}
			else // this is a terminal.
			{
				tmpNet->addTerm(_name2Term[termName]);
			}
		}
	}
}

void Floorplanner::treePacking(BstarTree* bstartree)
{

#if 1
	for (int i = 0; i < _numBlocks; i++) // construct the bstar tree.
	{
		insertNode2BstarTree(bstartree, _blkList[i]);
	}

	if (bstartree->getRoot() == NULL)
	{
		assert(false);
		cout << "The root of the bstar tree is NULL! " << endl;
	}
#endif
}

void Floorplanner::insertNode2BstarTree(BstarTree* tree, Block* blk)
{
	//	cout << "Insert node 2 bstar tree" << endl;
	queue<Node*> q;
	Node* current = tree->getRoot();
	Node* newNode = NULL;

	if (current == NULL)
	{
		newNode = new Node(blk);
		tree->setRoot(newNode);
	}
	else
	{
		while (current)
		{
			if (current->getLeftChild() != NULL)
			{
				q.push(current->getLeftChild());
			}
			else
			{
				newNode = new Node(blk);
				current->setLeftChild(newNode);
				current->getLeftChild()->setParent(current);
				break;
			}
			if (current->getRightChild() != NULL)
			{
				q.push(current->getRightChild());
			}
			else // can be here.
			{
				newNode = new Node(blk);
				current->setRightChild(newNode);
				current->getRightChild()->setParent(current);
				break;
			}
			current = q.front();
			q.pop();
		}
	}
	if (newNode != NULL)
	{
		_nodeList.push_back(newNode);
	}
	else
	{
		cerr << "NULL NODE OCCUR!!!" << endl;
	}
}

void Floorplanner::tree2floorplan(BstarTree* tree)
{
	// initialize the contour data structure.
	Node* current = tree->getRoot();
	packingNode2Floorplan(this->dummy, current, tree); //visit every node by DFS.

}

void Floorplanner::reportContour(Contour* dummy)
{
	// tidyContour(dummy);
	cout << "SHOW CONTOUR: " << endl;
	Contour* tmp = dummy->getNext();
	while (tmp != NULL)
	{
		cout << "( " << tmp->getStartX() << ", " << tmp->getEndX() << ", " << tmp->getY() << ")" << endl;
		tmp = tmp->getNext();
	}
}

void Floorplanner::tidyContour(Contour* dummy)
{
	Contour* tmp = dummy;
	while (tmp != NULL)
	{
		if (tmp->getNext() != NULL && tmp->getY() == tmp->getNext()->getY())
		{
			Contour* deleteNode = tmp->getNext();
			tmp->setEndX(tmp->getNext()->getEndX());
			tmp->setNext(deleteNode->getNext());
			deleteNode->getNext()->setPrev(tmp);
			delete deleteNode;
		}
		tmp = tmp->getNext();
	}
}

void Floorplanner::packingNode2Floorplan(Contour* dummy, Node* current, BstarTree* tree)
{
	Contour* tmp1;
	Contour* tmp2;
//	cout << "packing now: "<< current->getBlock()->getName();
	if (current == tree->getRoot())
	{
		current->getBlock()->setPos(0, 0, current->getBlock()->getWidth(), current->getBlock()->getHeight());

		tmp1 = new Contour(0, current->getBlock()->getX2(), current->getBlock()->getY2());
		tmp2 = new Contour(current->getBlock()->getX2(), OUTLINEINF, 0);
		dummy->setNext(tmp1); tmp1->setPrev(dummy); tmp1->setNext(tmp2); tmp1->getNext()->setPrev(tmp1);
	}
	else if (current == current->getParent()->getLeftChild()) // left child of its parent. (PUT right)
	{
		int nowX1 = current->getParent()->getBlock()->getX2();
		int nowX2 = current->getBlock()->getWidth() + nowX1;
		int nowY1 = 0;
		// nowY1 = the maximum y of [nowx1,nowX2)
		Contour* findY = dummy;
	//	cout << "\t "<< nowX1 << " " << nowX2 << endl <<"show find Y: " << endl;
		while (findY != NULL)
		{
			if ( (nowX1 >= findY->getStartX() && nowX2 <= findY->getEndX()) || ( nowX1 <= findY->getStartX() && nowX2 >= findY->getEndX()) || (nowX2 > findY->getStartX() && nowX2 < findY->getEndX()))
			{
				if (findY->getY() > nowY1)
				{
					nowY1 = findY->getY();
				}
			}

			findY = findY->getNext();
		}
		int nowY2 = nowY1 + current->getBlock()->getHeight();
		current->getBlock()->setPos(nowX1, nowY1, nowX2, nowY2);
		// reportBlockPos(current->getBlock());
		Contour* tmpNode = dummy->getNext();
		while (tmpNode != NULL) // UPDATE the contour structure.
		{
			Contour* deleteNode = NULL;
			if (tmpNode->getEndX() < nowX1 || tmpNode->getStartX() > nowX2) // Case 1 (NO cover)
			{
				// DO NOTHING
				// cout << "Case 1!!!" << endl;
			}
			else if (tmpNode->getStartX() <= nowX1 && tmpNode->getEndX() > nowX2) // Case 2 (this block is the part of the node)
			{
				// cout << "Case 2!!!" << endl;
				Contour* newContour = new Contour(nowX1, nowX2, nowY2); // insert the new contour before the tmpNode;
				tmpNode->getPrev()->setNext(newContour); newContour->setPrev(tmpNode->getPrev());
				newContour->setNext(tmpNode); tmpNode->setPrev(newContour);
				tmpNode->setStartX(nowX2);
			}
			else if (tmpNode->getStartX() >= nowX1 && tmpNode->getEndX() <= nowX2) // case 3 (totally cover)
			{
				// cout << "Case 3!!!" << endl;
				Contour* newContour = new Contour(nowX1, nowX2, nowY2);
				newContour->setPrev(tmpNode->getPrev()); newContour->getPrev()->setNext(newContour);
				Contour* tmpNext = tmpNode;
				while (tmpNext != NULL && tmpNext->getStartX() >= nowX1 && tmpNext->getEndX() <= nowX2)
				{
					tmpNext = tmpNode->getNext();
					delete tmpNode;
					tmpNode = tmpNext;
				}
				newContour->setNext(tmpNext); tmpNext->setPrev(newContour);
				newContour->getNext()->setStartX(nowX2);
			}
			tmpNode = tmpNode->getNext();
		}
	}
	else // right child of its parent.
	{
		int nowX1 = current->getParent()->getBlock()->getX1();
		int nowX2 = current->getBlock()->getWidth() + nowX1;
		int nowY1 = 0;
		// nowY1 = the maximum y of [nowx1,nowX2)
		Contour* findY = dummy;
//		cout << "\t "<< nowX1 << " " << nowX2 << endl <<"show find Y: " << endl;
		while (findY != NULL)
		{
			if ((nowX1 >= findY->getStartX() && nowX2 <= findY->getEndX()) ||( nowX1 <= findY->getStartX() && nowX2 >= findY->getEndX()) || (nowX2 > findY->getStartX() && nowX2 < findY->getEndX()))
			{
				if (findY->getY() > nowY1)
				{
					nowY1 = findY->getY();
				}
			}
			findY = findY->getNext();
		}
		int nowY2 = nowY1 + current->getBlock()->getHeight();
		current->getBlock()->setPos(nowX1, nowY1, nowX2, nowY2);
		// reportBlockPos(current->getBlock());
		Contour* tmpNode = dummy->getNext();
		while (tmpNode != NULL) // UPDATE the contour structure.
		{
			Contour* deleteNode = NULL;
			if (tmpNode->getEndX() < nowX1 || tmpNode->getStartX() > nowX2) // Case 1 (NO cover)
			{
				// DO NOTHING
				// cout << "Case 1!!!" << endl;
			}
			else if (tmpNode->getStartX() <= nowX1 && tmpNode->getEndX() > nowX2) // Case 2 (this block is the part of the node)
			{
				// cout << "Case 2!!!" << endl;
				Contour* newContour = new Contour(nowX1, nowX2, nowY2); // insert the new contour before the tmpNode;
				tmpNode->getPrev()->setNext(newContour); newContour->setPrev(tmpNode->getPrev());
				newContour->setNext(tmpNode); tmpNode->setPrev(newContour);
				tmpNode->setStartX(nowX2);
			}
			else if (tmpNode->getStartX() >= nowX1 && tmpNode->getEndX() <= nowX2) // case 3 (totally cover)
			{
				// cout << "Case 3!!!" << endl;
				Contour* newContour = new Contour(nowX1, nowX2, nowY2);
				newContour->setPrev(tmpNode->getPrev());
				newContour->getPrev()->setNext(newContour);
				Contour* tmpNext = tmpNode;
				while (tmpNext != NULL && tmpNext->getStartX() >= nowX1 && tmpNext->getEndX() <= nowX2)
				{
					tmpNext = tmpNode->getNext();
					delete tmpNode;
					tmpNode = tmpNext;
				}
				if (tmpNext != NULL)
				{
					newContour->setNext(tmpNext); tmpNext->setPrev(newContour);
					newContour->getNext()->setStartX(nowX2);
				}

			}
			tmpNode = tmpNode->getNext();
		}
	}
//	reportContour(dummy);
	if (current->getLeftChild() != NULL)
	{
		packingNode2Floorplan(dummy, current->getLeftChild(), tree);
	}
	if (current->getRightChild() != NULL)
	{
		packingNode2Floorplan(dummy, current->getRightChild(), tree);
	}
}

void Floorplanner::reportBlockPos(Block* blk)
{
	cout << blk->getName() << "\t" << blk->getX1() << "\t" << blk->getY1() << "\t" << blk->getX2() << "\t" << blk->getY2() << endl;
}

void Floorplanner::clearContours()
{
	Contour* c = dummy->getNext();
	while (c != NULL)
	{
		Contour* deleteC = c;
		c = c->getNext();
		delete deleteC;
	}
  Contour* tmp = new Contour(0, OUTLINEINF, 0);
  dummy->setNext(tmp);
}