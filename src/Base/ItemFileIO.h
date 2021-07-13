#ifndef CNOID_BASE_ITEM_FILE_IO_H
#define CNOID_BASE_ITEM_FILE_IO_H

#include "ItemList.h"
#include <cnoid/Referenced>
#include <string>
#include <vector>
#include "exportdecl.h"

class QWidget;

namespace cnoid {

class Item;
class ItemManager;
class ItemFileDialog;
class Mapping;

class CNOID_EXPORT ItemFileIO : public Referenced
{
public:
    enum API {
        Load = 1 << 0,
        Options = 1 << 1,
        OptionPanelForLoading = 1 << 2,
        Save = 1 << 3,
        OptionPanelForSaving = 1 << 4,
    };
    enum InterfaceLevel { Standard, Conversion, Internal };
    enum InvocationType { Direct, Dialog, DragAndDrop };
    class Impl;

    ~ItemFileIO();

    bool isFormat(const std::string& format) const;
    int api() const;
    bool hasApi(int api) const;
    const std::string& caption() const;
    const std::string& fileTypeCaption() const;
    void addFormatAlias(const std::string& format);
    [[deprecated("Use addFormatAlias")]]
    void addFormatIdAlias(const std::string& format){
        addFormatAlias(format);
    }

    // deprecated. This is internally used for specifing SceneItem's extensions dynamically.
    // The dynamic extension specification should be achieved by a signal to update the
    // extensions and usual the registerExtensions function.
    void setExtensionFunction(std::function<std::string()> func);

    std::vector<std::string> extensions() const;
    
    int interfaceLevel() const;

    // Set the invocation type before calling the loadItem or saveItem function
    // if the invocation type is not "Direct".
    void setInvocationType(int type);

    // Load API
    Item* loadItem(
        const std::string& filename,
        Item* parentItem = nullptr, bool doAddition = true, Item* nextItem = nullptr,
        const Mapping* options = nullptr);

    bool loadItem(
        Item* item, const std::string& filename,
        Item* parentItem = nullptr, bool doAddition = true, Item* nextItem = nullptr,
        const Mapping* options = nullptr);

    // Save API
    bool saveItem(Item* item, const std::string& filename, const Mapping* options = nullptr);
    
    // Options API
    virtual void resetOptions();
    virtual void storeOptions(Mapping* options);
    virtual bool restoreOptions(const Mapping* options);
    
    // OptionPanelForLoading API
    virtual QWidget* getOptionPanelForLoading();
    virtual void fetchOptionPanelForLoading();

    // OptionPanelForPostLoading API (pending)
    //virtual QWidget* getOptionPanelForPostLoading(Item* item);
    //virtual void fetchOptionPanelForPostLoading(Item* item);
    //virtual bool doPostProcessForLoading(Item* item);

    // OptionPanelForSaving API
    virtual QWidget* getOptionPanelForSaving(Item* item);
    virtual void fetchOptionPanelForSaving();

    Item* parentItem();
    int invocationType() const;

    bool isRegisteredForSingletonItem() const;
    Item* findSingletonItemInstance() const;

    void setItemClassInfo(Referenced* info);
    const Referenced* itemClassInfo() const;

    static std::vector<std::string> separateExtensions(const std::string& multiExtString);

protected:
    ItemFileIO(const std::string& format, int api);
    ItemFileIO(const ItemFileIO& org);
    ItemFileIO();
    void copyFrom(const ItemFileIO& org);
    
    void setApi(int apiSet);
    void setCaption(const std::string& caption);
    void setFileTypeCaption(const std::string& caption);
    void setExtension(const std::string& extension);
    void setExtensions(const std::vector<std::string>& extensions);
    void setInterfaceLevel(InterfaceLevel level);

    // Load API
    virtual bool load(Item* item, const std::string& filename);
    virtual Item* createItem();

    // Save API
    virtual bool save(Item* item, const std::string& filename);
    
    std::ostream& os();
    void putWarning(const std::string& message);
    void putError(const std::string& message);

    /**
       When the file is loaded as a composite item tree and the item
       that actually loads the file is different from the top item,
       the following function must be called with the corresponding item.
    */
    void setActuallyLoadedItem(Item* item);
    
private:
    Impl* impl;
    friend class ItemManager;
    friend class ItemFileDialog;
};

typedef ref_ptr<ItemFileIO> ItemFileIOPtr;

template<class ItemType>
class ItemFileIoBase : public ItemFileIO
{
public:
    ItemFileIoBase(const std::string& format, int api)
        : ItemFileIO(format, api) {
    }
    virtual Item* createItem() override {
        if(isRegisteredForSingletonItem()){
            return findSingletonItemInstance();
        }
        return new ItemType;
    }
    virtual bool load(Item* item, const std::string& filename) override final {
        if(auto derived = dynamic_cast<ItemType*>(item)){
            return load(derived, filename);
        }
        return false;
    }
    virtual bool load(ItemType* /* item */, const std::string& /* filename */) {
        return false;
    }
    virtual bool save(Item* item, const std::string& filename) override final {
        if(auto derived = dynamic_cast<ItemType*>(item)){
            return save(derived, filename);
        }
        return false;
    }
    virtual bool save(ItemType* /* item */, const std::string& /* filename */) {
        return false;
    }
    virtual QWidget* getOptionPanelForSaving(Item* item) override final {
        return getOptionPanelForSaving(static_cast<ItemType*>(item));
    }
    virtual QWidget* getOptionPanelForSaving(ItemType* item) {
        return nullptr;
    }
};

}

#endif
