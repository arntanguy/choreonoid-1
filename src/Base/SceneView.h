/**
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BASE_SCENE_VIEW_H
#define CNOID_BASE_SCENE_VIEW_H

#include "View.h"
#include "exportdecl.h"

namespace cnoid {

class SceneWidget;
class SceneWidgetEventHandler;
class SceneBar;
class SgGroup;
class Item;

class CNOID_EXPORT SceneView : public View
{
public:
    static void initializeClass(ExtensionManager* ext);

    //! This function returns the default instance
    static SceneView* instance();

    /**
       This function is used in the BodyTrackingCameraItem implementation, but
       the implementation should not use the function and the function should be removed.
    */
    static std::vector<SceneView*> instances();

    /**
       If you want to add a custom mode button to the scene bar, use the SceneBar::addCustomModeButton function.
       \return Mode id
    */
    static int registerCustomMode(SceneWidgetEventHandler* modeHandler);

    /**
       If the corresponding custom mode button is added to the scene bar, remove it with
       the SceneBar::removeCustomModeButton function.
    */
    static void unregisterCustomMode(int mode);
    
    static int customModeId(const std::string& modeName);

    static SignalProxy<void(SceneView* view)> sigLastFocusViewChanged();
        
    SceneView();
    ~SceneView();
        
    SceneWidget* sceneWidget();
    SgGroup* scene();

    bool setCustomMode(int mode);
    int customMode() const;
        
protected:
    virtual void onFocusChanged(bool on) override;
    virtual QWidget* indicatorOnInfoBar() override;
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;
        
private:
    class Impl;
    Impl* impl;
};

}

#endif
