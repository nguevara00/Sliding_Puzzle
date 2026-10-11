
#include <iostream>
#include <vector>
#include <string>
#include <limits>

#include <queue>
#include <ctime>
#include <stack>

// new includes for phase 2
#include <set>
#include <map>
#include <cstdlib>
#include <unordered_map> //for lookup table
#include <algorithm> //provides max()

using namespace std;

using Board = vector<vector<int>>;

struct Node {
    Board state;
    int depth;
    string moves;
    int blankRow;
    int blankCol;

    // This allows us to prevent immediately returning to the
    // parent when generating successors.
    //
    // For the root node, both values are -1.
    int parentBlankRow;
    int parentBlankCol;

    Node() : depth(0), moves(""), blankRow(-1), blankCol(-1), parentBlankRow(-1), parentBlankCol(-1) {}

    Node(const Board& board,
         int nodeDepth,
         const string& moveSequence,
         int row,
         int col,
         int parentRow = -1,
         int parentCol = -1)
        : state(board),
          depth(nodeDepth),
          moves(moveSequence),
          blankRow(row),
          blankCol(col),
          parentBlankRow(parentRow),
          parentBlankCol(parentCol)
          {}
};

struct SearchResult {
    bool solutionFound = false;
    int numberOfMoves = 0;
    string moveSequence = "";
    long long statesRemoved = 0;
    size_t maxFrontierSize = 0;
    double cpuTime = 0.0;
};

//nodes to go in the priority queue
struct AStarEntry {
    Node node;
    int f;
    long long insertionOrder;
};

//compares two nodes in the priority queue and sorts first by f, next by alphabetical
struct AStarEntryCompare {
    bool operator()(const AStarEntry& a, const AStarEntry& b) const {
        if (a.f != b.f) {
            return a.f < b.f;
        }
        return a.insertionOrder < b.insertionOrder;
    }
};

// The ordered set supports both removal of the minimum-f node and
// updating an existing Open entry when Improve finds a better path.
struct AStarData {
    using OpenSet = set<AStarEntry, AStarEntryCompare>;
    OpenSet open;
    map<Board, OpenSet::iterator> openPositions;
    set<Board> closed;
    map<Board, int> bestG;
    long long nextInsertionOrder = 0;
};

Board readBoard(const string& prompt);
void printBoard(const Board& board);
void printResult(const string& algorithmName, const SearchResult& result);

Node createRootNode(const Board& board);
bool findBlank(const Board& board, int& row, int& col);
vector<Node> generateSuccessors(const Node& node);

SearchResult BFS(const Board& start, const Board& goal);
SearchResult DLS(const Board& start, const Board& goal, int depthLimit);
SearchResult IDS(const Board& start, const Board& goal);

//new functions for phase 2

int manhattanDistance(const Board& state, const Board& goal); //function to calculate heuristic 1
void insertIntoOpen(AStarData& data, const Node& node, int f);

void Improve(const Node& u, const Node& v, const Board& goal, AStarData& data, int choice, const unordered_map<string, int>& table, AStarData& data);

SearchResult AStar(const Board& start, const Board& goal, int choice, const unordered_map<string, int>& table);

Board colorBoard(const Board& board);
string colorBoardKey(const Board& board);

void DLSForTable(const Board& mappedGoal, int depthLimit,
                 unordered_map<string, int>& table);

void IDSForTable(const Board& goal,
                 unordered_map<string, int>& table);

int heuristic2Distance(const Board& state,
                       const unordered_map<string, int>& table);

int heuristicValue(const Board& state, const Board& goal,
                   int choice, const unordered_map<string, int>& table);

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main()
{
    cout << "=============================================\n";
    cout << "       Sliding Tile Puzzle Solver\n";
    cout << "=============================================\n\n";

    // Read start and goal states.
    Board start = readBoard(
        "Enter the START state in the format:\n"
        "[[1,2,3,4],[5,6,7,8],[9,10,11,12],[13,14,15,0]]\n"
        "Start state: "
    );

    Board goal = readBoard(
        "\nEnter the GOAL state in the same format:\n"
        "Goal state: "
    );

    cout << "\nStart state:\n";
    printBoard(start);

    cout << "\nGoal state:\n";
    printBoard(goal);

    unordered_map<string, int> heuristicTable;
    bool tableBuilt = false;

    string selection;

    while (true) {
        cout << "\n=============================================\n";
        cout << "Choose an algorithm:\n";
        cout << "1. Best First Search - h1(s)\n";
        cout << "2. Best First Search - h2(s)\n";
        cout << "3. Best First Search - h3(s)\n";
        cout << "4. Halt\n";
        cout << "=============================================\n";
        cout << "Selection: ";

        if (!(cin >> selection)){
            return 0;
        }

        if (selection == "4"){
            break;
        }
        
        bool selected[4] = {false,false,false,false};
        bool valid = true;

        for (char digit : selection) {
            if (digit < '1' || digit > '3') {
                valid = false;
                break;
            }

            int index = digit - '0';

            if (selected[index]) {
                valid = false;
                break;
            }

            selected[index] = true;
        }
        
        if (!valid) {
            cout << "Invalid algorithm selection.\n";
            continue;
        }

         if ((selected[2] || selected[3]) && !tableBuilt) {
            IDSForTable(goal, heuristicTable);
            tableBuilt = true;
        }

        cout << '\n';

        for (char digit : selection) {
            int choice = digit - '0';

            SearchResult result = AStar(start, goal, choice, heuristicTable);

            if (choice == 1) {
                printResult("Best First Search - Heuristic 1 (Manhattan distance)", result);
            }
            else if (choice == 2) {
                printResult("Best First Search - Heuristic 2 (Red/Green heuristic)", result);
            }
            else {
                printResult("Best First Search - Heuristic 3 (Maximum of H1 and H2)", result);
            }

            cout << '\n';
        }
    }

    return 0;
}

