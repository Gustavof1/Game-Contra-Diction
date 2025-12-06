#include <GL/glew.h>
#include <SDL_ttf.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Font.h"
#include "../UI/UIElement.h"

Renderer::Renderer(SDL_Window *window)
: mBaseShader(nullptr)
, mLightShader(nullptr)
, mActiveShader(nullptr)
, mWindow(window)
, mContext(nullptr)
, mOrthoProjection(Matrix4::Identity)
{

}

Renderer::~Renderer()
{
    delete mSpriteVerts;
    mSpriteVerts = nullptr;
    if (mLightShader)
    {
        mLightShader->Unload();
        delete mLightShader;
    }
}

bool Renderer::Initialize(float width, float height)
{
    // Specify version 3.3 (core profile)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Enable double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Force OpenGL to use hardware acceleration
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Turn on vsync
    SDL_GL_SetSwapInterval(1);

    // Create an OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }

	// Make sure we can create/compile shaders
	if (!LoadShaders()) {
		SDL_Log("Failed to load shaders.");
		return false;
	}

    // Create quad for drawing sprites
    CreateSpriteVerts();

    // Set the clear color to light grey
    glClearColor(0.419f, 0.549f, 1.0f, 1.0f);

    // Enable alpha blending on textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create orthografic projection matrix
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    mBaseShader->SetActive();
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    mBaseShader->SetIntegerUniform("uTexture", 0);

    mLightShader->SetActive();
    mLightShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    mLightShader->SetIntegerUniform("uTexture", 0);

    // Activate shader
    mBaseShader->SetActive();

    if (TTF_Init() != 0) {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    return true;
}

void Renderer::Shutdown()
{
    // Destroy textures
    for (auto i : mTextures)
    {
        i.second->Unload();
        delete i.second;
    }
    mTextures.clear();

    for (auto i : mFonts)
    {
        i.second->Unload();
        delete i.second;
    }
    mFonts.clear();

    mBaseShader->Unload();
    delete mBaseShader;

    TTF_Quit();

    SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
}

void Renderer::Clear()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(RendererMode mode, const Matrix4 &modelMatrix, const Vector2 &cameraPos, VertexArray *vertices,
                    const Vector3 &color, Texture *texture, const Vector4 &textureRect, float textureFactor, float alpha, bool isVegetation)
{
    mActiveShader->SetMatrixUniform("uWorldTransform", modelMatrix);
    mActiveShader->SetVectorUniform("uColor", Vector4(color.x, color.y, color.z, 1.0f));
    mActiveShader->SetVectorUniform("uTexRect", textureRect);
    mActiveShader->SetVectorUniform("uCameraPos", cameraPos);
    mActiveShader->SetFloatUniform("uGlobalAlpha", alpha);
    mActiveShader->SetFloatUniform("uIsVegetation", isVegetation ? 1.0f : 0.0f);

    if(vertices)
    {
        vertices->SetActive();
    }

    if(texture)
    {
        texture->SetActive();
        mActiveShader->SetFloatUniform("uTextureFactor", textureFactor);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0);
        mActiveShader->SetFloatUniform("uTextureFactor", 0.0f);
    }

    if (mode == RendererMode::LINES)
    {
        glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
    }
    else if(mode == RendererMode::TRIANGLES)
    {
        glDrawElements(GL_TRIANGLES, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
    }
}

void Renderer::DrawRect(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                        const Vector2 &cameraPos, RendererMode mode, float alpha)
{
    Matrix4 model = Matrix4::CreateScale(Vector3(size.x, size.y, 1.0f)) *
                    Matrix4::CreateRotationZ(rotation) *
                    Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));

    Draw(mode, model, cameraPos, mSpriteVerts, color, nullptr, Vector4::UnitRect, 1.0f, alpha);
}

void Renderer::DrawTexture(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                           Texture *texture, const Vector4 &textureRect, const Vector2 &cameraPos, const Vector2 &scale,
                           float textureFactor, float alpha, bool isVegetation)
{
    Matrix4 model = Matrix4::CreateScale(Vector3(size.x * scale.x, size.y * scale.y, 1.0f)) *
                    Matrix4::CreateRotationZ(rotation) *
                    Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));

    Draw(RendererMode::TRIANGLES, model, cameraPos, mSpriteVerts, color, texture, textureRect, textureFactor, alpha, isVegetation);
}

void Renderer::DrawGeometry(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                            const Vector2 &cameraPos, VertexArray *vertexArray, RendererMode mode)
{
    Matrix4 model = Matrix4::CreateScale(Vector3(size.x, size.y, 1.0f)) *
                    Matrix4::CreateRotationZ(rotation) *
                    Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));

    Draw(mode, model, cameraPos, vertexArray, color);
}

