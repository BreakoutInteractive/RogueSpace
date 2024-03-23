//
//  Tileset.hpp
//  RS
//
//  Created by Zhiyuan Chen on 3/20/24.
//

#ifndef Tileset_hpp
#define Tileset_hpp

#include <cugl/cugl.h>

using namespace cugl;

class Tileset {
    
private:
    
    enum class TilesetType : int {
        IMAGE = 0,
        COLLECTION = 1
    };
    
    struct ImageProps {
        int spacing = 0;
        int tileWidth = 1;
        int tileHeight = 1;
        int rows = 1;
        int columns = 1;
        std::string name = "";
    };
    
    /** Image tileset properties */
    ImageProps _imageProperties;
    
    TilesetType type;
    
    /** the associated list of tiles in this tileset (may be empty) */
    std::shared_ptr<JsonValue> tiles;
    
public:
    
    /**
     * structured atlas for a subtexture
     */
    struct TextureRegionData {
        /** texture name */
        std::string source;
        /** texture region starting x*/
        int startX;
        /** texture region starting y*/
        int startY;
        /** texture region length (in pixels) */
        int lengthX;
        /** texture region length (in pixels) */
        int lengthY;
    };

    /**
     * Constructs a tileset from the given Tiled json data
     */
    Tileset(std::shared_ptr<JsonValue> json);
    
    /**
     * returned value can be anything in the case that `id` is not contained in this tileset
     *
     * @return texture subregion data for the given valid `id`
     */
    TextureRegionData getTextureData(int id);
    
    /**
     * @return whether the given tile id exists in this tileset
     */
    bool containsId(int id);
};

#endif /* Tileset_hpp */
