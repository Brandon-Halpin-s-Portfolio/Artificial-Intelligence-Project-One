//============================================================================
// Name        : AI_projectOne.cpp
// Author      : Brandon Halpin
// Version     : One
// Description : Project One for Artificial Intelligence
// Notes       : This project utilized:
//					-https://www.youtube.com/watch?v=iJ-NSxH3QNc which was a tutorial on implementing
//					 custom comparators for priority queues
//
//				 One of the reasons why this program uses up so many lines of code is because I often
//               hardcode the process of converting two-dimensional arrays to strings to pass between
//               functions and vice-versa. I did this because I read that it is quite difficult in C++
//               to pass two-dimensional arrays into and from functions. Since this is my first major
//               project using C++, which I have been teaching myself, I did it this way to make it easier
//               to code
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <cstdlib>
using namespace std;

//Struct which stores the initial and goal states of the problem
//Used in the inputProcessing method
struct stringContainer {
	string initial;
	string goal;
} initialAndGoal, problem;

//Struct which stores whether a node can generate child nodes for the up, down, left and right directions
//If a blank space is in a position where going up, down, left, or right are impossible, this data structure
//will store that
struct movesViabilityStore {
	bool up;
	bool down;
	bool left;
	bool right;
};

//Struct which represents a node in the a* search algorithm
struct node{
	int originDirection;
	int state[4][4];
	node* parent;
	int gVal;
	int hVal;
	int fVal;
	bool isRoot = false;
};

//Struct which stores the data from a* search which will be passed into the main method and
//printed into a text file
struct sol{
	string moveSet;
	string funcSet;
	int depth;
	int nodeNum;
};

//Custom comparator for the frontier priority queue
//Compares the f(n) values of two nodes and returns whether the first node
//has a greater f(n) value
class nodeCompare{
public:
	int operator()(node a, node b){
		return a.fVal > b.fVal;
	}
};

//Function which calculates the sum of the Manhattan distances of a state
//Takes a string which represents the state of the node, and a string which represents the goal state
//Outputs a integer which represents the sum of Manhattan distances of the state
int calcHeuristic(string inState, string inGoal){
	int stateArr[4][4];
	int goalArr[4][4];

	//Parses the state string into a 2D array
	int iRow = 0;
	int iCol = 0;
	stringstream stateStream(inState);
	while(stateStream.good()){
		string sub;
		getline(stateStream, sub, ',');
		stateArr[iRow][iCol] = stoi(sub);

		iCol++;
		if(iCol >= 4){
			iCol = 0;
			iRow++;
		}
	}

	//Parses the goal string into a 2D array
	iRow = 0;
	iCol = 0;
	stringstream goalHeurStream(inGoal);
	while(goalHeurStream.good()){
		string sub;
		getline(goalHeurStream, sub, ',');
		goalArr[iRow][iCol] = stoi(sub);

		iCol++;
		if(iCol >= 4){
			iCol = 0;
			iRow++;
		}
	}

	//Finds the sum of the Manhattan distances of all elements of the array
	//For each element of the current state, it adds the absolute value of the difference of rows
	//to the absolute value of the difference of columns, and adds this to a running sum for all elements
	//of the array
	int cur = 0;
	int sum = 0;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			cur = stateArr[i][j];
			for(int iRowGoal = 0; iRowGoal < 4; iRowGoal++){
				for(int iColGoal = 0; iColGoal < 4; iColGoal++){
					if(goalArr[iRowGoal][iColGoal] == cur){
						sum += abs(iRowGoal - i) + abs(iColGoal - j);
					}
				}
			}
		}
	}

	return sum;
}


//Function to determine if a node contains a goal state
bool goalCheck(node inNode, string inGoal){
	int goalCheckArr[4][4];

	//Parses the goal string into a 2D array
	int iRow = 0;
	int iCol = 0;
	stringstream goalCheckStream(inGoal);
	while(goalCheckStream.good()){
		string sub;
		getline(goalCheckStream, sub, ',');
		goalCheckArr[iRow][iCol] = stoi(sub);

		iCol++;
		if(iCol >= 4){
			iCol = 0;
			iRow++;
		}
	}

	//Is the goal state the same as the state the node contains
	bool isSame = true;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(inNode.state[i][j] != goalCheckArr[i][j]){
				isSame = false;
			}
		}
	}
	return isSame;
}

