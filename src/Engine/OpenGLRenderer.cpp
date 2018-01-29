// This file was copied from the bsnes project. 

// This is the license info, from ruby.hpp:

/*
  ruby
  version: 0.08 (2011-11-25)
  license: public domain
 */

#if !defined (__NO_OPENGL)

#include <SDL.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "OpenGLRenderer.h"
#include "Logger.h"
#include "Surface.h"
#include "Options.h"
#include "CrossPlatform.h"
#include "FileMap.h"
#include "../lodepng.h"
#include "Screen.h"

namespace OpenXcom
{
// Following code is copy-pasted with minor adjustments from the OpenXcom's OpenGL.cpp file

bool OpenGLRenderer::checkErrors = true;

std::string strGLError(GLenum glErr)
{
	std::string err;

	switch(glErr)
	{
	case GL_INVALID_ENUM:
		err = "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		err = "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		err = "GL_INVALID_OPERATION";
		break;
	case GL_OUT_OF_MEMORY:
		err = "GL_OUT_OF_MEMORY";
		break;
	case GL_NO_ERROR:
		err = "No error! How did you even reach this code?";
		break;
	default:
		err = "Unknown error code!";
		break;
	}

	return err;
}

class Shader
{

};


std::vector<ScalerInfo> OpenGLRenderer::_scalers;

const std::string SHADER_FOLDER = "ShadersModern/";
const std::string SHADER_EXT = "glsl.shader";


const std::string VERTEX_PREAMBLE_ES100 =
        "#version 100\n"
        "\n"
        "#define gl_Vertex pos\n"
        "#define gl_ModelViewProjectionMatrix MVP\n"
        "#define gl_MultiTexCoord0 texCoord0\n"
        "#define gl_TexCoord vTexCoord\n"
        "\n"
        "attribute vec4 pos;\n"
        "attribute vec4 texCoord0;\n"
        "uniform mat4 MVP;\n"
        "varying vec4 vTexCoord[8];\n"
        "\n"
        "vec4 ftransform()\n"
        "{\n"
        "    return MVP * pos;\n"
        "}\n";

const std::string FRAGMENT_PREAMBLE_ES100 =
        "#version 100\n"
        "\n"
        "#define gl_TexCoord vTexCoord\n"
        "#define filter safe_filter\n"
        "\n"
        "precision mediump float;\n"
        "varying vec4 vTexCoord[8];";

const std::string VERTEX_FALLBACK_ES100 =
"varying vec2 vTexCoord0;\n"
"void main()\n"
"{\n"
"  vTexCoord0 = texCoord0;\n"
"  gl_Position = MVP * pos;\n"
"}\n";


const std::string FRAGMENT_FALLBACK_ES100 =
"varying vec2 vTexCoord0;\n"
"uniform sampler2D rubyTexture;\n"
"void main()\n"
"{\n"
"  gl_FragColor = texture2D(rubyTexture, vTexCoord0);\n"
"}\n";

void OpenGLRenderer::_scanScalers()
{
	std::set<std::string> shaders = FileMap::filterFiles(FileMap::getVFolderContents(SHADER_FOLDER), SHADER_EXT);
	for(const auto& sname : shaders)
	{
		ScalerInfo si;
		si.program = -1;
		si.path = SHADER_FOLDER + sname;
		YAML::Node shaderDoc = YAML::LoadFile(FileMap::getFilePath(si.path));
		si.name = shaderDoc["name"].as<std::string>(sname.substr(0, sname.length() - SHADER_EXT.length() - 1));
		std::string shaderVer = shaderDoc["version"].as<std::string>("bad_version");
		if (shaderVer == "essl100")
		{
			_scalers.push_back(si);
		}
	}
}

GLint compileShader(const std::string &shaderSource, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	const char* cShaderSource = shaderSource.c_str();
	glShaderSource(shader, 1, &cShaderSource, nullptr);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE)
	{
		return shader;
	}
	else
	{
		int infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char *infoLog = new char[infoLogLength];
		glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog);
		Log(LOG_ERROR) << "[GLRenderer] Error while compiling shader";
		Log(LOG_ERROR) << "[GLRenderer] Shader source:\n" << shaderSource;
		Log(LOG_ERROR) << "[GLRenderer] Compiler errors:\n" << infoLog;
		delete[] infoLog;
		glDeleteShader(shader);
		return -1;
	}
}

GLint linkProgram(GLuint vertex, GLuint fragment)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_TRUE)
	{
		return program;
	}
	int infoLogLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	char *infoLog = new char[infoLogLength];
	glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog);
	Log(LOG_ERROR) << "[GLRenderer] Error while linking shading program";
	Log(LOG_ERROR) << "[GLRenderer] Linker errors:\n" << infoLog;
	delete[] infoLog;
	glDeleteProgram(program);
	return -1;
}

