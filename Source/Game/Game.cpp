#include "../Core/Mobitech.h"

class GameScene : public Scene, IInputListener
{
    ShaderProgram* shader;

    vec2 vertices[3];
    vec4 colors[3];

public:
    
    GameScene()
    {  
        shader = Engine::rf.Load(ASSETS_ROOT + "Shaders\\diffuse.vs", ASSETS_ROOT + "Shaders\\diffuse.ps");        
        Input::GetInstance()->Register(this);

        vertices[0] = vec2(0.0f, 0.5f);
        vertices[1] = vec2(-0.5f, -0.5f);
        vertices[3] = vec2(0.5f, -0.5f);

        colors[0] = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        colors[1] = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        colors[2] = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
    ~GameScene() { Input::GetInstance()->Unregister(this); }

    virtual void Update(float dt)
    {
    
    }

    virtual void DrawFrame()
    {
        Renderer::GetInstance()->SetShaderProgram(shader);

        glVertexAttribPointer(shader->attributeLocations.position, 2, GL_FLOAT, GL_FALSE, 0, vertices[0].v);
        glEnableVertexAttribArray(shader->attributeLocations.position);

        glVertexAttribPointer(shader->attributeLocations.color, 4, GL_FLOAT, GL_FALSE, 0, colors);
        glEnableVertexAttribArray(shader->attributeLocations.color);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    virtual void OnTouchDown(int x, int y) {}
    virtual void OnTouchUp(int x, int y) {}
    virtual void OnMove(int x, int y) {}
};

void GameMain()
{
    Engine engine = *(Engine::GetInstance());
    engine.Initialize();
    
    GameScene gameScene;
    engine.SetScene(&gameScene);
    
    engine.Run();
    engine.Stop();
}

