/*
 * $Revision: 373 $ $Date: 2010-07-20 10:50:15 -0700 (Tue, 20 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_MESH_GEOMETRY_H_
#define _VESTA_MESH_GEOMETRY_H_

#include <Eigen/Core>
#include "Geometry.h"
#include "Submesh.h"
#include "Material.h"
#include <vector>

namespace vesta
{
class PrimitiveBatch;
class VertexArray;
class TextureMapLoader;
class GLVertexBuffer;

/** MeshGeometry is a Geometry object for triangle meshes, typically
  * loaded from a 3D model file.
  */
class MeshGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    MeshGeometry();
    virtual ~MeshGeometry();

    void render(RenderContext& rc,
                float cameraDistance,
                double animationClock) const;
    void renderShadow(RenderContext& rc,
                      float cameraDistance,
                      double animationClock) const;

    float boundingSphereRadius() const;

    void addSubmesh(Submesh* submesh);
    void addMaterial(Material* material);

    unsigned int materialCount() const
    {
        return m_materials.size();
    }

    Material* material(unsigned int index) const
    {
        if (index < m_materials.size())
        {
            return m_materials[index].ptr();
        }
        else
        {
            return 0;
        }
    }

    Eigen::Vector3f meshScale() const
    {
        return m_meshScale;
    }

    /** Set the scale factor that will be applied along each axis of the
      * mesh. The default scale is one in all directions.
      */
    void setMeshScale(const Eigen::Vector3f& scale)
    {
        m_meshScale = scale;
    }

    /** Set the uniform scaling factor for the mesh. */
    void setMeshScale(float scale)
    {
        m_meshScale.fill(scale);
    }

    /** Return the bounding box of the mesh, not accounting for the mesh
      * scale factor.
      */
    BoundingBox meshBoundingBox() const
    {
        return m_boundingBox;
    }

    void setMeshChanged();

    bool mergeSubmeshes();
    bool uniquifyVertices(float positionTolerance = 0.0f, float normalTolerance = 0.0f, float texCoordTolerance = 0.0f);

    static MeshGeometry* loadFromFile(const std::string& filename, TextureMapLoader* textureLoader);

private:
    void freeSubmeshBuffers() const;
    bool realize() const;

protected:
    virtual bool handleRayPick(const Eigen::Vector3d& pickOrigin,
                               const Eigen::Vector3d& pickDirection,
                               double* distance ) const;

private:
    std::vector< counted_ptr<Submesh> > m_submeshes;
    std::vector< counted_ptr<Material> > m_materials;
    float m_boundingSphereRadius;
    BoundingBox m_boundingBox;
    Eigen::Vector3f m_meshScale;

    // TODO: these values are only mutable because the render() method
    // must be const. Consider changing this requirement.
    mutable std::vector<GLVertexBuffer*> m_submeshBuffers;
    mutable bool m_hwBuffersCurrent;
};

}

#endif // _VESTA_MESH_GEOMETRY_H_