void OpenGLRenderer::setUpscaler(int upscalerId)
{
	ScalerInfo &si = _scalers[upscalerId];
	if (si.program == -1)
	{
		Log(LOG_DEBUG) << "[GLRenderer] Scaler " << si.name << " not in shader cache, compiling...";
		YAML::Node shaderDoc = YAML::LoadFile(FileMap::getFilePath(si.path));
		std::stringstream vtxSource, frgSource;
		vtxSource << VERTEX_PREAMBLE_ES100
				  << shaderDoc["vertex"].as<std::string>(VERTEX_FALLBACK_ES100);
		frgSource << FRAGMENT_PREAMBLE_ES100
				  << shaderDoc["fragment"].as<std::string>(FRAGMENT_FALLBACK_ES100);
		GLint vertex = compileShader(vtxSource.str(), GL_VERTEX_SHADER);
		if (vertex < 0)
		{
			Log(LOG_ERROR) << "[GLRenderer] Unable to compile vertex shader! Not switching scalers";
			return;
		}
		GLint fragment = compileShader(frgSource.str(), GL_FRAGMENT_SHADER);
		if (fragment < 0)
		{
			Log(LOG_ERROR) << "[GLRenderer] Unable to compile fragment shader! Not switching scalers";
			glDeleteShader(vertex);
			return;
		}
		si.program = linkProgram(vertex, fragment);
		if (si.program < 0)
		{
			Log(LOG_ERROR) << "[GLRenderer] Could not link shader program! Not switching scalers";
			return;
		}
		Log(LOG_DEBUG) << "[GLRenderer] Scaler " << si.name << " now in shader cache with number " << si.program;
	}
	glUseProgram(si.program);
	_shaderData.pos = glGetAttribLocation(si.program, "pos");
	_shaderData.texCoord0 = glGetAttribLocation(si.program, "texCoord0");
	_shaderData.MVP = glGetUniformLocation(si.program, "MVP");
}

std::vector<std::string> OpenGLRenderer::getUpscalers()
{
	std::vector<std::string> result;
	result.reserve(_scalers.size());
	for(const auto& si : _scalers)
	{
		result.push_back(si.name);
	}
	return result;
}

void OpenGLRenderer::setUpscalerByName(const std::string &scalerName)
{
	for(size_t i = 0; i < _scalers.size(); ++i)
	{
		if (_scalers[i].name == scalerName)
		{
			setUpscaler(i);
		}
	}
}

OpenGLRenderer::flip()
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	SDL_GL_SwapWindow(_window);
}

OpenGLRenderer::OpenGLRenderer(Screen &gameScreen) : Renderer(gameScreen, nullptr), _screen(gameScreen), _window(nullptr)
{
	if (_scalers.size() == 0)
	{
		_scanScalers();
	}
}


OpenGLRenderer::OpenGLRenderer(Screen &gameScreen, SDL_Window *window) : Renderer(gameScreen, window), _screen(gameScreen), _window(window)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	if (_scalers.size() == 0)
	{
		_scanScalers();
	}

	_ctx = SDL_GL_CreateContext(_window);

}

OpenGLRenderer::~OpenGLRenderer()
{
	for(auto& si : _scalers)
	{
		si.program = -1;
	}
}


void OpenGLRenderer::resize(unsigned width, unsigned height)
{
    if(gltexture == 0) glGenTextures(1, &gltexture);
	glErrorCheck();
	
	iwidth = width;
	iheight = height;

    glBindTexture(GL_TEXTURE_2D, gltexture);
	glErrorCheck();
    glPixelStorei(GL_UNPACK_ROW_LENGTH, iwidth);
	glErrorCheck();
    glTexImage2D(GL_TEXTURE_2D,
      /* mip-map level = */ 0, /* internal format = */ GL_RGB16_EXT,
      width, height, /* border = */ 0, /* format = */ GL_BGRA,
      iformat, buffer);
	glErrorCheck();

}

bool OpenGLRenderer::lock(uint32_t *&data, unsigned &pitch)
{
    pitch = iwidth * ibpp;
    return (data = buffer);
}

