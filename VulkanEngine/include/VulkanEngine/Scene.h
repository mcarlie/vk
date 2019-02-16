#ifndef SCENE_H
#define SCENE_H

#include <VulkanEngine/SceneObject.h>

#include <memory>

namespace VulkanEngine {

  class Scene : public SceneObject {

  public:

    /// Constructor.
    Scene();

    /// Destructor.
    virtual ~Scene();

  };

}

#endif /* SCENE_H */
