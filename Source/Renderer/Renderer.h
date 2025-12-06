#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include "../Math.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Font.h"
#include "../UI/UIElement.h"

enum class RendererMode
{
    TRIANGLES,
    LINES
};

class Renderer
{
public:
	Renderer(SDL_Window* window);
	~Renderer();

	bool Initialize(float width, float height);
	void Shutdown();

    void SetZoom(float zoom);

    void DrawRect(const Vector2 &position, const Vector2 &size,  float rotation,
                  const Vector3 &color, const Vector2 &cameraPos, RendererMode mode, float alpha = 1.0f);

    void DrawTexture(const Vector2 &position, const Vector2 &size,  float rotation,
                     const Vector3 &color, Texture *texture,
                     const Vector4 &textureRect = Vector4::UnitRect,
                     const Vector2 &cameraPos = Vector2::Zero, const Vector2 &scale = Vector2(1.0f, 1.0f),
                     float textureFactor = 1.0f, float alpha = 1.0f, bool isVegetation = false);

    void DrawGeometry(const Vector2 &position, const Vector2 &size,  float rotation,
                      const Vector3 &color, const Vector2 &cameraPos, VertexArray *vertexArray, RendererMode mode);

    void Clear();
    void Present();

    void AddUIElement(class UIElement* element);
    void RemoveUIElement(class UIElement* element);
    void DrawUI(class Shader* shader);

    class VertexArray* GetSpriteVerts() { return mSpriteVerts; }

    // Getters
    class Texture* GetTexture(const std::string& fileName);
	class Shader* GetBaseShader() const { return mBaseShader; }
	class Shader* GetLightShader() const { return mLightShader; }
    class Font* GetFont(const std::string& fileName);

    void SetLightUniforms(const std::vector<Vector2>& lightPos, const std::vector<float>& lightRadii, const std::vector<Vector3>& lightColors, const Vector3& ambient);
    void SetFlashlightUniforms(bool active, const Vector2& pos, const Vector2& dir);
    void SetActiveShader(class Shader* shader);

private:
    void Draw(RendererMode mode, const Matrix4 &modelMatrix, const Vector2 &cameraPos, VertexArray *vertices,
              const Vector3 &color,  Texture *texture = nullptr, const Vector4 &textureRect = Vector4::UnitRect, float textureFactor = 1.0f, float alpha = 1.0f, bool isVegetation = false);


	bool LoadShaders();
    void CreateSpriteVerts();

	// Game
	class Game* mGame;

	// Basic shader
	class Shader* mBaseShader;
	class Shader* mLightShader;
    class Shader* mActiveShader;

    // Sprite vertex array
    class VertexArray *mSpriteVerts;

	// Window
	SDL_Window* mWindow;

	// OpenGL context
	SDL_GLContext mContext;

	// Ortho projection for 2D shaders
	Matrix4 mOrthoProjection;

    float mScreenWidth;
    float mScreenHeight;

    // Map of textures loaded
    std::unordered_map<std::string, class Texture*> mTextures;

    // Map of fonts
    std::unordered_map<std::string, class Font*> mFonts;

    std::vector<class UIElement*> mUIElements;
};