/* pathfinder.cpp
 * ---------------
 * A starter file with some starting constants and handy utility routines.
 */
 
#include "genlib.h"
#include "extgraph.h"
#include "set.h"
#include "map.h"
#include "stack.h"
#include "pqueue.h"
#include <iostream>
#include <fstream>
#include <cmath>

/* Constants
 * --------
 * A few program-wide constants concerning the graphical display.
 */
const double CircleRadius =.05;     	// the radius of a node
const int LabelFontSize = 9;          // for node name labels


/* Type: coordT
 * ------------
 * Just a simple struct to handle a pair of x,y coordinate values.
 */
struct coordT {
	double x, y;
};

struct arcT;

struct nodeT {
    string name;
    coordT position;
    
    Set<arcT *> arcs;
};

struct arcT {
    double cost;
    
    nodeT *start;
    nodeT *finish;
};

/* Function: DrawFilledCircleWithLabel
 * Usage:  DrawFilledCircleWithLabel(center, "Green", "You are here");
 * -------------------------------------------------------------------
 * Uses facilities from extgraph to draw a circle filled with
 * color specified. The circle is centered at the given coord has the
 * specified radius.  A label is drawn to the right of the circle.
 * You can leave off the last argument if no label is desired.
 */
void DrawFilledCircleWithLabel(coordT center, std::string color, std::string label = "")
{
	MovePen(center.x + CircleRadius, center.y);
	SetPenColor(color);
	StartFilledRegion(1.0);
	DrawArc(CircleRadius, 0, 360);
	EndFilledRegion();
	if (!label.empty()) {
		MovePen(center.x + CircleRadius, center.y);
		SetFont("Helvetica");
		SetPointSize(LabelFontSize);
		DrawTextString(label);
	}
}

/* Function: DrawLineBetween
 * Usage:  DrawLineBetween(coord1, coord2, "Black");
 * -------------------------------------------------
 * Uses facilities from extgraph to draw a line of the
 * specified color between the two given coordinates.
 */
void DrawLineBetween(coordT start, coordT end, string color)
{
	SetPenColor(color);
	MovePen(start.x, start.y);
	DrawLine(end.x - start.x, end.y - start.y);
}


/* Function: GetMouseClick
 * Usage:  loc = GetMouseClick();
 * ------------------------------
 * Waits for the user to click somewhere on the graphics window
 * and returns the coordinate of where the click took place.
 */
coordT GetMouseClick()
{
	coordT where;
	WaitForMouseDown();
	WaitForMouseUp();
	where.x = GetMouseX();
	where.y = GetMouseY();
	return where;
}

/* Function: WithinDistance
 * Usage:  if (WithinDistance(click, pt))...
 * -----------------------------------------
 * Returns true if the Cartesian distance between the two coordinates
 * is <= the specified distance, false otherwise. If not specified,
 * the distance is assumed to be size of the node diameter on screen.
 */
bool WithinDistance(coordT pt1, coordT pt2, double maxDistance = CircleRadius*2)
{
	double dx = pt2.x - pt1.x;
	double dy = pt2.y - pt1.y;
	double distance = sqrt(dx*dx + dy*dy);
	return (distance <= maxDistance);
}



void clearData(string &imageName, Map<nodeT *> &graph) {
    imageName = "";
    graph.clear();
}

string getFilePath(string filename) {
    string bundlePath = "./Assignment7.app/Contents/Resources/";
    return bundlePath + filename;
}

enum readGraphModeT {
    Mark,
    Image,
    Nodes,
    Arcs
};

bool isNext(fstream &file, string str) {
    int i = 0;
    bool isNext = false;
    for (i = 0; i < str.length(); i++) {
        int nextCh = file.get();
        if (nextCh == EOF) {
            isNext = false;
            break;
        }
        if (str[i] == (char)nextCh) {
            isNext = true;
        } else {
            isNext = false;
            break;
        }
    }
    if (isNext) {
        return true;
    } else {
        for (int j = i; j >= 0; j--) {
            file.unget();
        }
        return false;
    }
}

