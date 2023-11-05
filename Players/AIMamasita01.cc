#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Mamasita01


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }
  
  /**
   * Types and attributes for your player can be defined here.
   */
  typedef deque<Dir> PATH;
  typedef map<int, PATH> PATHS;

  PATHS myPaths;
  
  struct aStarCell {
    //position of previous cell
    Pos prev;
    // f = g + h
    int f, g, h;
    aStarCell() : prev(-1, -1) , f(-1) , g(-1) , h(-1){}
  };

  bool validCell(const Pos& pos) {
    return (0 <= pos.i and pos.i < rows()) and (0 <= pos.j and pos.j < cols());
  }
  //returns de direction that, added to source is dest. NONE if they are not adjecent
  Dir whichDirection(const Pos& source, const Pos& dest) {
    if (source + BOTTOM == dest) return BOTTOM;
    if (source + TOP == dest) return TOP;
    if (source + RIGHT == dest) return RIGHT;
    if (source + LEFT == dest) return LEFT;
    return NONE;
  }

  PATH makePath (const vector<vector<aStarCell>>& cellInfo, const Pos& final) {
    deque<Dir> path;
    Pos actualCell = final;
    int i = actualCell.i;  
    int j = actualCell.j; 
    do {
      Pos nextCell = cellInfo[i][j].prev;
      Dir dir = whichDirection(nextCell, actualCell);
      path.push_front(dir);
      actualCell = nextCell;
      i = actualCell.i;
      j = actualCell.j;
    } while (actualCell != cellInfo[i][j].prev);

    return path;
  }
  /*
  A* algorithm
  g(cost from initial postion to actual position) = g(pather) + 1;
  h(estimated cost from actual position to final position) = abs(final.x - actual.x) + abs(final.y - actual.y)
  */
  PATH aStarAlgorithm(const Pos& initial, const Pos& final) {
    PATH path;
    if (initial == final) return path;
    //create closed list, inicialize all to false;
    bool closedList[rows()][cols()];
    for(int i = 0; i < rows(); ++i) for(int j = 0; j < cols(); ++j) closedList[i][j] = false;
    //2d vector with all info of all cells
    vector<vector<aStarCell>> cellInfo(rows(),vector<aStarCell>(cols()));
    //inicialize the initial cell, prev = initial and f=g=h=0;
    int i = initial.i;
    int j = initial.j;
    cellInfo[i][j].f = 0;
    cellInfo[i][j].g = 0;
    cellInfo[i][j].h = 0;
    cellInfo[i][j].prev = initial;
    //create openList: first int is f, the Pos is position of cell
    priority_queue<pair<int,Pos>,vector<pair<int,Pos>>,greater<pair<int,Pos>>> openList;
    set<Pos> openListAux;
    //inicialize openList
    openList.emplace(0, initial);
    openListAux.insert(initial);
    //starts the main loop
    while(!openList.empty()) {
      //we take the element at the top os the queue (lowest f) and we pop from queue
      Pos actual = openList.top().second;
      if (actual == final) {
        cellInfo[actual.i][actual.j].prev.i = i;
        cellInfo[actual.i][actual.j].prev.j = j;
        return makePath(cellInfo, final);
      }
      openList.pop();
      openListAux.erase(actual);
      // we add to the closedList
      i = actual.i;
      j = actual.j;
      closedList[i][j] = true;
      //now, for each direction(BOTTOM, RIGHT, TOP, LEFT) we procces
      for(int dirInt = BOTTOM; dirInt != NONE; ++dirInt) {
        Dir dir = static_cast<Dir>(dirInt); 
        Pos adjecent = actual + dir;
        int iAux = adjecent.i;
        int jAux = adjecent.j;
        if(validCell(adjecent) and positionType(adjecent) != WALL and !closedList[iAux][jAux]) {
          if (openListAux.find(adjecent) == openListAux.end()) {
            cellInfo[iAux][jAux].g = cellInfo[i][j].g + 1;
            cellInfo[iAux][jAux].h = abs(final.i - iAux) + abs(final.j - jAux);
            cellInfo[iAux][jAux].f = cellInfo[iAux][jAux].g + cellInfo[iAux][jAux].h;
            cellInfo[iAux][jAux].prev.i = i;
            cellInfo[iAux][jAux].prev.j = j;
            openList.emplace(cellInfo[iAux][jAux].f,adjecent);
            openListAux.insert(adjecent);
          } else if(cellInfo[i][j].g + 1 < cellInfo[iAux][jAux].g) {
              cellInfo[iAux][jAux].g = cellInfo[i][j].g + 1;
              cellInfo[iAux][jAux].f = cellInfo[iAux][jAux].g + cellInfo[iAux][jAux].h;
              cellInfo[iAux][jAux].prev.i = i;
              cellInfo[iAux][jAux].prev.j = j;
          }
        }
      }
    }
    return path;
  }
  
  CellType positionType(const Pos& position) {
    return cell(position).type;
  }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    if (status(me()) >= 0.9) return;
    Pos middle;
    middle.i = rows() / 2;
    middle.j = cols() / 2;
    vector<int> unitsId = my_units(me());
    int numUnits = unitsId.size();
    
    for(int i = 0; i < numUnits; ++i) {
      if (round() == 0) myPaths.insert(make_pair(unitsId[i], aStarAlgorithm(unit(unitsId[i]).pos, middle)));
      if (!myPaths[unitsId[i]].empty()) {
        Dir dir = myPaths[unitsId[i]].front();
        move(unitsId[i], dir);
        myPaths[unitsId[i]].pop_front();
      }
      Dir dir = BOTTOM;
    }
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