void OpenGLRenderer::clear() {
    //memset(buffer, 0, iwidth * iheight * ibpp);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
	glErrorCheck();
}

 void OpenGLRenderer::set_shader(const char *source_yaml_filename) {
    if(!shader_support) return;

    if(fragmentshader) {
      glDetachShader(glprogram, fragmentshader);
      glDeleteShader(fragmentshader);
      fragmentshader = 0;
    }

    if(vertexshader) {
      glDetachShader(glprogram, vertexshader);
      glDeleteShader(vertexshader);
      vertexshader = 0;
    }

    if(source_yaml_filename && strlen(source_yaml_filename)) {
   	try {
      YAML::Node document = YAML::LoadFile(source_yaml_filename);

      bool is_glsl;
	  std::string language = document["language"].as<std::string>();
	  is_glsl = (language == "GLSL");


      linear = document["linear"].as<bool>(false); // some shaders want texture linear interpolation and some don't
      std::string fragment_source = document["fragment"].as<std::string>("");
	  std::string vertex_source = document["vertex"].as<std::string>("");

      if(is_glsl) {
        if(fragment_source != "") set_fragment_shader(fragment_source.c_str());
        if(vertex_source != "") set_vertex_shader(vertex_source.c_str());
      }
	  }
	  catch (YAML::Exception &e) {
	    Log(LOG_ERROR) << source_yaml_filename << ": " << e.what();
	  }
    }
    glLinkProgram(glprogram);
  }

 void OpenGLRenderer::set_fragment_shader(const char *source) {
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &source, 0);
    glCompileShader(fragmentshader);
    glAttachShader(glprogram, fragmentshader);
  }

 void OpenGLRenderer::set_vertex_shader(const char *source) {
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &source, 0);
    glCompileShader(vertexshader);
    glAttachShader(glprogram, vertexshader);
  }

 void OpenGLRenderer::refresh(bool smooth, unsigned inwidth, unsigned inheight, unsigned outwidth, unsigned outheight) {
    while (glGetError() != GL_NO_ERROR); // clear possible error from who knows where
	clear();
    if(shader_support && (fragmentshader || vertexshader)) {    
      glUseProgram(glprogram);
      GLint location;

      float inputSize[2] = { (float)_srcRect.w, (float)_srcRect.h };
      location = glGetUniformLocation(glprogram, "rubyInputSize");
      glUniform2fv(location, 1, inputSize);

	  float outputSize[2] = { (float)Options::displayWidth, (float)Options::displayHeight };
      location = glGetUniformLocation(glprogram, "rubyOutputSize");
      glUniform2fv(location, 1, outputSize);

      float textureSize[2] = { (float)_srcRect.w, (float)_srcRect.h };
      location = glGetUniformLocation(glprogram, "rubyTextureSize");
      glUniform2fv(location, 1, textureSize);
    }

	glErrorCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);

	glErrorCheck();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Options::displayWidth, 0, Options::displayHeight, -1.0, 1.0);
	glViewport(0, 0, Options::displayWidth, Options::displayHeight);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glErrorCheck();

    glPixelStorei(GL_UNPACK_ROW_LENGTH, bufSurface->pitch / bufSurface->format->BytesPerPixel);

	glErrorCheck();

    glTexSubImage2D(GL_TEXTURE_2D,
      /* mip-map level = */ 0, /* x = */ 0, /* y = */ 0,
      iwidth, iheight, GL_BGRA, iformat, buffer);


    //OpenGL projection sets 0,0 as *bottom-left* of screen.
    //therefore, below vertices flip image to support top-left source.
    //texture range = x1:0.0, y1:0.0, x2:1.0, y2:1.0
    //vertex range = x1:0, y1:0, x2:width, y2:height
    double w = 1.0;
    double h = 1.0;
	int u1 = _dstRect.x;;
    int u2 = _dstRect.w + _dstRect.x;
    int v1 = _dstRect.h + _dstRect.y;
    int v2 = _dstRect.y;;
	
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2i(u1, v1);
		glTexCoord2f(w, 0); glVertex2i(u2, v1);
		glTexCoord2f(w, h); glVertex2i(u2, v2);
		glTexCoord2f(0, h); glVertex2i(u1, v2);
    glEnd();
	glErrorCheck();

    glFlush();
	glErrorCheck();

    if(shader_support) {
      glUseProgram(0);
    }
  }

 void OpenGLRenderer::init(int width, int height)
{
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_DITHER);
	glEnable(GL_TEXTURE_2D);

    //bind shader functions
