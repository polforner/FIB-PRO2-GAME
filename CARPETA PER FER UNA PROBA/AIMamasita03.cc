#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Mamasita03


struct PLAYER_NAME : public Player {


  static Player* factory () {
    return new PLAYER_NAME;
  }
  

//TYPEDEFS AND STRUCTS
  typedef stack<Dir> PATH;
  typedef tuple<int,int,Pos> PQTYPE; //type for priority queue. int1->key. int2->instance

  struct unitInfo {
    Pos prevPosition;
    bool isAttacking; //true if the unit is in a battle process
    Pos objective;
    int objectiveId;
    PATH path;

    unitInfo() : prevPosition(-1, -1) , isAttacking(false) , objective(-1, -1), objectiveId(-1), path(){}
  };

  struct aStarCell {
    //position of previous cell
    Pos prev;
    // f = g + h
    int f, g, h;
    aStarCell() : prev(-1, -1) , f(-1) , g(-1) , h(-1){}
  };

  struct place {
    Pos position;
    bool isAvailable;
    place(): position(-1,-1), isAvailable(true){}
  };

//DATA
  map<int,unitInfo> myUnits;
  vector<place> myCities;

  int numUnits;

//AUXILIAR FUNCTIONS
  Dir whichDirection(const Pos& source, const Pos& dest) {
  //Returns de direction that, added to 'source' is 'dest'. NONE if 'source' and 'dest' are not adjecent
    if (source + BOTTOM == dest) return BOTTOM;
    if (source + TOP == dest) return TOP;
    if (source + RIGHT == dest) return RIGHT;
    if (source + LEFT == dest) return LEFT;
    return NONE;
  }

  CellType positionType(const Pos& position) {
    return cell(position).type;
  }

  int manhattanDist(const Pos& initial, const Pos& final) {
    return abs(final.i - initial.i) + abs(final.j - initial.j);
  }

  bool closestUnit(const int& unitId,const int& cityId) {
    bool isClosest = true;
    int dist = manhattanDist(unit(unitId).pos, myCities[cityId].position);
    map<int,unitInfo>::const_iterator it = myUnits.begin();
    while (isClosest and it != myUnits.end()) {
      int auxDist = manhattanDist(unit(it -> first).pos, myCities[cityId].position);
      if (it -> second.objectiveId == -1  and auxDist < dist) isClosest = false;
      else ++it;
    }
    return isClosest;
  }

  Pos findNearestPlace(const int& unitId) {
    int minDist = 1000;
    int finalPlace = unitId;
    bool first = true;
    int size = myCities.size();
    for (int i = 0; i < size; ++i) {
      if(myCities[i].isAvailable and closestUnit(unitId,i)) {
        int dist = manhattanDist(unit(unitId).pos, myCities[i].position);
        if (first) {
          finalPlace = i;
          minDist = dist;
          first = false;
        } else if(dist < minDist) {
           finalPlace = i;
          minDist = dist;
        }
      }
    }
    if (!first) {
      myUnits[unitId].objectiveId = finalPlace;
      myCities[finalPlace].isAvailable = false;
      return myCities[finalPlace].position;
    }
    return unit(unitId).pos;
  }

//A star algorithm
  PATH makePath(const vector<vector<aStarCell>>& cellInfo, const Pos& final) {
    PATH path;
    Pos actualCell = final;
    int i = actualCell.i;  
    int j = actualCell.j; 
    do {
      Pos nextCell = cellInfo[i][j].prev;
      Dir dir = whichDirection(nextCell, actualCell);
      path.push(dir);
      actualCell = nextCell;
      i = actualCell.i;
      j = actualCell.j;
    } while (actualCell != cellInfo[i][j].prev);
    return path;
  }

