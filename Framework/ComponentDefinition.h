
#pragma once
#ifndef HELIUM_FRAMEWORK_COMPONENT_DESCRIPTOR_H
#define HELIUM_FRAMEWORK_COMPONENT_DESCRIPTOR_H

#include "Engine/Asset.h"
#include "Engine/Components.h"

namespace Helium
{
    // Generally components are created by filling out a component definition and using that to allocate a component.
    // In order to support the case where components depend on each other's presence to finish initialization, we use 
    // two phases. (CreateComponent, then FinalizeComponent) Definitions can reference other definitions, so we retain
    // a reference to the created component to help in wiring components directly to components
    //
    // TODO: Support abstract Assets so we can make a few of these functions pure virtual
    class HELIUM_FRAMEWORK_API ComponentDefinition : public Helium::Asset
    {
    public:
        HELIUM_DECLARE_OBJECT(ComponentDefinition, Helium::Asset);

        // Allocates a component. Initialization is not complete without calling FinalizeComponent()
        inline Helium::Component *CreateComponent(struct Components::ComponentSet &_target) const;

        // Implemented by child classes to allocate a component of the appropriate type and return it
        inline virtual Helium::Component *CreateComponentInternal(struct Components::ComponentSet &_target) const;

        // Implemented by child classes to finish setting up the component.
        inline virtual void FinalizeComponent() const;

        // Gets the component that this definition generated previously
        inline Helium::Component *GetCreatedComponent() const;

    private:
        mutable Helium::ComponentPtr<Component> m_Instance;
    };
    typedef Helium::StrongPtr<ComponentDefinition> ComponentDefinitionPtr;
}

#include "Framework/ComponentDefinition.inl"

#endif