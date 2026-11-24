#pragma once

#include "../abstraction/OpenGlShape.hpp"


class Drawer
{
public:
    void draw(OpenGlShape&);
    void draw(OpenGlShapeWithTexture&);
};