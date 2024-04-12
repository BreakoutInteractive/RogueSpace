//
//  LevelGrid.cpp
//  RS
//
//  Created by Zhiyuan Chen on 4/11/24.
//

#include "LevelGrid.hpp"

LevelGrid::LevelGrid(int width, int height, Vec2 origin){
    _origin = origin;
    _width = width;
    _height = height;
    _grid.resize(_width, std::vector<int>(_height, 0));
}

int LevelGrid::getNode(int tx, int ty){
    if (tx < 0 || tx >= _width || ty < 0 || ty >= _height){
        return 0;
    }
    return _grid[tx][ty];
}

int LevelGrid::getNode(Vec2 tileIndex){
    return getNode(tileIndex.x, tileIndex.y);
}

void LevelGrid::setNode(int tx, int ty, int val){
    if (tx < 0 || tx >= _width || ty < 0 || ty >= _height){
        return;
    }
    _grid[tx][ty] = val;
}

void LevelGrid::setNode(Vec2 tileIndex, int val){
    setNode(tileIndex.x, tileIndex.y, val);
}

Vec2 LevelGrid::worldToTile(Vec2 worldPos){
    worldPos -= _origin;
    int N = floor(worldPos.x / 2 + worldPos.y);
    Vec2 tilePos(0, 2*N);   // Ox, Oy
    int y0 = -N;
    int y1 = ceil(worldPos.x /  2 - worldPos.y);
    int diff = y1 - y0;
    tilePos.y -= diff;  // ty = Oy - diff
    if (diff >= 0){
        tilePos.x += diff/2;
    }
    else {
        tilePos.x += (diff - 1)/2;
    }
    return tilePos;
}

Vec2 LevelGrid::tileToWorld(int tx, int ty){
    Vec2 pos = _origin;
    // first compute the bottom center coordinate
    pos.y += ty / 2.0f;
    if (ty % 2 == 0){
        // even row
        pos.x += tx * 2;
    }
    else {
        pos.x += tx * 2 + 0.5f;
    }
    // adjust from bottom center to vertical center
    pos.y += 0.5;
    return pos;
}

Vec2 LevelGrid::tileToWorld(Vec2 tileIndex){
    return tileToWorld(tileIndex.x, tileIndex.y);
}
