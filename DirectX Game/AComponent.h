#pragma once
#include <string>
class GameObject; 

class AComponent {
public:
    typedef std::string String;
    enum ComponentType {
        NotSet = -1,
        Script = 0,
        Renderer = 1,
        Input = 2,
        Physics = 3,
        TransformComp = 4, 
        MeshComp = 5       
    };

    AComponent(const String& name, ComponentType type, GameObject* owner)
        : owner(owner), type(type), name(name) {}
    virtual ~AComponent() {}

    void attachOwner(GameObject* o) { owner = o; }
    void detachOwner() { owner = nullptr; }
    GameObject* getOwner() const { return owner; }
    ComponentType getType() const { return type; }
    String getName() const { return name; }

    virtual void perform(float /*deltaTime*/) = 0;

protected:
    GameObject* owner = nullptr;
    ComponentType type = NotSet;
    String name;
};