  PATH aStarAlgorithm(const Pos& initial, const Pos& final) {
  //A* algorithm
  //Return a PATH to go from 'initial' to 'final'
    //If initial and final are equal, return empty path(no move)
    PATH path;
    if (initial == final) return path;
    //Create closed list(finished cells), inicialize all to false;
    bool closedList[rows()][cols()];
    for(int i = 0; i < rows(); ++i) for(int j = 0; j < cols(); ++j) closedList[i][j] = false;
    //2d vector with all info of all cells()
    vector<vector<aStarCell>> cellInfo(rows(),vector<aStarCell>(cols()));
    //Inicialize the initial cell, prev = initial and f=g=h=0
    int i = initial.i;
    int j = initial.j;
    cellInfo[i][j].f = 0;
    cellInfo[i][j].g = 0;
    cellInfo[i][j].h = 0;
    cellInfo[i][j].prev = initial;
    //Create openList(potencial cells): int1->f, int2->instance
    //O(1) to get the next position
    priority_queue<PQTYPE,vector<PQTYPE>,greater<PQTYPE>> openList;
    int instance = 0;
    //Auxiliar openList for fast search (O(logn) to search one position)
    set<Pos> openListAux;
    //inicialize openList
    openList.emplace(make_tuple(0,instance, initial));
    openListAux.insert(initial);
    //starts the main loop
    while(!openList.empty()) {
      //we take the element at the top os the queue (lowest f) and we pop from queue
      Pos actual = get<2>(openList.top());
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
        if(pos_ok(adjecent) and positionType(adjecent) != WALL and !closedList[iAux][jAux]) {
          if (openListAux.find(adjecent) == openListAux.end()) {
            cellInfo[iAux][jAux].g = cellInfo[i][j].g + 1;
            cellInfo[iAux][jAux].h = abs(final.i - iAux) + abs(final.j - jAux);
            cellInfo[iAux][jAux].f = cellInfo[iAux][jAux].g + cellInfo[iAux][jAux].h;
            cellInfo[iAux][jAux].prev.i = i;
            cellInfo[iAux][jAux].prev.j = j;
            ++instance;
            openList.emplace(make_tuple(cellInfo[iAux][jAux].f,instance,adjecent));
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

//to find the adjecent city from path
  Pos findCity(const Pos& position) {
    if (positionType(position + BOTTOM) == CITY) return (position + BOTTOM);
    if (positionType(position + TOP) == CITY) return (position + TOP);
    if (positionType(position + RIGHT) == CITY) return (position + RIGHT);
    if (positionType(position + LEFT) == CITY) return (position + LEFT);
    return position;
  }

//moveUnit
  void moveUnit(const int& unitId) {
    if(!myUnits[unitId].path.empty()) {
      Dir dir = myUnits[unitId].path.top();
      move(unitId, dir);
    }
  }

//UPDATE
  void updateUnits() {
    vector<int> vectorUnits = my_units(me());
    numUnits = vectorUnits.size();
    map<int,unitInfo>::iterator it1 = myUnits.begin();
    int it2 = 0;
    while (it1 != myUnits.end() and  it2 < numUnits) {
      if (it1 -> first < vectorUnits[it2]) {
        int objectiveId = it1 -> second.objectiveId;
        if (objectiveId != -1) myCities[objectiveId].isAvailable = true;
        it1 = myUnits.erase(it1);
      } else if(it1 -> first > vectorUnits[it2]) {
        unitInfo actualUnit;
        actualUnit.prevPosition = unit(vectorUnits[it2]).pos;
        myUnits.insert(make_pair(vectorUnits[it2],actualUnit));
        ++it2;
      } else {
        ++it1;
        ++it2;
      }
    }
    if (it1 != myUnits.end()) {
        int objectiveId = it1 -> second.objectiveId;
        if (objectiveId != -1) myCities[objectiveId].isAvailable = true;
       myUnits.erase(it1, myUnits.end());
    } else {
      for (int i = it2; i < numUnits; ++i) {
        unitInfo actualUnit;
        actualUnit.prevPosition = unit(vectorUnits[i]).pos;
        myUnits.insert(make_pair(vectorUnits[i],actualUnit));
      }
     }
  }
  
  void updatePaths() {
    map<int,unitInfo>::iterator it;
    for(it = myUnits.begin(); it != myUnits.end(); ++it) {
      //control of objects in the path
      Pos position = unit(it -> first).pos;
      int dist = manhattanDist(it -> second.prevPosition,position);

      if (dist == 1) {
         it -> second.path.pop();
         it -> second.isAttacking = false;
      }
      else if (dist > 1) {
        while(!(it -> second.path.empty())) it -> second.path.pop();
        it -> second.isAttacking = false;
      } else if (it -> second.isAttacking) {
        it -> second.isAttacking = false;
        it -> second.path.pop();
        it -> second.path.pop();
      }
      //set path for units without path
      if (it -> second.path.empty() and unit(it -> first).damage == 0) {
        if (it -> second.objective == unit(it -> first).pos) {
          it -> second.objective = {-1,-1};
          Pos adjecentCity = findCity(unit(it -> first).pos);
          it -> second.path = aStarAlgorithm(unit(it -> first).pos, adjecentCity);
        } else {
          int prevObjectiveId = it -> second.objectiveId;
          Pos objective = findNearestPlace(it -> first);
          it -> second.path = aStarAlgorithm(unit(it -> first).pos, objective);
          it -> second.objective = objective;
          if (prevObjectiveId != -1) myCities[prevObjectiveId].isAvailable = true;
        }
      }
      checkNearly(it -> first);
    }
  }

  void updatePrevPos() {
    map<int,unitInfo>::iterator it;
    for(it = myUnits.begin(); it != myUnits.end(); ++it) {
      it -> second.prevPosition = unit(it -> first).pos;;
    }
  }

  void updateAll() {
    updateUnits();
    numUnits = myUnits.size();
    updatePaths();
    updatePrevPos();
  }

//ATACKING
  void checkNearly(const int& unitId) {
    for(int dirInt = BOTTOM; dirInt != NONE; ++dirInt) {
      Dir dir = static_cast<Dir>(dirInt);
      Cell adjecent = cell(unit(unitId).pos + dir);
      bool isEnemy = (adjecent.unit_id != -1) and (myUnits.find(adjecent.unit_id) == myUnits.end());
      bool inMyPath = !(myUnits[unitId].path.empty()) and (myUnits[unitId].path.top() == dir);
      if(isEnemy and !inMyPath) {
        myUnits[unitId].isAttacking = true;
        myUnits[unitId].path.push(static_cast<Dir>((dir + 2) % 4));
        myUnits[unitId].path.push(dir);
        return;
      }
    }
    return;
  }

//INITIALIZE
  void initializeCities() {
    int npaths = nb_paths();
    vector<place> aux(npaths*2);
    myCities = aux;
    for (int i = 0; i < npaths; ++i) {
      Path actualPath = path(i);
      myCities[2 * i].position = actualPath.second[0];
      myCities[2 * i + 1].position = actualPath.second[actualPath.second.size() - 1];
    }
    cerr << myCities[0].position.i << ',' << myCities[0].position.j << endl;
  }

  void initializeAll() {
    initializeCities();
    //initialize myUnits
    vector<int> vectorUnits = my_units(me());
    numUnits = vectorUnits.size();
    for (int i = 0; i < numUnits; ++i) {
      unitInfo actualUnit;
      actualUnit.prevPosition = unit(vectorUnits[i]).pos;
      myUnits.insert(make_pair(vectorUnits[i],actualUnit));
    }
    //inizialize all paths
    map<int,unitInfo>::iterator it;
    for (it = myUnits.begin(); it != myUnits.end(); ++it) {
      Pos objective = findNearestPlace(it -> first);
      it -> second.path = aStarAlgorithm(unit(it -> first).pos, objective);
      it -> second.objective = objective;
    }
    
  }
  
  virtual void play () { 
    if (round() == 0) initializeAll();
    else updateAll();

    map<int,unitInfo>::iterator it;
    for(it = myUnits.begin(); it != myUnits.end(); ++it) {
      moveUnit(it -> first);
    }
  }
};

RegisterPlayer(PLAYER_NAME);
