/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SKY_LAYER_H_
#define _VESTA_SKY_LAYER_H_

#include "Object.h"


namespace vesta
{

class RenderContext;

class SkyLayer : public Object
{
public:
    SkyLayer()
    {
    }

    virtual ~SkyLayer()
    {
    }

    /** Draw the sky layer. Subclasses must implement this method.
      */
    virtual void render(RenderContext& rc) = 0;

    /** Return true if the layer is visible, false if it is not. */
    bool isVisible() const
    {
        return m_visible;
    }

    /** Set whether the layer should be visible or hidden. */
    void setVisibility(bool visible)
    {
        m_visible = visible;
    }

private:
    bool m_visible;
};

}
#endif // _VESTA_SKY_LAYER_H_