void Renderer::Present()
{
	// Swap the buffers
	SDL_GL_SwapWindow(mWindow);
}

bool Renderer::LoadShaders()
{
	// Create sprite shader
	mBaseShader = new Shader();
	if (!mBaseShader->Load("../Shaders/Base")) {
		return false;
	}

    mLightShader = new Shader();
    if (!mLightShader->Load("../Shaders/Light")) {
        return false;
    }

	mBaseShader->SetActive();
    mActiveShader = mBaseShader;

    return true;
}

void Renderer::CreateSpriteVerts()
{
    float vertices[] = {
        // pos.x, pos.y, tex.u, tex.v
        -0.5f, -0.5f, 0.0f, 0.0f, // Top-left
         0.5f, -0.5f, 1.0f, 0.0f, // Top-right
         0.5f,  0.5f, 1.0f, 1.0f, // Bottom-right
        -0.5f,  0.5f, 0.0f, 1.0f  // Bottom-left
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    mSpriteVerts = new VertexArray(vertices, 4, indices, 6);
}

void Renderer::AddUIElement(UIElement* element)
{
    mUIElements.emplace_back(element);

    std::sort(mUIElements.begin(), mUIElements.end(), [](UIElement* a, UIElement* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Renderer::RemoveUIElement(UIElement* element)
{
    auto iter = std::find(mUIElements.begin(), mUIElements.end(), element);
    if (iter != mUIElements.end())
    {
        mUIElements.erase(iter);
    }
}

void Renderer::DrawUI(Shader* shader)
{
    shader->SetVectorUniform("uCameraPos", Vector2::Zero);
    shader->SetFloatUniform("uGlobalAlpha", 1.0f);

    for (auto ui : mUIElements)
    {
        ui->Draw(shader);
    }
}

Texture* Renderer::GetTexture(const std::string& fileName)
{
    Texture* tex = nullptr;
    auto iter = mTextures.find(fileName);
    if (iter != mTextures.end())
    {
        tex = iter->second;
    }
    else
    {
        tex = new Texture();
        if (tex->Load(fileName))
        {
            mTextures.emplace(fileName, tex);
            return tex;
        }
        else
        {
            delete tex;
            return nullptr;
        }
    }
    return tex;
}

Font* Renderer::GetFont(const std::string& fileName)
{
    auto iter = mFonts.find(fileName);
    if (iter != mFonts.end())
    {
        return iter->second;
    }
    else
    {
        Font* font = new Font();
        if (font->Load(fileName))
        {
            mFonts.emplace(fileName, font);
        }
        else
        {
            font->Unload();
            delete font;
            font = nullptr;
        }
        return font;
    }
}

void Renderer::SetLightUniforms(const std::vector<Vector2>& lightPos, const std::vector<float>& lightRadii, const std::vector<Vector3>& lightColors, const Vector3& ambient)
{
    SetActiveShader(mLightShader);
    
    mLightShader->SetFloatUniform("uTime", static_cast<float>(SDL_GetTicks()) / 1000.0f);
    
    // Set ambient light
    mLightShader->SetVectorUniform("uAmbientLight", ambient);
    
    // Set number of lights
    int numLights = std::min((int)lightPos.size(), 16);
    mLightShader->SetIntegerUniform("uNumLights", numLights);
    
    // Set light positions and radii
    // We need to pass arrays. Shader::SetVectorUniform doesn't support arrays directly usually, 
    // but let's check Shader.h/cpp. If not, we loop.
    
    for (int i = 0; i < numLights; i++)
    {
        std::string posName = "uLightPositions[" + std::to_string(i) + "]";
        std::string radName = "uLightRadii[" + std::to_string(i) + "]";
        std::string colName = "uLightColors[" + std::to_string(i) + "]";
        
        mLightShader->SetVectorUniform(posName.c_str(), lightPos[i]);
        mLightShader->SetFloatUniform(radName.c_str(), lightRadii[i]);
        mLightShader->SetVectorUniform(colName.c_str(), lightColors[i]);
    }
}

void Renderer::SetFlashlightUniforms(bool active, const Vector2& pos, const Vector2& dir)
{
    SetActiveShader(mLightShader);
    mLightShader->SetIntegerUniform("uFlashlightActive", active ? 1 : 0);
    if (active)
    {
        mLightShader->SetVectorUniform("uFlashlightPos", pos);
        mLightShader->SetVectorUniform("uFlashlightDir", dir);
        mLightShader->SetFloatUniform("uFlashlightAngle", 0.5f); // Approx 30 degrees half-angle
    }
}

void Renderer::SetActiveShader(Shader* shader)
{
    mActiveShader = shader;
    mActiveShader->SetActive();
}