Board readBoard(const string& prompt) {
    cout << prompt;

    Board board(4, vector<int>(4));

    /*
        Input is expected in this form:

        [[1,2,3,4],[5,6,7,8],[9,10,11,12],[13,14,15,0]]

        We ignore punctuation characters and extract only the
        sixteen integer values.
    */

    char ch;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            // Ignore everything until the next digit or minus sign.
            while (cin.peek() != EOF && !isdigit(cin.peek()) && cin.peek() != '-') {
                cin.get(ch);
            }

            cin >> board[row][col];
        }
    }

    // Remove any punctuation left at the end of the input line.
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    return board;
}

void printBoard(const Board& board) {
    for (const auto& row : board) {
        cout << "[ ";

        for (int value : row) {
            cout << value << '\t';
        }

        cout << "]\n";
    }
}

void printResult(const string& algorithmName, const SearchResult& result) {
    cout << "=============================================\n";
    cout << algorithmName << " Results\n";
    cout << "=============================================\n";

    if (result.solutionFound) {
        cout << "The solution found.\n";
        cout << "The number of moves: " << result.numberOfMoves << '\n';

        cout << "The sequence of moves: " << result.moveSequence << '\n';
    } else {
        cout << "The solution was not found.\n";
        cout << "The number of moves: 0\n";
        cout << "The sequence of moves: N/A\n";
    }

    cout << "The number of states removed: " << result.statesRemoved << '\n';
    cout << "The maximum size of the queue (stack): " << result.maxFrontierSize << '\n';
    cout << "CPU time: " << result.cpuTime << " seconds\n";
    cout << "=============================================\n";
}

bool findBlank(const Board& board, int& row, int& col) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (board[r][c] == 0) {
                row = r;
                col = c;
                return true;
            }
        }
    }

    row = -1;
    col = -1;

    return false;
}

Node createRootNode(const Board& board) {
    int blankRow;
    int blankCol;

    findBlank(board, blankRow, blankCol);

    return Node(
        board,       // puzzle state
        0,           // root depth
        "",          // no moves yet
        blankRow,
        blankCol,
        -1,          // root has no parent
        -1
    );
}

vector<Node> generateSuccessors(const Node& node) {
    vector<Node> successors;

    const int rowChange[4] = {-1, 1, 0, 0};
    const int colChange[4] = {0, 0, -1, 1};
    const char moveChar[4] = {'U', 'D', 'L', 'R'};

    for (int i = 0; i < 4; ++i) {
        int newRow = node.blankRow + rowChange[i];
        int newCol = node.blankCol + colChange[i];

        // Skip moves that go outside the 4x4 board.
        if (newRow < 0 || newRow >= 4 || newCol < 0 || newCol >= 4) {
            continue;
        }

        // Do not immediately return to the parent state.

        if (newRow == node.parentBlankRow && newCol == node.parentBlankCol) {
            continue;
        }

        Board newState = node.state;

        // Slide the adjacent tile into the blank position.
        std::swap(newState[node.blankRow][node.blankCol], newState[newRow][newCol]);

        // Create the child node.
        Node child(
            newState,
            node.depth + 1,
            node.moves + moveChar[i],
            newRow,
            newCol,
            node.blankRow,
            node.blankCol
        );

        successors.push_back(child);
    }

    return successors;
}

// ------------------------------------------------------------
// Heuristic 1: Sum of Manhattan distances of nonblank tiles
// from their positions in the user-supplied goal board.
// ------------------------------------------------------------
int manhattanDistance(const Board& state, const Board& goal) {
    int totalDistance = 0;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int tile = state[row][col];
            if (tile == 0) {
                continue; // The blank tile contributes no distance.
            }

            // Find this tile in the goal board.
            for (int goalRow = 0; goalRow < 4; ++goalRow) {
                for (int goalCol = 0; goalCol < 4; ++goalCol) {
                    if (goal[goalRow][goalCol] == tile) {
                        totalDistance += abs(row - goalRow) + abs(col - goalCol);
                    }
                }
            }
        }
    }

    return totalDistance;
}

