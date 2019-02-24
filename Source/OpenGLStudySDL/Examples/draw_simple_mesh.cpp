
#include "render_proxy.h"

#include "..\Core\camera.h"
#include "..\Core\display.h"
#include "..\Core\mesh.h"
#include "..\Core\shader.h"
#include "..\Core\texture.h"
#include "..\Core\transform.h"

#include <memory>

class RenderSimpleMesh : public RenderProxy
{
public:
    RenderSimpleMesh(void) = default;
    virtual ~RenderSimpleMesh(void) = default;

protected:
    virtual void InitInternal(void);
    virtual void CleanUpInternal(void);
    virtual void DrawInternal(Display& display);

private:  
      
    std::unique_ptr<Shader> pShader_;
    std::unique_ptr<Texture> pTexture_;
    std::unique_ptr<Mesh> pMesh1_;
    std::unique_ptr<Mesh> pMesh2_;
    std::unique_ptr<Camera> pCamera_;
    Transform transform_;
    float counter_ = 0.0f;
};

RenderSimpleMesh staticCraete;

//-----------------------------------------------------------------------------------

Vertex vertices[] = 
{
    Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f)),
    Vertex(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec2(0.5f, 1.0f)),
    Vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f)),
};

unsigned int indices[] = {0, 1, 2};

void RenderSimpleMesh::InitInternal(void)
{
    pShader_.reset(new Shader("../../Resource/OpenGLSDL/Shader/basic_shader"));
    pTexture_.reset(new Texture("../../Resource/OpenGLSDL/Image/bricks.jpg"));
    
    pMesh1_.reset(new Mesh(vertices, sizeof(vertices) / sizeof(vertices[0]), indices, sizeof(indices) / sizeof(indices[0])));
    pMesh2_.reset(new Mesh("../../Resource/OpenGLSDL/Object/monkey3.obj"));
    
    pCamera_.reset(new Camera(glm::vec3(0.0f, 0.0f, -5.0f), 70.0f, float(WIDTH) / float(HEIGHT), 0.01f, 1000.0f));
}

void RenderSimpleMesh::CleanUpInternal(void)
{
}

void RenderSimpleMesh::DrawInternal(Display& display)
{
    display.Clear(0.0f, 0.15f, 0.5f, 1.0f);        

    float sinCounter = sinf(counter_);
    float cosCounter = cosf(counter_);

    transform_.GetPos().x = sinCounter;
    transform_.GetPos().z = cosCounter;
    transform_.GetRot().x = counter_ * 5.0f;
    transform_.GetRot().y = counter_ * 5.0f;
    transform_.GetRot().z = counter_ * 5.0f;
    transform_.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) * cosCounter);

    pShader_->Bind();
    pShader_->Update(transform_, *pCamera_);
    pTexture_->Bind(0);
    //pMesh1_->Draw();
    pMesh2_->Draw();

    display.Update();

    counter_ += 0.005f;
}