bool isNextArcs(fstream &file) {
    return isNext(file, "\rARCS") || isNext(file, "\nARCS");
}

void addArc(Map<nodeT *> &graph, string startNodeName, string finishNodeName, double cost) {
    nodeT *startNode = graph[startNodeName];
    nodeT *finishNode = graph[finishNodeName];
    
    arcT *arc = new arcT();
    arc->start = startNode;
    arc->finish = finishNode;
    arc->cost = cost;
    
    startNode->arcs.add(arc);
}

void readGraph(string filepath, string &imageName, Map<nodeT *> &graph) {
    fstream file;
    file.open(filepath.c_str());
    readGraphModeT mode = Image;
    while (file.peek() != EOF) {
        if (mode == Image) {
            file >> imageName;
            mode = Mark;
        } else if (mode == Mark) {
            string mark;
            file >> mark;
            if (mark == "NODES") {
                mode = Nodes;
            } else {
                Error("Invalid read state: Expecting valid mark");
            }
        } else if (mode == Nodes) {
            string nodeName;
            double x, y;
            file >> nodeName >> x >> y;
            
            nodeT *node = new nodeT();
            node->name = nodeName;
            node->position = {x, y};
            graph.add(nodeName, node);
            
            if (isNextArcs(file)) {
                mode = Arcs;
            }
        } else if (mode == Arcs) {
            string startNodeName;
            string finishNodeName;
            double cost;
            file >> startNodeName >> finishNodeName >> cost;
            if (!file.fail()) {
                // last line read twice somehow
                // so, check for fail state here
                
                addArc(graph, startNodeName, finishNodeName, cost);
                addArc(graph, finishNodeName, startNodeName, cost);
            }
        } else {
            Error("Invalid read state: State not defined");
        }
    }
    file.close();
}

void clearScreen() {
    MovePen(0, 0);
    SetPenColor("White");
    double w = GetWindowWidth();
    double h = GetWindowHeight();
    StartFilledRegion(1);
    DrawLine(w, 0);
    DrawLine(0, h);
    DrawLine(-w, 0);
    DrawLine(0, -h);
    EndFilledRegion();
}

void DrawArc(arcT *arc) {
    DrawLineBetween(arc->start->position, arc->finish->position, "Black");
}

void DrawNode(string key, nodeT *node) {
    node->arcs.mapAll(DrawArc);
    
    DrawFilledCircleWithLabel(node->position, "Blue", node->name);
    //Pause(0.5);
}

void drawGraph(string imageName, Map<nodeT *> &graph) {
    clearScreen();
    // following function do not work properly with last version of Xcode...
    //DrawNamedPicture(imageName);
    
    graph.mapAll(DrawNode);
}

nodeT *getClosestNode(Map<nodeT *> &graph, coordT position) {
    Map<nodeT *>::Iterator itrNodes = graph.iterator();
    while (itrNodes.hasNext()) {
        string key = itrNodes.next();
        nodeT *node = graph[key];
        if (WithinDistance(node->position, position)) {
            return node;
        }
    }
    return NULL;
}

nodeT *getNodeByClick(Map<nodeT *> &graph) {
    nodeT *node = NULL;
    while (node == NULL) {
        node = getClosestNode(graph, GetMouseClick());
        if (node != NULL) {
            break;
        }
    }
    return node;
}

void selectPathEnds(Map<nodeT *> &graph, nodeT *&startNode, nodeT *&finishNode) {
    startNode = getNodeByClick(graph);
    finishNode = NULL;
    while (true) {
        finishNode = getNodeByClick(graph);
        if (finishNode != startNode) {
            break;
        }
    }
}

double length(Stack<arcT *> path) {
    double length = 0;
    while (!path.isEmpty()) {
        length += path.pop()->cost;
    }
    return length;
}