// Put a new or improved entry into A*'s Open set.
void insertIntoOpen(AStarData& data, const Node& node, int f) {
    AStarEntry entry{node, f, data.nextInsertionOrder++};
    auto inserted = data.open.insert(entry);
    data.openPositions[node.state] = inserted.first;
}

// ------------------------------------------------------------
// Improve(u, v): w(u, v) = 1 move for the sliding tile puzzle.
// The generated Node v already stores parent information in
// its moves string and parentBlankRow / parentBlankCol fields.
// ------------------------------------------------------------
void Improve(const Node& u, const Node& v, const Board& goal, int choice,  const unordered_map<string, int>& table, AStarData& data) {
    int tentativeG = u.depth + 1;
    auto openEntry = data.openPositions.find(v.state);

    if (openEntry != data.openPositions.end()) {
        // v is already in Open.
        if (tentativeG < data.bestG[v.state]) {
            // Remove its old f value, then reinsert with the improved path.
            data.open.erase(openEntry->second);
            data.openPositions.erase(openEntry);
            data.bestG[v.state] = tentativeG;
            insertIntoOpen(data, v, tentativeG + heuristicValue(v.state, goal, choice, table));
        }
    }
    else if (data.closed.count(v.state) != 0) {
        // v was expanded earlier; reopen it only if the path is shorter.
        if (tentativeG < data.bestG[v.state]) {
            data.closed.erase(v.state);
            data.bestG[v.state] = tentativeG;
            insertIntoOpen(data, v, tentativeG + heuristicValue(v.state, goal, choice, table));
        }
    }
    else {
        // v has never been discovered.
        data.bestG[v.state] = tentativeG;
        insertIntoOpen(data, v, tentativeG + heuristicValue(v.state, goal, choice, table));
    }
}

// ------------------------------------------------------------
// A* best-first search.  Open has exactly one active entry per
// board state, and Closed contains expanded board states.
// ------------------------------------------------------------
SearchResult AStar(const Board& start, const Board& goal, int choice, const unordered_map<string, int>& table) {
    SearchResult result;
    clock_t startTime = clock();
    AStarData data;

    Node root = createRootNode(start);
    data.bestG[root.state] = 0;

    insertIntoOpen(data, root, heuristicValue(root.state, goal, choice, table));
    result.maxFrontierSize = data.open.size();

    while (!data.open.empty()) {
        // Remove the minimum-f node from Open and insert it into Closed.
        auto first = data.open.begin();
        Node current = first->node;

        data.openPositions.erase(current.state);
        data.open.erase(first);
        data.closed.insert(current.state);

        ++result.statesRemoved;

        if (current.state == goal) {
            result.solutionFound = true;
            result.numberOfMoves = current.depth;
            result.moveSequence = current.moves;
            result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
            return result;
        }

        // Reuse the existing U, D, L, R successor generation function.
        vector<Node> successors = generateSuccessors(current);

        for (const Node& child : successors) {
            Improve(current, child, goal, choice, table, data);
        }

        if (data.open.size() > result.maxFrontierSize) {
            result.maxFrontierSize = data.open.size();
        }
    }

    result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    return result;
}


Board colorBoard(const Board& board) {
    Board colored = board;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (colored[row][col] >= 1 && colored[row][col] <= 8) {
                colored[row][col] = 1;
            }
            else if (colored[row][col] >= 9 && colored[row][col] <= 15) {
                colored[row][col] = 2;
            }
        }
    }

    return colored;
}

string colorBoardKey(const Board& board) {
    string key;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            key += char('0' + board[row][col]);
        }
    }

    return key;
}

void DLSForTable(const Board& mappedGoal, int depthLimit,
                 unordered_map<string, int>& table) {
    stack<Node> frontier;
    unordered_map<string, int> visitedDepth;

    frontier.push(createRootNode(mappedGoal));

    while (!frontier.empty()) {
        Node current = frontier.top();
        frontier.pop();

        string key = colorBoardKey(current.state);
        auto visited = visitedDepth.find(key);

        if (visited != visitedDepth.end() &&
            visited->second <= current.depth) {
            continue;
        }

        visitedDepth[key] = current.depth;

        if (table.find(key) == table.end()) {
            table[key] = current.depth;
        }

        if (current.depth >= depthLimit) {
            continue;
        }

        vector<Node> successors = generateSuccessors(current);

        for (auto it = successors.rbegin();
             it != successors.rend(); ++it) {
            frontier.push(*it);
        }
    }
}

