//
//  Tileset.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/20/24.
//

#define IMAGE_KEY       "image"
#define IMG_WIDTH       "imagewidth"
#define IMG_HEIGHT      "imageheight"
#define TILE_WIDTH      "tilewidth"
#define TILE_HEIGHT     "tileheight"
#define TILE_SPACING    "spacing"


#include "Tileset.hpp"
#include "Helper.hpp"

Tileset::Tileset(const std::shared_ptr<JsonValue> json){
    if (json->has(IMAGE_KEY)){
        auto source = json->getString(IMAGE_KEY);
        _imageProperties.name = Helper::fileName(Helper::baseName(source));
        int tileCount = json->getInt("tilecount");
        int columns = json->getInt("columns");;
        _imageProperties.tileWidth = json->getInt(TILE_WIDTH);
        _imageProperties.tileHeight = json->getInt(TILE_HEIGHT);
        _imageProperties.rows = tileCount / columns;
        _imageProperties.columns = columns;
        _imageProperties.spacing = json->getInt(TILE_SPACING);
        _type = TilesetType::IMAGE;
        
        std::shared_ptr<JsonValue> tilesJson = json->get("tiles");
        if (tilesJson != nullptr){
            auto tiles = tilesJson->children();
            for (std::shared_ptr<JsonValue>& tile: tiles) {
                int id = tile->getInt("id");
                _tiles[id] = tile;
            }
        }
    }
    else {
        // TODO: support collection types
        CUAssertLog(false, "unsupported collection sets");
        _type = TilesetType::COLLECTION;
    }
}

bool Tileset::containsId(int id){
    if (_type == TilesetType::IMAGE){
        int maxId = _imageProperties.rows * _imageProperties.columns - 1;
        return 0 <= id && id <= maxId;
    }
    else {
        // TODO: support collection types
        CUAssertLog(false, "unsupported collection sets");
        return false;
    }
}

Tileset::TextureRegionData Tileset::getTextureData(int id){
    TextureRegionData atlas;
    
    if (_type == TilesetType::IMAGE){
        int row = id / _imageProperties.columns;
        int col = id % _imageProperties.columns;
        atlas.source = _imageProperties.name;
        atlas.startX = _imageProperties.spacing * (col + 1) + _imageProperties.tileWidth * col;
        atlas.startY = _imageProperties.spacing * (row + 1) + _imageProperties.tileHeight * row;
        atlas.lengthX = _imageProperties.tileWidth;
        atlas.lengthY = _imageProperties.tileHeight;
    }
    else {
        // TODO: support collection types
        CUAssertLog(false, "unsupported collection sets");
    }
    
    return atlas;
}

std::shared_ptr<JsonValue> Tileset::getTileData(int id){
    std::shared_ptr<JsonValue> data = nullptr;
    auto iterator = _tiles.find(id);
    if (iterator != _tiles.end()){
        data = iterator->second;
    }
    return data;
}
