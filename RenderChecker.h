#ifndef RENDERCHECKER_H_
#define RENDERCHECKER_H_

#include <EGL/egl.h>

#ifdef USE_OPENGL_ES_20
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif 

#ifdef USE_OPENGL_ES_30
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

#include "Pattern/Singleton.h"
#include "Logging.h"

namespace RenderEvaluator
{
class RenderChecker : public Singleton<RenderChecker>
{
public:
    RenderChecker() {}
    void checkGLDriver(EGLDisplay display);
    void checkEGLConfig(EGLConfig config, EGLDisplay display);
    void checkGLErrors();
};
}
#endif
