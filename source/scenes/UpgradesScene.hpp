//
//  UpgradesScene.hpp
//  RS
//
//  Created by Dasha Griffiths on 4/13/24.
//

#ifndef UpgradesScene_hpp
#define UpgradesScene_hpp

#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include "../models/Upgradeable.hpp"


using namespace cugl;
using namespace std;
/**
 *
 */
class UpgradesScene : public cugl::Scene2 {
public:
    /**
     * The upgrade choice.
     *
     * This state allows the top level application to know what the user
     * chose.
     */
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants upgrade option 1 */
        UPGRADE_1,
        /** User wants upgrade option 2 */
        UPGRADE_2
    };
    
    /**
     * upgrade classes.
     *
     * Should be consistent with GameScene.
     */
    enum upgrades {HEALTH, PARRY, RANGED, MELEE, DODGE, DEFENSE,  MOVEMENT};
    
protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** The button for upgrade option 1 */
    std::shared_ptr<cugl::scene2::Button> _option1;
    /** The button to confirm upgrade option 1*/
    std::shared_ptr<cugl::scene2::Button> _confirm1;
    shared_ptr<scene2::Label> _option1Name;
    shared_ptr<scene2::Label> _option1Descrip;
    shared_ptr<scene2::Label> _option1Level;
    
    
    /** The button for upgrade option 2 */
    std::shared_ptr<cugl::scene2::Button> _option2;
    /** The button to confirm upgrade option 2*/
    std::shared_ptr<cugl::scene2::Button> _confirm2;
    shared_ptr<scene2::Label> _option2Name;
    shared_ptr<scene2::Label> _option2Descrip;
    shared_ptr<scene2::Label> _option2Level;
    
    int _displayedAttribute1;
    int _displayedAttribute2;
    /** The player choice */
    Choice _choice;
    bool _active;
    
public:
#pragma mark -
    
    /** enum number of selcted upgrade */
    int _selectedUpgrade;
#pragma mark Constructors
    /**
     * Creates a new  menu scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    UpgradesScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~UpgradesScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * Initializes the controller contents.
     *
     * In previous labs, this method "started" the scene.  But in this
     * case, we only use to initialize the scene user interface.  We
     * do not activate the user interface yet, as an active user
     * interface will still receive input EVEN WHEN IT IS HIDDEN.
     *
     * That is why we have the method {@link #setActive}.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    /**
     * Updates button infomartionon on scene
     *
     *
     * @param attributes available payer upgrades
     */
    void updateScene(std::vector<int> attributes,std::vector<std::shared_ptr<Upgradeable>> availableUpgrades);
    
    void setButtonText(int upgrade, int buttonType);

    /**
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    
    /**
     * Returns a random Upgrade
     *
     * This will make sure upgrades returned are .
     *
     * @return the user's menu choice.
     */
    Choice getChoice() const { return _choice; }
        

};

#endif /* UpgradesScene_hpp */
    
