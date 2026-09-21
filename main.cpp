#include <cctype>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <utility>
#include <queue>
#include <ctime>
#include <stack>

using namespace std;

using Board = vector<vector<int>>;

struct Node
{
    Board state;

    // Depth of this node in the search tree.
    // The start/root node is at depth 0.
    int depth;

    // Sequence of moves used to reach this node.
    // Example: "ULDR"
    string moves;

    // Current location of the blank tile (0).
    int blankRow;
    int blankCol;

    // Location of the blank tile in the parent state.
    //
    // This allows us to prevent immediately returning to the
    // parent when generating successors.
    //
    // For the root node, both values are -1.
    int parentBlankRow;
    int parentBlankCol;

    // Default constructor.
    Node()
        : depth(0),
          moves(""),
          blankRow(-1),
          blankCol(-1),
          parentBlankRow(-1),
          parentBlankCol(-1)
    {
    }

    // Constructor for creating a node from a board.
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
    {
    }
};

// Structure returned by each search algorithm.
struct SearchResult
{
    bool solutionFound = false;
    int numberOfMoves = 0;
    string moveSequence = "";
    long long statesRemoved = 0;
    size_t maxFrontierSize = 0;
    double cpuTime = 0.0;
};

// ------------------------------------------------------------
// Function prototypes
// ------------------------------------------------------------

// Input / output helper functions
Board readBoard(const string& prompt);
void printBoard(const Board& board);
void printResult(const string& algorithmName, const SearchResult& result);

// Node helper functions.
Node createRootNode(const Board& board);
bool findBlank(const Board& board, int& row, int& col);
vector<Node> generateSuccessors(const Node& node);

// Search algorithm stubs
SearchResult BFS(const Board& start, const Board& goal);
SearchResult DLS(const Board& start, const Board& goal, int depthLimit);
SearchResult IDS(const Board& start, const Board& goal);

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

    // Algorithm menu.
    cout << "\n=============================================\n";
    cout << "Choose an algorithm:\n";
    cout << "1. BFS\n";
    cout << "2. IDS\n";
    cout << "3. BFS and IDS\n";
    cout << "4. DLS\n";
    cout << "=============================================\n";
    cout << "Selection: ";

    int choice;
    cin >> choice;

    cout << '\n';

    switch (choice)
    {
        case 1:
        {
            SearchResult result = BFS(start, goal);
            printResult("BFS", result);
            break;
        }

        case 2:
        {
            SearchResult result = IDS(start, goal);
            printResult("IDS", result);
            break;
        }

        case 3:
        {
            SearchResult bfsResult = BFS(start, goal);
            printResult("BFS", bfsResult);

            cout << '\n';

            SearchResult idsResult = IDS(start, goal);
            printResult("IDS", idsResult);
            break;
        }

        case 4:
        {
            int depthLimit;

            cout << "Enter the depth limit for DLS: ";
            cin >> depthLimit;

            while (depthLimit < 0)
            {
                cout << "Depth limit must be non-negative. Enter again: ";
                cin >> depthLimit;
            }

            SearchResult result = DLS(start, goal, depthLimit);
            printResult("DLS", result);
            break;
        }

        default:
            cout << "Invalid algorithm selection.\n";
            return 1;
    }

    return 0;
}

// ------------------------------------------------------------
// Input / output helper functions
// ------------------------------------------------------------

Board readBoard(const string& prompt)
{
    cout << prompt;

    Board board(4, vector<int>(4));

    /*
        Input is expected in this form:

        [[1,2,3,4],[5,6,7,8],[9,10,11,12],[13,14,15,0]]

        We ignore punctuation characters and extract only the
        sixteen integer values.
    */

    char ch;

    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            // Ignore everything until the next digit or minus sign.
            while (cin.peek() != EOF &&
                   !isdigit(cin.peek()) &&
                   cin.peek() != '-')
            {
                cin.get(ch);
            }

            cin >> board[row][col];
        }
    }

    // Remove any punctuation left at the end of the input line.
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    return board;
}

void printBoard(const Board& board)
{
    for (const auto& row : board)
    {
        cout << "[ ";

        for (int value : row)
        {
            cout << value << '\t';
        }

        cout << "]\n";
    }
}

void printResult(const string& algorithmName, const SearchResult& result)
{
    cout << "=============================================\n";
    cout << algorithmName << " Results\n";
    cout << "=============================================\n";

    if (result.solutionFound)
    {
        cout << "The solution found.\n";
        cout << "The number of moves: "
             << result.numberOfMoves << '\n';

        cout << "The sequence of moves: "
             << result.moveSequence << '\n';
    }
    else
    {
        cout << "The solution was not found.\n";
        cout << "The number of moves: 0\n";
        cout << "The sequence of moves: N/A\n";
    }

    cout << "The number of states removed: "
         << result.statesRemoved << '\n';

    cout << "The maximum size of the queue (stack): "
         << result.maxFrontierSize << '\n';

    cout << "CPU time: "
         << result.cpuTime << " seconds\n";

    cout << "=============================================\n";
}

// ------------------------------------------------------------
// Node helper functions
// ------------------------------------------------------------

