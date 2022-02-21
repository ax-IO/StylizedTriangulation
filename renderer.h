#ifndef RENDERER_H
#define RENDERER_H

#include "triangulation.h"

class Renderer
{
public:
    void render(const Triangulation&, unsigned int);
};


#endif // RENDERER_H