//Function to determine which directions the blank space of a node can move in for the next move
movesViabilityStore movesViable(node inNode){
	movesViabilityStore viab;
	int blankRow = 0;
	int blankCol = 0;
	for(blankRow; blankRow < 4; blankRow++){
		for(blankCol; blankCol < 4; blankCol++){
			if(inNode.state[blankRow][blankCol] == 0){
				break;
			}
		}
	}

	//Is moving the blank space up viable?
	if(blankRow != 0){
		viab.up = true;
	}
	else{
		viab.up = false;
	}
	//Is moving the blank space down viable?
	if(blankRow != 3){
		viab.down = true;
	}
	else{
		viab.down = false;
	}
	//Is moving the blank space left viable?
	if(blankCol != 0){
		viab.left = true;
	}
	else{
		viab.left = false;
	}
	//Is moving the blank space right viable?
	if(blankCol != 3){
		viab.right = true;
	}
	else{
		viab.right = false;
	}

	return viab;
}

stringContainer inputProcessing(string textFile){
	//INPUT PROCESSING SECTION
	//Reads from the input file, at the end produces two arrays: one represents the initial state,
	//the other represents the goal state
	fstream inputFile(textFile);

	//Reads from the input file, separating it into two strings, one for the intial
	//state and another for the goal state

	int lineNum = 0;
	string initialStr = "";
	string goalStr = "";
	string curLine;
	if(inputFile.is_open()){
		while(getline(inputFile, curLine)){
			if(lineNum < 4){
				initialStr += curLine;
			}
			else if(lineNum > 4){
				goalStr += curLine;
			}
			lineNum++;
		}
		inputFile.close();
	}

	//Formats the initial and goal strings to remove line breaks and spaces
	for(int i = 0; i < initialStr.size(); i++){
		if(!isdigit(initialStr[i])){
			initialStr[i] = ',';
		}
	}

	string initialFormat = "";
	stringstream s_stream(initialStr);
	while(s_stream.good()){
		string sub;
		getline(s_stream, sub, ',');
		if(sub[0] != '\0'){
			initialFormat += sub + ',';
		}
	}

	for(int i = 0; i < goalStr.size(); i++){
		if(!isdigit(goalStr[i])){
			goalStr[i] = ',';
		}
	}

	string goalFormat = "";
	stringstream s2_stream(goalStr);
	while(s2_stream.good()){
		string subTwo;
		getline(s2_stream, subTwo, ',');
		if(subTwo[0] != '\0'){
			goalFormat += subTwo + ',';
		}
	}

	//Cuts off the last comma from the initial and goal strings
	//not doing so would cause problems when parsing this into an array
	initialAndGoal.initial = initialFormat.substr(0, (initialFormat.size() - 1));
	initialAndGoal.goal = goalFormat.substr(0, (goalFormat.size() - 1));

	return initialAndGoal;
	//END OF INPUT PROCESSING SECTION
}

