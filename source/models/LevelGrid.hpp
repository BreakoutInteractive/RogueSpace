//
//  LevelGrid.hpp
//  RS
//
//  Created by Zhiyuan Chen on 4/11/24.
//

#ifndef LevelGrid_hpp
#define LevelGrid_hpp

#include <cugl/cugl.h>
#include <vector>

#include <iostream> // remove when no longer using prints

using namespace cugl;

class LevelGrid {
    
protected:
    std::vector<std::vector<int>> _grid;
    int _width;
    int _height;
    Vec2 _origin;
    
public:
    
    /**
     * creates a grid of size w x h whose bottom left `origin` is not necessarily (0,0).
     */
    LevelGrid(int width, int height, Vec2 origin);
    
#pragma mark -
#pragma mark Accessors
    
    /**
     * node/tile data retrieval.  `0` implies that the node is either non existent or could not be traversed on.
     *
     * @return the node data at the given tile index (tx, ty)
     */
    int getNode(int tx, int ty);
    
    /**
     * node/tile data retrieval.  `0` implies that the node is either non existent or could not be traversed on.
     *
     * @return the node data at the given tile index (tx, ty), treating the vector components as integers.
     */
    int getNode(Vec2 tileIndex);
    
    /**
     * sets value `val` at the given tile (tx,ty)
     */
    void setNode(int tx, int ty, int val);
    
    /**
     * sets value `val` at the given tile (tx,ty)
     */
    void setNode(Vec2 tileIndex, int val);
    
#pragma mark -
#pragma mark Utility
    /**
     * converts world position to staggered tile position (indices)
     */
    Vec2 worldToTile(Vec2 worldPos);
    
    /**
     * converts tile index (tx, ty) to the diamond tile's center coordinate.
     */
    Vec2 tileToWorld(Vec2 tileIndex);
    
    /**
     * converts tile index (tx, ty) to the diamond tile's center coordinate.
     */
    Vec2 tileToWorld(int tx, int ty);
    
    void printGrid() {
        for (int y = _height-1; y >= 0; y--) {
            if (y % 2 == 1){
                std::cout << " ";
            }
            for (int x = 0; x < _width; x++) {
                std::cout << _grid[x][y] << " ";
            }
            std::cout << std::endl;
        }
    }
};

#endif /* LevelGrid_hpp */