int PathCmp(Stack<arcT *> pathOne, Stack<arcT *> pathTwo) {
    double one = length(pathOne);
    double two = length(pathTwo);
    
    if (one == two) return 0;
    else if (one < two) return 1;
    else return -1;
}

bool containsNode(Stack<arcT *> path, nodeT *node) {
    while (!path.isEmpty()) {
        arcT *arc = path.pop();
        if (arc->start == node || arc->finish == node) {
            return true;
        }
    }
    return false;
}

void print(Stack<arcT *> path) {
    while (!path.isEmpty()) {
        arcT *arc = path.pop();
        if (arc->finish != arc->start) {
            cout << arc->finish->name << " <- " << arc->start->name;
        }
        if (path.size() > 1) {
            cout << ", ";
        }
    }
    cout << endl;
}

Stack<arcT *> findShortestPath(Map<nodeT *> &graph, nodeT *startNode, nodeT *finishNode) {
    Map<double> pathLengths;
    
    arcT *emptyArc = new arcT();
    emptyArc->start = startNode;
    emptyArc->finish = startNode;
    emptyArc->cost = 0;
    
    Stack<arcT *> initialPath;
    initialPath.push(emptyArc);
    
    PQueue<Stack<arcT *>> queue(PathCmp);
    queue.enqueue(initialPath);
    
    Stack<arcT *> shortestPath = Stack<arcT *>();
    while (!queue.isEmpty()) {
        Stack<arcT *> path = queue.dequeueMax();
        cout << "Dequeued with cost " << length(path) << " ";
        print(path);
        
        nodeT *pathFinishNode = path.peek()->finish;
        bool isFoundPath = (pathFinishNode == finishNode);
        bool isPathOptimal = (shortestPath.isEmpty() || length(path) < length(shortestPath));
        if (isFoundPath && isPathOptimal) {
            shortestPath = path;
        }
        
        Set<arcT *> arcs = pathFinishNode->arcs;
        Set<arcT *>::Iterator itrArcs = arcs.iterator();
        while (itrArcs.hasNext()) {
            arcT *arc = itrArcs.next();
            nodeT *nextNode = arc->finish;
            cout << "Next " << nextNode->name << " with cost " << arc->cost << endl;
            
            Stack<arcT *> newPath = path;
            newPath.push(arc);
            double newPathLength = length(newPath);
            
            if (!containsNode(path, nextNode) && (!pathLengths.containsKey(nextNode->name) || newPathLength < pathLengths[nextNode->name])) {
                cout << "New path ";
                print(newPath);
                
                queue.enqueue(newPath);
                pathLengths[nextNode->name] = newPathLength;
            }
        }
    }
    return shortestPath;
}

int main()
{
	InitGraphics();
	SetWindowTitle("CS106 Pathfinder");
    cout << "This masterful piece of work is a graph extravaganza!" << endl
        << "The main attractions include a lovely visual presentation of the graph" << endl
        << "along with an implementation of Dijkstra's shortest path algorithm and" << endl
        << "the computation of a minimal spanning tree.  Enjoy!" << endl;
    
    string imageName;
    Map<nodeT *> graph;
    
    clearData(imageName, graph);
    
    string filename = "USA.txt";
    string filepath = getFilePath(filename);
    
    readGraph(filepath, imageName, graph);
    
    drawGraph(imageName, graph);
    
//    nodeT *startNode = NULL;
//    nodeT *finishNode = NULL;
//    selectPathEnds(graph, startNode, finishNode);
//    cout << "Path from " << startNode->name << " to " << finishNode->name << endl;
    
    nodeT *startNode = graph["Orlando"];
    nodeT *finishNode = graph["Portland"];
    
    Stack<arcT *> shortestPath = findShortestPath(graph, startNode, finishNode);
    print(shortestPath);
    cout << "Shortest path length: " << length(shortestPath) << endl;
    
    return (0);
}