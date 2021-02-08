#include "BodyItem.h"
#include "BodyOverwriteAddon.h"
#include <cnoid/ItemFileIO>  
#include <cnoid/BodyLoader>
#include <cnoid/StdBodyWriter>
#include <cnoid/StdSceneWriter>
#include <cnoid/SceneItemFileIO>
#include <cnoid/ItemManager>
#include <cnoid/SceneGraph>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

ItemFileIO* bodyFileIO;
ItemFileIO* meshFileIO;
ItemFileIO* stdSceneFileOutput;


class BodyFileIO : public ItemFileIOBase<BodyItem>
{
    BodyLoader bodyLoader;
    unique_ptr<StdBodyWriter> bodyWriter;
    
public:
    BodyFileIO()
        : ItemFileIOBase<BodyItem>("CHOREONOID-BODY", Load | Save)
    {
        setCaption(_("Body"));
        setExtensions({ "body", "yaml", "yml", "wrl" });
        addFormatIdAlias("OpenHRP-VRML-MODEL");

        bodyLoader.setMessageSink(os());
    }

    virtual bool load(BodyItem* item, const std::string& filename) override
    {
        BodyPtr newBody = new Body;
        if(!bodyLoader.load(newBody, filename)){
            return false;
        }
        item->setBody(newBody);

        if(item->name().empty()){
            item->setName(newBody->modelName());
        } else {
            newBody->setName(item->name());
        }

        auto itype = invocationType();
        if(itype == Dialog || itype == DragAndDrop){
            item->setChecked(true);
        }
        
        return true;
    }

    virtual bool save(BodyItem* item, const std::string& filename) override
    {
        if(!bodyWriter){
            bodyWriter.reset(new StdBodyWriter);
        }
        if(bodyWriter->writeBody(item->body(), filename)){
            item->getAddon<BodyOverwriteAddon>()->clearOverwriteItems();
            return true;
        }
        return false;
    }
};

class SceneFileIO : public SceneItemFileIO
{
public:
    SceneFileIO()
    {
        setCaption(_("Body"));
        setFileTypeCaption(_("Scene / Mesh"));
    }

    virtual Item* createItem() override
    {
        return new BodyItem;
    }

    virtual bool load(Item* item, const std::string& filename) override
    {
        SgNode* shape = loadScene(filename);
        if(!shape){
            return false;
        }

        auto bodyItem = static_cast<BodyItem*>(item);
        bodyItem->body()->rootLink()->addShapeNode(shape);

        auto itype = invocationType();
        if(itype == Dialog || itype == DragAndDrop){
            item->setChecked(true);
        }
        
        return true;
    }
};

class StdSceneFileOutput : public ItemFileIOBase<BodyItem>
{
    unique_ptr<StdSceneWriter> sceneWriter;

public:
    StdSceneFileOutput()
        : ItemFileIOBase<BodyItem>("STD-SCENE-FILE", Save)
    {
        setCaption(_("Standard scene file"));
        setExtensions({ "scen" });
        setInterfaceLevel(Conversion);
    }

    virtual bool save(BodyItem* item, const std::string& filename) override
    {
        bool saved = false;
        if(!sceneWriter){
            sceneWriter.reset(new StdSceneWriter);
            sceneWriter->setIndentWidth(1);
        }
        auto body = item->body();
        int numLinks = body->numLinks();
        vector<SgNode*> shapes;
        shapes.reserve(numLinks);
        vector<int> shapeIndicesToClearName;
        shapeIndicesToClearName.reserve(numLinks);
        const Isometry3 T0 = body->rootLink()->T();
        for(auto& link : body->links()){
            bool stripped;
            if(SgNode* shape = strip(link->shape(), stripped)){
                if(!link->T().isApprox(T0)){
                    auto transform = new SgPosTransform(T0.inverse() * link->T());
                    if(stripped){
                        transform->addChild(shape);
                    } else {
                        link->shape()->copyChildrenTo(transform);
                    }
                    transform->setName(link->name());
                    shape = transform;
                } else if(shape->name().empty()){
                    shape->setName(link->name());
                    shapeIndicesToClearName.push_back(shapes.size());
                }
                shapes.push_back(shape);
            }
        }
        if(!shapes.empty()){
            saved = sceneWriter->writeScene(filename, shapes);
        }
        // Clear temporary names
        for(auto& index : shapeIndicesToClearName){
            shapes[index]->setName("");
        }
            
        return saved;
    }

    SgNode* strip(SgGroup* group, bool& out_stripped)
    {
        int n = group->numChildren();
        if(n >= 2){
            out_stripped = false;
            return group;
        } else if(n == 1){
            out_stripped = true;
            return group->child(0);
        }
        return nullptr;
    }
};

}

namespace cnoid {

void registerBodyItemFileIoSet(ItemManager& im)
{
    ::bodyFileIO = new BodyFileIO;
    im.registerFileIO<BodyItem>(::bodyFileIO);

    ::meshFileIO = new SceneFileIO;
    im.registerFileIO<BodyItem>(::meshFileIO);

    ::stdSceneFileOutput = new StdSceneFileOutput;
    im.registerFileIO<BodyItem>(::stdSceneFileOutput);
}

}


ItemFileIO* BodyItem::bodyFileIO()
{
    return ::bodyFileIO;
}


ItemFileIO* BodyItem::meshFileIO()
{
    return ::meshFileIO;
}