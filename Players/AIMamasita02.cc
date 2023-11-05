#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Mamasita02


struct PLAYER_NAME : public Player {


  static Player* factory () {
    return new PLAYER_NAME;
  }
  

  //Types and attributes for your player can be defined here.
  typedef stack<Dir> PATH;
  typedef map<int, PATH> PATHS; //int -> unitId, PATH-> current path of the unit;
  typedef map<Pos,pair<int,int>> PLACES; //int1-> idCity. int2->idPath
  typedef tuple<int,int,Pos> PQTYPE; //type for priority queue. int1->key. int2->instance
  typedef tuple<Pos,int,int,int> PLACE;

  PATHS myPaths; //contais all the paths for every unit
  vector<PLACE> importantPlaces;
  vector<int> myUnitsId;
  map<int,Pos> prevUnitPos; //contais the previous position of the unit
  
  struct aStarCell {
    //position of previous cell
    Pos prev;
    // f = g + h
    int f, g, h;
    aStarCell() : prev(-1, -1) , f(-1) , g(-1) , h(-1){}
  };

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

  Pos nearestPlace(const Pos& position) {
    Pos place = position;
    int dist = rows()*cols();
    int nUnits = 1000;
    int nplaces = importantPlaces.size();
    int finali = 0;
    for (int i = 0; i < nplaces; ++i) {
      Pos auxPlace = get<0>(importantPlaces[i]);
      int auxNUnits = get<3>(importantPlaces[i]);
      int auxDist = manhattanDist(position, auxPlace);
      if ((auxNUnits < nUnits) or (auxNUnits == nUnits and auxDist < dist)) {
        place = auxPlace;
        dist = auxDist;
        nUnits = auxNUnits;
        finali = i;
      }
    }
    ++get<3>(importantPlaces[finali]);
    return place;
  }

  void initializeAll() {
    //initialize important places
    int npaths = nb_paths();
    importantPlaces.resize(npaths * 2);
    for (int i = 0; i < npaths; ++i) {
      Path actualPath = path(i);
      importantPlaces[2*i] = make_tuple(actualPath.second[0],actualPath.first.first,i,0);
      int n = actualPath.second.size() - 1;
      importantPlaces[2*i + 1] = make_tuple(actualPath.second[n], actualPath.first.second,i,0);
    }
    //initialize myUnits
    myUnitsId = my_units(me());
    //inicialize prev positions
    updatePrevPos();
    //inizialize all paths
    int numUnits = myUnitsId.size();
    for(int i = 0; i < numUnits; ++i) {
      Unit actualUnit = unit(myUnitsId[i]);
      Pos city = nearestPlace(actualUnit.pos);
      myPaths[actualUnit.id] = aStarAlgorithm(actualUnit.pos, city);
    }
  }
  
  void updateUnits() {
    myUnitsId = my_units(me());
    PATHS::iterator it1 = myPaths.begin();
    int it2 = 0;
    int numUnits = myUnitsId.size();
    while (it1 != myPaths.end() and  it2 < numUnits) {
      if (it1 -> first < myUnitsId[it2]) {
        it1 = myPaths.erase(it1);
      } else if(it1 -> first > myUnitsId[it2]) {
        PATH path;
        myPaths.insert(make_pair(myUnitsId[it2],path));
        ++it2;
      } else {
        ++it1;
        ++it2;
      }
    }
    if (it1 != myPaths.end()) {
       myPaths.erase(it1, myPaths.end());
    } else {
      for (int i = it2; i < numUnits; ++i) {
        PATH path;
        myPaths.insert(make_pair(myUnitsId[i],path));
      }
     }
  }

  void updatePaths() {
    int numUnits = myUnitsId.size();
    for(int i = 0; i < numUnits; ++i) {
      Unit actualUnit = unit(myUnitsId[i]);
      int dist = manhattanDist(prevUnitPos[actualUnit.id],actualUnit.pos);
      if (dist == 1) myPaths[actualUnit.id].pop();
      else if (dist > 1) {
        while(!myPaths[actualUnit.id].empty()) myPaths[actualUnit.id].pop();
      }

      if (myPaths[myUnitsId[i]].empty() and positionType(actualUnit.pos) == GRASS) {
        Pos city = nearestPlace(actualUnit.pos);
        myPaths[actualUnit.id] = aStarAlgorithm(actualUnit.pos, city);
      } else checkNerly(actualUnit.id);
    }
  }

  void updatePrevPos() {
     int numUnits = myUnitsId.size();
    for (int i = 0; i < numUnits; ++i) {
      Unit actualUnit = unit(myUnitsId[i]);
      prevUnitPos[actualUnit.id] = actualUnit.pos;
    }
  }

  void updateAll() {
    updateUnits();
    updatePaths();
    updatePrevPos();
  }

  void checkNerly(const int& unitId) {
    Unit actualUnit = unit(unitId);
    for(int dirInt = BOTTOM; dirInt != NONE; ++dirInt) {
      Dir dir = static_cast<Dir>(dirInt);
      Cell adjecent = cell(actualUnit.pos + dir);
      bool itsMyUnit = binary_search(myUnitsId.begin(),myUnitsId.end(), adjecent.unit_id);
      if(adjecent.unit_id != -1 and (!itsMyUnit or adjecent.mask)) {
        myPaths[unitId].push(static_cast<Dir>((dir + 2) % 4));
        myPaths[unitId].push(dir);
        return;
      }
    }
    return;
  }

  void moveUnit(const int& unitId) {
    if(!myPaths[unitId].empty()) {
      Dir dir = myPaths[unitId].top();
      move(unitId, dir);
    }
  }
  
  
  virtual void play () {
    if (status(me()) >= 0.9) return;

    if (round() == 0) initializeAll();
    else updateAll();

    int numUnits = myUnitsId.size();
    for(int i = 0; i < numUnits; ++i) {
      moveUnit(myUnitsId[i]);
    }

    updatePrevPos();
  }
};

RegisterPlayer(PLAYER_NAME);