//Function which implements the actual a* search
sol aStar(stringContainer problemPara){
	sol mySol;

	//Two-dimensional 4x4 arrays which will be used to store the initial and goal states
	int goalState[4][4];

	//Parses the goal string into a 2D array
	int iRow = 0;
	int iCol = 0;
	stringstream goalStream(problemPara.goal);
	while(goalStream.good()){
		string sub;
		getline(goalStream, sub, ',');
		goalState[iRow][iCol] = stoi(sub);

		iCol++;
		if(iCol >= 4){
			iCol = 0;
			iRow++;
		}
	}

	//Parses the initial string into a node
	node root;
	iRow = 0;
	iCol = 0;
	stringstream initialStream(problemPara.initial);
	while(initialStream.good()){
		string sub;
		getline(initialStream, sub, ',');
		root.state[iRow][iCol] = stoi(sub);
		iCol++;
		if(iCol >= 4){
			iCol = 0;
			iRow++;
		}
	}

	//Sets the parameters of the root node
	root.parent = NULL;
	root.gVal = 0;
	root.hVal = calcHeuristic(problemPara.initial, problemPara.goal);
	root.fVal = root.gVal + root.hVal;
	root.isRoot = true;

	//The frontier of A* search, represented by a priority queue
	//Priority is represented by the f(n) values of the nodes
	std::priority_queue<node, vector<node>, nodeCompare> frontier;

	//The list of explored states, represented by a vector of nodes
	vector<string> explored;

	//List of all addresses which store nodes
	//This is so it will be deleted later to avoid a stack overflow
	vector<node*> addresses;

	//Push the root node into the frontier and its state into the explored list
	frontier.push(root);
	explored.push_back(problemPara.initial);

	//Keeps the total number of lists, starting with one (the root node)
	int totalNodes = 1;

	node myTop;
	while(!frontier.empty()){
		//Pops the top node from the frontier and stores it
		node* myTop = new node();
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				myTop->state[i][j] = (frontier.top().state[i][j]);
			}
		}
		myTop->gVal = (frontier.top().gVal);
		myTop->hVal = (frontier.top().hVal);
		myTop->fVal = (frontier.top().fVal);
		myTop->parent = (frontier.top().parent);
		myTop->originDirection = (frontier.top().originDirection);
		frontier.pop();

		//If the top node of the frontier is not a goal node
		//See if the blank space (as represented in the node) can move up, down, left and right, and if so
		//generate child nodes to represent these moves
		if(!goalCheck(*myTop, problemPara.goal)){
			//Determine where the blank space is
			int blankRow;
			int blankCol;
			for(int iterate = 0; iterate < 4; iterate++){
				for(int jIterate = 0; jIterate < 4; jIterate++){
					if(myTop->state[iterate][jIterate] == 0){
						blankRow = iterate;
						blankCol = jIterate;
					}
				}
			}

			//Stores what moves are viable
			movesViabilityStore myVia = movesViable(*myTop);
			//If "up" is a viable move
			int temp;
			if(myVia.up){
				//Create new node
				node* up = new node();
				//Sets up the state of the node
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						up->state[i][j] = myTop->state[i][j];
					}
				}
				temp = up->state[blankRow - 1][blankCol];
				up->state[blankRow - 1][blankCol] = 0;
				up->state[blankRow][blankCol] = temp;

				//Converts state of the new node into a string
				string myStateStringUp = "";
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						myStateStringUp += to_string(up->state[i][j]) + ',';
					}
				}
				string myStateStringUpFormat = myStateStringUp.substr(0, (myStateStringUp.size() - 1));

				//Sets up the paremeter of the new node
				up->originDirection = 0;
				up->parent = myTop;
				up->gVal = myTop->gVal + 1;
				up->hVal = calcHeuristic(myStateStringUpFormat, problemPara.goal);
				up->fVal = up->gVal + up->hVal;

				//Is the state in the explored list?
				//Has this state already been generated by a* search?
				bool isInExplored = false;
				for(int i = 0; i < explored.size(); i++){
					if((explored.at(i)).compare(myStateStringUpFormat) == 0){
						isInExplored = true;
					}
				}
				//If the state has not been previously explored, add it to the frontier, add its state to the explored list,
				//add the memory address to the list of addresses, and increment the total number of nodes
				if(!isInExplored){
					addresses.push_back(up);
					frontier.push(*up);
					explored.push_back(myStateStringUpFormat);
					totalNodes++;
				}

			}