void IDSForTable(const Board& goal,
                 unordered_map<string, int>& table) {
    Board mappedGoal = colorBoard(goal);
    table.clear();

    int depthLimit = 0;

    while (table.size() < 102960) {
        DLSForTable(mappedGoal, depthLimit, table);
        ++depthLimit;
    }
}

int heuristic2Distance(const Board& state,
                       const unordered_map<string, int>& table) {
    return table.at(colorBoardKey(colorBoard(state)));
}

int heuristicValue(const Board& state, const Board& goal, int choice, const unordered_map<string, int>& table) {
    if (choice == 1) {
        return manhattanDistance(state, goal);
    }

    if (choice == 2) {
        return heuristic2Distance(state,table);
    }

    int h1 = manhattanDistance(state,goal);
    int h2 = heuristic2Distance(state,table);

    return max(h1,h2);
}

SearchResult BFS(const Board& start, const Board& goal) {
    SearchResult result;

    // Start measuring CPU time.
    clock_t startTime = clock();

    // BFS uses an explicit FIFO queue.
    queue<Node> frontier;

    // Create and enqueue the root node.
    Node root = createRootNode(start);
    frontier.push(root);

    // The queue initially contains the root.
    result.maxFrontierSize = frontier.size();

    while (!frontier.empty())
    {
        // Remove the node at the front of the queue.
        Node current = frontier.front();
        frontier.pop();

        // A state counts as removed when it is taken
        // out of the queue.
        ++result.statesRemoved;

        // Check whether this node is the goal.
        if (current.state == goal) {
            result.solutionFound = true;
            result.numberOfMoves = current.depth;
            result.moveSequence = current.moves;

            result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
            return result;
        }

        // Generate all legal successors.
        vector<Node> successors = generateSuccessors(current);

        // BFS inserts successors in U, D, L, R order.
        for (const Node& child : successors) {
            frontier.push(child);
        }

        // Record the largest queue size seen so far.
        if (frontier.size() > result.maxFrontierSize) {
            result.maxFrontierSize = frontier.size();
        }
    }

    // Queue became empty, so no solution was found.
    result.solutionFound = false;
    result.numberOfMoves = 0;
    result.moveSequence = "";
    result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    return result;
}

SearchResult DLS(const Board& start, const Board& goal, int depthLimit) {
    SearchResult result;

    // Start measuring CPU time.
    clock_t startTime = clock();

    // DLS uses an explicit LIFO stack.
    stack<Node> frontier;

    // Create and push the root node.
    Node root = createRootNode(start);
    frontier.push(root);

    result.maxFrontierSize = frontier.size();

    while (!frontier.empty()) {
        // Remove the node at the top of the stack.
        Node current = frontier.top();
        frontier.pop();

        // Count every node removed from the stack.
        ++result.statesRemoved;

        // Check for the goal before checking the depth limit.
        // This is important because a goal at exactly depthLimit
        // should still be accepted.
        if (current.state == goal) {
            result.solutionFound = true;
            result.numberOfMoves = current.depth;
            result.moveSequence = current.moves;
            result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
            return result;
        }

        // If this node is already at the depth limit,
        // do not generate children.
        if (current.depth >= depthLimit) {
            continue;
        }

        vector<Node> successors = generateSuccessors(current);

        /*
            generateSuccessors() returns:

                U, D, L, R

            Because a stack is LIFO, we push them in reverse:

                R, L, D, U

            so that they are actually VISITED in:

                U, D, L, R
        */
        for (auto it = successors.rbegin(); it != successors.rend(); ++it) {
            frontier.push(*it);
        }

        if (frontier.size() > result.maxFrontierSize) {
            result.maxFrontierSize = frontier.size();
        }
    }

    // Stack became empty without finding the goal.
    result.solutionFound = false;
    result.numberOfMoves = 0;
    result.moveSequence = "";

    result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    return result;
}

SearchResult IDS(const Board& start, const Board& goal) {
    SearchResult result;
    clock_t startTime = clock();
    int depthLimit = 0;

    while (true) {
        // Perform a depth-limited search at the current limit.
        SearchResult dlsResult = DLS(start, goal, depthLimit);

        // IDS must count all states removed during every
        // DLS iteration.
        result.statesRemoved += dlsResult.statesRemoved;

        // Keep the largest stack size reached by any DLS run.
        if (dlsResult.maxFrontierSize > result.maxFrontierSize) {
            result.maxFrontierSize = dlsResult.maxFrontierSize;
        }

        // If DLS found the goal, IDS is finished.
        if (dlsResult.solutionFound) {
            result.solutionFound = true;
            result.numberOfMoves = dlsResult.numberOfMoves;
            result.moveSequence = dlsResult.moveSequence;
            result.cpuTime = static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
            return result;
        }

        // No solution at this depth.
        // Try the next depth limit.
        ++depthLimit;
    }
}