#pragma once
#include <string>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>

class Texture
{
public:
	Texture();
	~Texture();

	bool Load(const std::string& fileName);
	void Unload();

	void CreateFromSurface(struct SDL_Surface* surface);

	void SetActive(int index = 0) const;

    static GLenum SDLFormatToGL(SDL_PixelFormat* fmt);

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }
    const std::string& GetFileName() const { return mFileName; }

	unsigned int GetTextureID() const { return mTextureID; }

private:
    std::string mFileName;
	unsigned int mTextureID;
	int mWidth;
	int mHeight;
};