//			//Adding a node to represent moving the blank space down
			if(myVia.down){
				//Create new node
				node* down = new node();
				//Sets up the state of the node
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						down->state[i][j] = myTop->state[i][j];
					}
				}
				temp = down->state[blankRow + 1][blankCol];
				down->state[blankRow + 1][blankCol] = 0;
				down->state[blankRow][blankCol] = temp;

				//Converts state of the new node into a string
				string myStateStringDown = "";
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						myStateStringDown += to_string(down->state[i][j]) + ',';
					}
				}
				string myStateStringDownFormat = myStateStringDown.substr(0, (myStateStringDown.size() - 1));

				//Sets up the parameter of the node
				down->originDirection = 1;
				down->parent = myTop;
				down->gVal = myTop->gVal + 1;
				down->hVal = calcHeuristic(myStateStringDownFormat, problemPara.goal);
				down->fVal = down->gVal + down->hVal;

				//Has the state already been explored?
				bool isInExplored = false;
				for(int i = 0; i < explored.size(); i++){
					if((explored.at(i)).compare(myStateStringDownFormat) == 0){
						isInExplored = true;
					}
				}
				//If the node has not already been explored, add it to the frontier, add its state to the explored list,
				//add the memory address to the list of addresses, and increment the total number of nodes
				if(!isInExplored){
					addresses.push_back(down);
					frontier.push(*down);
					explored.push_back(myStateStringDownFormat);
					totalNodes++;
				}

			}
     		//Adding a node to represent moving the blank space left
			if(myVia.left){
				//Generate a new node
				node* left = new node();
				//Sets up the state of the node
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						left->state[i][j] = myTop->state[i][j];
					}
				}
				temp = left->state[blankRow][blankCol - 1];
				left->state[blankRow][blankCol - 1] = 0;
				left->state[blankRow][blankCol] = temp;

				//Converts state of the new node into a string
				string myStateStringLeft = "";
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						myStateStringLeft += to_string(left->state[i][j]) + ',';
					}
				}
				string myStateStringLeftFormat = myStateStringLeft.substr(0, (myStateStringLeft.size() - 1));

				//Sets up the parameters of the node
				left->originDirection = 2;
				left->parent = myTop;
				left->gVal = myTop->gVal + 1;
				left->hVal = calcHeuristic(myStateStringLeftFormat, problemPara.goal);
				left->fVal = left->gVal + left->hVal;

				//Is the state in the explored list?
				bool isInExplored = false;
				for(int i = 0; i < explored.size(); i++){
					if((explored.at(i)).compare(myStateStringLeftFormat) == 0){
						isInExplored = true;
					}
				}
				//If the state has not been previously explored, add it to the explored list, add the node to the frontier,
				//add the memory address to the list of addresses, and increment the total number of nodes
				if(!isInExplored){
					addresses.push_back(left);
					frontier.push(*left);
					explored.push_back(myStateStringLeftFormat);
					totalNodes++;
				}

			}
			//Adding a node to represent moving the blank space right
			if(myVia.right){
				//Generate a new node
				node* right = new node();
				//Sets up the state
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						right->state[i][j] = myTop->state[i][j];
					}
				}
				temp = right->state[blankRow][blankCol + 1];
				right->state[blankRow][blankCol + 1] = 0;
				right->state[blankRow][blankCol] = temp;

				//Converts state of the new node into a string
				string myStateStringRight = "";
				for(int i = 0; i < 4; i++){
					for(int j = 0; j < 4; j++){
						myStateStringRight += to_string(right->state[i][j]) + ',';
					}
				}
				string myStateStringRightFormat = myStateStringRight.substr(0, (myStateStringRight.size() - 1));

				//Sets up the parameters of the node
				right->originDirection = 3;
				right->parent = myTop;
				right->gVal = myTop->gVal + 1;
				right->hVal = calcHeuristic(myStateStringRightFormat, problemPara.goal);
				right->fVal = right->gVal + right->hVal;

				//Has the state been explored?
				bool isInExplored = false;
				for(int i = 0; i < explored.size(); i++){
					if((explored.at(i)).compare(myStateStringRightFormat) == 0){
						isInExplored = true;
					}
				}
				//If the state has not been explored, add it to the explored list, add the node to the frontier, add the memory
				//address to the list of addresses, and increment the total number of nodes
				if(!isInExplored){
					addresses.push_back(right);
					frontier.push(*right);
					explored.push_back(myStateStringRightFormat);
					totalNodes++;
				}
			}
		}
		else{
			//This code runs once a goal node has been generated

			//Take the g(n) value of the top node. This is the depth of the shallowest goal node, as
			//asked for in the specifications of the assignment
			mySol.depth = myTop->gVal;

			//Create a node to store the information of the top node in the frontier
			node* end = new node();
			end->parent = myTop->parent;
			end->gVal = myTop->gVal;
			end->hVal = myTop->hVal;
			end->fVal = myTop->fVal;
			end->originDirection = myTop->originDirection;

			//Create lists which will store the moves made to reach the goal node
			//and the f(n) values of the nodes
			vector<char> moveReversed;
			vector<int> funcStore;

			//Starting from the current node, keep a list of the moves taken to get from the initial state to
			//the goal state, as well as the f(n) value of each node. This will be backwards (we start from the goal node and go back to the root) but will
			//be reversed to the proper order later
			while(end->parent != nullptr){
				if(end->originDirection == 0){
					moveReversed.push_back('U');
				}
				else if(end->originDirection == 1){
					moveReversed.push_back('D');
				}
				else if(end->originDirection == 2){
					moveReversed.push_back('L');
				}
				else if(end->originDirection == 3){
					moveReversed.push_back('R');
				}
				funcStore.push_back(end->fVal);
				end = end->parent;
			}

			//Clear up all of the memory that was used to store the nodes
			//Not doing this will lead to a stack overflow
			int size = addresses.size();
			for(int i = 0; i < size; i++){
				delete (node*) addresses.at(i);
			}

			//Push the f(n) value of the root onto the list
			funcStore.push_back(root.fVal);

			//Reverse the order of the lists of the f(n) values of the nodes and the
			// moves taken to get from the initial state to the goal state so they read in the correct order
			string funcStr = "";
			for(int i = (funcStore.size() - 1); i >= 0; i--){
				funcStr += to_string(funcStore.at(i)) + ' ';
			}
			string moves = "";
			for(int i = (moveReversed.size() - 1); i >= 0; i--){
				moves += moveReversed.at(i);
				moves += ' ';
			}

			//Set the list of moves, the list of f(n) values, and the total number of nodes generated
			//as parameters of a custom data structure, which will be returned to the main method
			mySol.funcSet = funcStr;
			mySol.moveSet = moves;
			mySol.nodeNum = totalNodes;
			break;
		}
	}

	return mySol;
}

