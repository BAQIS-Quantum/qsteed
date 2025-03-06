#pragma once
#include "backend.h"
#include "layout.h"
#include <map> 


class Model
{
private:
    Backend _backend;

public:
    Layout initial_layout;
    Layout final_layout;

    Model() = default;

    Model(Layout initial_layout)
        : initial_layout(initial_layout) {}

    Model(LayoutStructure initial_layout)
        : initial_layout(Layout(initial_layout)) {}

    Model(Layout initial_layout, Layout final_layout)
        : initial_layout(initial_layout), final_layout(final_layout) {}

    Model(LayoutStructure initial_layout, LayoutStructure final_layout)
        : initial_layout(Layout(initial_layout)), final_layout(Layout(final_layout)) {}


    // Model(Backend backend);
    // Backend get_backend() const; 
};