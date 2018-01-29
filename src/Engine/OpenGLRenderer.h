/*
 * An OpenGL renderer, based on OpenXcom's OpenGL.cpp 
 * (which, in turn, is based on bsnes's ruby.cpp?)
 */

#ifndef OPENXCOM_OPENGLRENDERER_H
#define OPENXCOM_OPENGLRENDERER_H

#ifndef __NO_OPENGL

#include <GLES2/gl2.h>
#include <string>
#include "Renderer.h"

namespace OpenXcom 
{

class Surface;
class Screen;


std::string strGLError(GLenum glErr);

#define glErrorCheck() {\
	static bool reported = false;\
	GLenum glErr;\
	if (OpenGLRenderer::checkErrors && !reported && (glErr = glGetError()) != GL_NO_ERROR)\
	{\
		reported = true;\
		\
		do \
		{ \
			Log(LOG_WARNING) << __FILE__ << ":" << __LINE__ << ": glGetError() complaint: " << strGLError(glErr);\
		} while (((glErr = glGetError()) != GL_NO_ERROR));\
	}\
}



class OpenGLRenderer :
	public Renderer
{
private:
	static const RendererType _rendererType = RENDERER_OPENGL;
    Screen &_screen;
	SDL_Window *_window;

    SDL_GLContext _ctx;

	SDL_Rect _dstRect;

    struct ScalerInfo
    {
        std::string name;
        std::string path;
        GLint program;
    };

    struct
    {
        GLint pos;
        GLint texCoord0;
        GLint MVP;
    } _shaderData;

	struct
	{

	};

    static std::vector<ScalerInfo> _scalers;
    static void _scanScalers();

public:
	static bool checkErrors;
    /// Create a dummy renderer to populate scalers
    OpenGLRenderer(Screen& gameScreen);
    /// Create an OpenGL (ES) renderer and make it current
	OpenGLRenderer(Screen& gameScreen, SDL_Window *window);
	~OpenGLRenderer() override;

    virtual std::vector<std::string> getUpscalers() override;

    void setUpscaler(int upscalerId) override;
    void setUpscalerByName(const std::string &scalerName) override;
	/// Sets the desired output rectangle.
	void setOutputRect(SDL_Rect *dstRect) override;
	void flip() override;
	RendererType getRendererType() override { return _rendererType; };
	void screenshot(const std::string &filename) const;
    /// Returns the renderer name
    std::string getRendererName() override {return "GL";}
};
}
#endif //__NO_OPENGL

#endif //OPENXCOM_OPENGLRENDERER_H