#ifndef __APPLE__
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)glGetProcAddress("glCreateProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)glGetProcAddress("glUseProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)glGetProcAddress("glCreateShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)glGetProcAddress("glDeleteShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)glGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)glGetProcAddress("glCompileShader");
    glAttachShader = (PFNGLATTACHSHADERPROC)glGetProcAddress("glAttachShader");
    glDetachShader = (PFNGLDETACHSHADERPROC)glGetProcAddress("glDetachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)glGetProcAddress("glLinkProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)glGetProcAddress("glUniform1i");
    glUniform2fv = (PFNGLUNIFORM2FVPROC)glGetProcAddress("glUniform2fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)glGetProcAddress("glUniform4fv");
#endif
	glXGetCurrentDisplay = (void* (APIENTRYP)())glGetProcAddress("glXGetCurrentDisplay");
	glXGetCurrentDrawable = (Uint32 (APIENTRYP)())glGetProcAddress("glXGetCurrentDrawable");
	glXSwapIntervalEXT = (void (APIENTRYP)(void*, Uint32, int))glGetProcAddress("glXSwapIntervalEXT");

	wglSwapIntervalEXT = (Uint32 (APIENTRYP)(int))glGetProcAddress("wglSwapIntervalEXT");



    shader_support = glCreateProgram && glUseProgram && glCreateShader
    && glDeleteShader && glShaderSource && glCompileShader && glAttachShader
    && glDetachShader && glLinkProgram && glGetUniformLocation
    && glUniform1i && glUniform2fv && glUniform4fv;
	
    if(shader_support) glprogram = glCreateProgram();

    //create surface texture
    //resize(width, height);
}

void OpenGLRenderer::term() {
    if(gltexture) {
      glDeleteTextures(1, &gltexture);
      gltexture = 0;
    }

    if(buffer) {
      buffer = 0;
      iwidth = 0;
      iheight = 0;
    }

    //delete buffer_surface;
  }



OpenGLRenderer::OpenGLRenderer(SDL_Window *window): _window(window), gltexture(0), glprogram(0), fragmentshader(0), linear(false), vertexshader(0),
                     buffer(NULL), iwidth(0), iheight(0),
                     iformat(GL_UNSIGNED_INT_8_8_8_8_REV), // this didn't seem to be set anywhere before...
                     ibpp(32),                              // ...nor this
					 _resizeRequested(false)
{
	glContext = SDL_GL_CreateContext(_window);
	init(0, 0);
	setShader(FileMap::getFilePath(Options::useOpenGLShader).c_str());
	setVSync(Options::vSyncForOpenGL);
	checkErrors = Options::checkOpenGLErrors;
}


void OpenGLRenderer::setInternalRect(SDL_Rect *srcRect)
{
	_srcRect.x = srcRect->x;
	_srcRect.y = srcRect->y;
	_srcRect.w = srcRect->w;
	_srcRect.h = srcRect->h;
	// defer actual resizing until a screen refresh
	//resize(_srcRect.w, _srcRect.h);
	_resizeRequested = true;
	// Update shader while we're at it
	setShader(FileMap::getFilePath(Options::useOpenGLShader).c_str());
	setVSync(Options::vSyncForOpenGL);
	checkErrors = Options::checkOpenGLErrors;

}

void OpenGLRenderer::setOutputRect(SDL_Rect *dstRect)
{
	_dstRect.x = dstRect->x;
	_dstRect.y = dstRect->y;
	_dstRect.w = dstRect->w;
	_dstRect.h = dstRect->h;
}

void OpenGLRenderer::flip(SDL_Surface *srcSurface)
{
	bufSurface = srcSurface;
	buffer = (uint32_t*) srcSurface->pixels;
	if (_resizeRequested)
	{
		resize(_srcRect.w, _srcRect.h);
	}
	refresh(linear, _srcRect.w, _srcRect.h, _dstRect.w, _dstRect.h);
	SDL_GL_SwapWindow(_window);
}

void OpenGLRenderer::setShader(std::string shaderPath)
{
	set_shader(shaderPath.c_str());
}

void OpenGLRenderer::setVSync(bool sync)
{
	const int interval = sync ? 1 : 0;
	if (glXGetCurrentDisplay && glXGetCurrentDrawable && glXSwapIntervalEXT)
	{
		void *dpy = glXGetCurrentDisplay();
		Uint32 drawable = glXGetCurrentDrawable();
		if (drawable)
		{
				glXSwapIntervalEXT(dpy, drawable, interval);
				// Log(LOG_INFO) << "Made an attempt to set vsync via GLX.";
		}
	}
	else if (wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(interval);
		// Log(LOG_INFO) << "Made an attempt to set vsync via WGL.";
	}

}

void OpenGLRenderer::screenshot(const std::string &filename) const
{
	int width, height;
	SDL_GetWindowSize(_window, &width, &height);
	unsigned char *pixels = new unsigned char[width * height * 3];
	GLenum format = GL_RGB;
	for (int y = 0; y < height + 2 * _dstRect.y; ++y)
	{
		glReadPixels(0, height - (y + 1), width, 1, format, GL_UNSIGNED_BYTE, (pixels) + y * width * 3);
	}
	glErrorCheck();
	unsigned error = lodepng::encode(filename, (const unsigned char*)pixels, width, height, LCT_RGB);
	if (error)
	{
		Log(LOG_ERROR) << "Saving to PNG failed: " << lodepng_error_text(error);
	}
	delete[] pixels;
	//assert(0 && "Not yet!");
}


}

#endif //__NO_OPENGL
