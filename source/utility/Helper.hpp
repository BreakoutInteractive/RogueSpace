//
//  Helper.hpp
//  RS
//
//  Created by Zhiyuan Chen on 3/23/24.
//

#ifndef Helper_hpp
#define Helper_hpp

#include<string>

class Helper {
    
public:
    /**
     * @return name of file in a given filepath
     */
    static std::string baseName(std::string const & path)
    {
        return path.substr(path.find_last_of("/\\") + 1);
    }
    
    /**
     * @return the file name without the extension
     */
    static std::string fileName(std::string const & fileName)
    {
        return fileName.substr(0, fileName.find_last_of("."));
    }
};

#endif /* Helper_hpp */