bool findBlank(const Board& board, int& row, int& col)
{
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            if (board[r][c] == 0)
            {
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

Node createRootNode(const Board& board)
{
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

vector<Node> generateSuccessors(const Node& node)
{
    vector<Node> successors;

    // Move definitions:
    // U = blank moves up
    // D = blank moves down
    // L = blank moves left
    // R = blank moves right
    const int rowChange[4] = {-1, 1, 0, 0};
    const int colChange[4] = {0, 0, -1, 1};
    const char moveChar[4] = {'U', 'D', 'L', 'R'};

    for (int i = 0; i < 4; ++i)
    {
        int newRow = node.blankRow + rowChange[i];
        int newCol = node.blankCol + colChange[i];

        // Skip moves that go outside the 4x4 board.
        if (newRow < 0 || newRow >= 4 ||
            newCol < 0 || newCol >= 4)
        {
            continue;
        }

        // Do not immediately return to the parent state.
        //
        // If the blank moves into the location it occupied
        // in the parent node, that successor would simply
        // recreate the parent.
        if (newRow == node.parentBlankRow &&
            newCol == node.parentBlankCol)
        {
            continue;
        }

        // Copy the current board.
        Board newState = node.state;

        // Slide the adjacent tile into the blank position.
        swap(
            newState[node.blankRow][node.blankCol],
            newState[newRow][newCol]
        );

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
// Search algorithm stubs
// ------------------------------------------------------------

SearchResult BFS(const Board& start, const Board& goal)
{
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
        if (current.state == goal)
        {
            result.solutionFound = true;
            result.numberOfMoves = current.depth;
            result.moveSequence = current.moves;

            result.cpuTime =
                static_cast<double>(clock() - startTime)
                / CLOCKS_PER_SEC;

            return result;
        }

        // Generate all legal successors.
        vector<Node> successors =
            generateSuccessors(current);

        // BFS inserts successors in U, D, L, R order.
        for (const Node& child : successors)
        {
            frontier.push(child);
        }

        // Record the largest queue size seen so far.
        if (frontier.size() > result.maxFrontierSize)
        {
            result.maxFrontierSize = frontier.size();
        }
    }

    // Queue became empty, so no solution was found.
    result.solutionFound = false;
    result.numberOfMoves = 0;
    result.moveSequence = "";

    result.cpuTime =
        static_cast<double>(clock() - startTime)
        / CLOCKS_PER_SEC;

    return result;
}

SearchResult DLS(const Board& start,
                 const Board& goal,
                 int depthLimit)
{
    SearchResult result;

    // Start measuring CPU time.
    clock_t startTime = clock();

    // DLS uses an explicit LIFO stack.
    stack<Node> frontier;

    // Create and push the root node.
    Node root = createRootNode(start);
    frontier.push(root);

    result.maxFrontierSize = frontier.size();

    while (!frontier.empty())
    {
        // Remove the node at the top of the stack.
        Node current = frontier.top();
        frontier.pop();

        // Count every node removed from the stack.
        ++result.statesRemoved;

        // Check for the goal before checking the depth limit.
        // This is important because a goal at exactly depthLimit
        // should still be accepted.
        if (current.state == goal)
        {
            result.solutionFound = true;
            result.numberOfMoves = current.depth;
            result.moveSequence = current.moves;

            result.cpuTime =
                static_cast<double>(clock() - startTime)
                / CLOCKS_PER_SEC;

            return result;
        }

        // If this node is already at the depth limit,
        // do not generate children.
        if (current.depth >= depthLimit)
        {
            continue;
        }

        vector<Node> successors =
            generateSuccessors(current);

        /*
            generateSuccessors() returns:

                U, D, L, R

            Because a stack is LIFO, we push them in reverse:

                R, L, D, U

            so that they are actually VISITED in:

                U, D, L, R
        */
        for (auto it = successors.rbegin();
             it != successors.rend();
             ++it)
        {
            frontier.push(*it);
        }

        if (frontier.size() > result.maxFrontierSize)
        {
            result.maxFrontierSize = frontier.size();
        }
    }

    // Stack became empty without finding the goal.
    result.solutionFound = false;
    result.numberOfMoves = 0;
    result.moveSequence = "";

    result.cpuTime =
        static_cast<double>(clock() - startTime)
        / CLOCKS_PER_SEC;

    return result;
}

SearchResult IDS(const Board& start, const Board& goal)
{
    SearchResult result;

    // Measure the total CPU time for the entire IDS search,
    // including all repeated DLS calls.
    clock_t startTime = clock();

    int depthLimit = 0;

    while (true)
    {
        // Perform a depth-limited search at the current limit.
        SearchResult dlsResult =
            DLS(start, goal, depthLimit);

        // IDS must count all states removed during every
        // DLS iteration.
        result.statesRemoved += dlsResult.statesRemoved;

        // Keep the largest stack size reached by any DLS run.
        if (dlsResult.maxFrontierSize >
            result.maxFrontierSize)
        {
            result.maxFrontierSize =
                dlsResult.maxFrontierSize;
        }

        // If DLS found the goal, IDS is finished.
        if (dlsResult.solutionFound)
        {
            result.solutionFound = true;
            result.numberOfMoves =
                dlsResult.numberOfMoves;
            result.moveSequence =
                dlsResult.moveSequence;

            result.cpuTime =
                static_cast<double>(
                    clock() - startTime
                ) / CLOCKS_PER_SEC;

            return result;
        }

        // No solution at this depth.
        // Try the next depth limit.
        ++depthLimit;
    }
}