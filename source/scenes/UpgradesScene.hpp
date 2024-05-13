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

/**
 * A pop-up menu scene that allows users to select upgrades
 */
class UpgradesScene : public cugl::Scene2 {
public:
    
    /**
     * Level classes of upgrades.
     *
     * Should be consistent with GameScene.
     */
    enum upgradeLevel {NORMAL, RARE, EPIC, LEGENDARY, MAX};
    
protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
#pragma mark - Scene Nodes
    /** The button for upgrade option 1 */
    std::shared_ptr<cugl::scene2::Button> _option1;
    /** The button to confirm upgrade option 1*/
    std::shared_ptr<cugl::scene2::Button> _confirm1;
    std::shared_ptr<scene2::Label> _option1Name;
    std::shared_ptr<scene2::Label> _option1Descrip;
    std::shared_ptr<scene2::Label> _option1Level;
    std::shared_ptr<scene2::TexturedNode> _option1Icon;
    
    /** The button for upgrade option 2 */
    std::shared_ptr<cugl::scene2::Button> _option2;
    /** The button to confirm upgrade option 2*/
    std::shared_ptr<cugl::scene2::Button> _confirm2;
    std::shared_ptr<scene2::Label> _option2Name;
    std::shared_ptr<scene2::Label> _option2Descrip;
    std::shared_ptr<scene2::Label> _option2Level;
    std::shared_ptr<scene2::TexturedNode> _option2Icon;
    
#pragma mark - Icon Textures
    std::shared_ptr<Texture> _healthTexture;
    std::shared_ptr<Texture> _parryTexture;
    std::shared_ptr<Texture> _shieldTexture;
    std::shared_ptr<Texture> _atkSdTexture;
    std::shared_ptr<Texture> _dashTexture;
    std::shared_ptr<Texture> _bowTexture;
    std::shared_ptr<Texture> _swordTexture;
    
#pragma mark - Scene State
    std::pair<UpgradeType, int> _displayedAttribute1;
    std::pair<UpgradeType, int> _displayedAttribute2;
    int _level;
    bool _selectedUpgrade;
    UpgradeType _upgrade;
    
    /**
     * sets the corresponding set of buttons with the upgrade information
     */
    void setButtonText(UpgradeType upgrade, int level, int button);
    
public:
#pragma mark -
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
    
#pragma mark Scene Configuration
    /**
     * Updates button infomartionon on scene
     *
     * @param attributes available payer upgrades
     */
    void updateScene(std::pair<Upgradeable, Upgradeable> upgradeOptions);

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
    
#pragma mark Scene Selection
    /**
     * @return whether an upgrade has been chosen
     */
    bool hasSelectedUpgrade() const { return _selectedUpgrade;}
    
    /**
     * @return the selected upgrade, if chosen.
     */
    UpgradeType getChoice()  { return _upgrade; }
    
    /**
     * @return the level of the selected upgrade, if chosen.
     */
    int getUpgradeLevel(){ return _level; }
    
    /**
     * Input can be viewed as being processed simultaneously but only one UI element across different active screen components should be processing the user input.
     *
     * @return whether an event with the given position will be processed by the Upgrades Menu
     */
    bool isInputProcessed(Vec2 pos);

};

#endif /* UpgradesScene_hpp */
    
