//
//  JoyStick.hpp
//  RS
//
//  Created by Dasha Griffiths on 3/4/24.
//

#ifndef JoyStick_hpp
#define JoyStick_hpp
#include <cugl/cugl.h>

class JoyStick {
#pragma mark JoyStick
public:
    /**
     * An inner class that represents the movable top of the joystick
     */
    class Ball {
    public:
        /** ball position */
        cugl::Vec2 position;
        /** ball angle */
        float angle;
        /** The drawing scale of the joystick ball */
        cugl::Vec2 _drawBallScale;
        
    public:
        Ball();
        /**
         * Allocates a joystick ball by setting its position and angle.
         *
         * @param p         The position
         * @param ang     The angle
         */
        Ball(const cugl::Vec2 p, float ang);
        
        /**
         * Allocates a joystick ball by setting its position .
         *
         * @param p         The position
         */
        Ball(const cugl::Vec2 p);
        
        /**
         * Moves the joystick ball one animation frame
         */
        void update(const cugl::Vec2 p, float ang);
    };
    
private:
    /** The texture for the joystick ball sprite. */
    std::shared_ptr<cugl::Texture> _ballTexture;
    
    /** The texture for the joystick base sprite. */
    std::shared_ptr<cugl::Texture> _baseTexture;
    
    /** The drawing scale of the joystick base  */
    cugl::Vec2 _drawBaseScale;
    
    /** Whether the joystick is active or not  */
    float _active;
    
    /** The position of a general joystick */
    cugl::Vec2 _basePosition;
    
    std::shared_ptr<Ball> _joyBall;
    
    /** The radius of a general joystick base */
    float _radius;
    
    
#pragma mark The Base
public:
    
    JoyStick();
    
    JoyStick(const cugl::Vec2 p, cugl::Vec2 scale);
    
    /**
     * Spawns joystick at the given location.
     *
     * @param p     The joystick position.
     */
    void setActive(bool active);
    
    /**
     * Spawns joystick at the given location.
     *
     * @param p     The joystick position.
     */
    bool getActive() const { return _active; }
    
    /**
     * Returns the scale of this joystick.
     *
     * @return the scale of this joystick.
     */
    cugl::Vec2 getScale() const { return _drawBaseScale; }
    cugl::Vec2 getPositon() const { return _basePosition; }
        
    /**
     * Moves the active joystick.
     */
    void updateBasePos(cugl::Vec2 inputPos);
    
    /**
     * Moves the ball of an active joystick.
     */
    void updateBallPos(cugl::Vec2 inputDir, cugl::Vec2 inputPos);
    
    /**
     * Sets the image for a joystick.
     *
     * The sprite sheet information (size, number of columns) should have
     * been set in the initial JSON. If not, this texture will be ignored.
     *
     * @param valueBall the image for a single joystick ball.
     * @param valueBase the image for a single joystick base.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /**
     * Draws the active joystick to the sprite batch within the given bounds.
     *
     * @param batch     The sprite batch to draw to
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
    
    void setDrawScale(cugl::Vec2 scale);
};

#endif /* JoyStick_hpp */