int main(int argc, char* argv[]){

	//Reads the argument given to the program and sets the corresponding input file to it
	string inputName = argv[1];

	//Reads from the input file and stores the initial state and goal state of the problem
	problem = inputProcessing(inputName);

	//Uses the initial and goal states to solve the problem using a* search
	sol solution = aStar(problem);

	//Creates an output file and writes to it
	//If an input file is given and is in the format "Input<Number>", use the number
	//to name the output file. Otherwise name the output file "aStarResults.txt"
	string outputName = "";
	size_t fIndex = inputName.find("Input");
	if(fIndex == string::npos){
		outputName = "aStarResults.txt";
	}
	else if((static_cast<int>(fIndex) + 5) != (inputName.size() - 1)){
		int fIndexInt = static_cast<int>(fIndex);
		char addOn = inputName[fIndexInt + 5];
		outputName = "Output";
		outputName.push_back(addOn);
		outputName += ".txt";
	}


	ofstream output;
	output.open(outputName);

	//Outputs the original initial and goal states to the output file
	stringstream initialPrint(problem.initial);
	int count = 0;
	while(initialPrint.good()){
		string sub;
		getline(initialPrint, sub, ',');
		output << sub + ' ';
		count++;
		if(count >= 4){
			count = 0;
			output << endl;
		}
	}
	output << endl;
	stringstream goalPrint(problem.goal);
	int goalCount = 0;
	while(goalPrint.good()){
		string sub;
		getline(goalPrint, sub, ',');
		output << sub + ' ';
		goalCount++;
		if(goalCount >= 4){
			goalCount = 0;
			output << endl;
		}
	}
	output << endl;

	//Ouputs the following information to the output file
	//Depth of the shallowest goal node
	//The total number of nodes generated
	//The set of moves taken to get from the initial state to the goal state
	//The f(n) value of each node from the root node to the goal node
	output << solution.depth << endl;
	output << solution.nodeNum << endl;
	output << solution.moveSet << endl;
	output << solution.funcSet << endl;

	//Completes writing to the output file
	output.close();
	return 0;
}
