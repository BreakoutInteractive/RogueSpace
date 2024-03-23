//
//  Tileset.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/20/24.
//

#define NAME            "name"
#define IMG_WIDTH       "imagewidth"
#define IMG_HEIGHT      "imageheight"
#define TILE_WIDTH       "tilewidth"
#define TILE_HEIGHT      "tileheight"
#define TILE_SPACING    "spacing"

#include "Tileset.hpp"

Tileset::Tileset(std::shared_ptr<JsonValue> json){
    if (json->has("image")){
        auto name = json->getString(NAME);
        CUAssertLog(name != "", "texture name must be given for proper rendering");
        _imageProperties.name = name;
        int iwidth = json->getInt(IMG_WIDTH);
        int iheight = json->getInt(IMG_HEIGHT);
        int tileCount = json->getInt("tilecount");
        int columns = json->getInt("columns");;
        _imageProperties.tileWidth = json->getInt(TILE_WIDTH);
        _imageProperties.tileHeight = json->getInt(TILE_HEIGHT);
        _imageProperties.rows = tileCount / columns;
        _imageProperties.columns = columns;
        _imageProperties.spacing = json->getInt(TILE_SPACING);
        type = TilesetType::IMAGE;
    }
    else {
        // TODO: support collection types
        CUAssertLog(false, "unsupported collection sets");
        type = TilesetType::COLLECTION;
    }
}

bool Tileset::containsId(int id){
    if (type == TilesetType::IMAGE){
        int maxId = _imageProperties.rows * _imageProperties.columns - 1;
        return 0 <= id && id <= maxId;
    }
    else {
        // TODO: support collection types
        CUAssertLog(false, "unsupported collection sets");
        return false;
    }
}

Tileset::TextureRegionData Tileset:: getTextureData(int id){
    TextureRegionData atlas;
    
    if (type == TilesetType::IMAGE){
        int row = id / _imageProperties.columns;
        int col = id % _imageProperties.columns;
        atlas.source = _imageProperties.name;
        atlas.startX = _imageProperties.spacing * (col + 1) + _imageProperties.tileWidth * col;
        atlas.startY = _imageProperties.spacing * (row + 1) + _imageProperties.tileHeight * row;
        atlas.lengthX = _imageProperties.tileWidth;
        atlas.lengthY = _imageProperties.tileHeight;
    }
    
    return atlas;
}
