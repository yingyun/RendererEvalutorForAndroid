/*
*MEIZU Technology. ZHUHAI 2013
*Cui.YY
*The implementation of GLES render.
*Detailed see class header.
*
* TODO: add below operation in the shader
* =>1: Add error check for gl function
* =>2: Texturing    -Done
* =>3: Fragment Shader / Operation
* =>4: FBO/ Advanced Tech
*/

#include "Main_Renderer.h"
//#define LOG_ENABLE
#ifdef LOG_ENABLE
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif
#define error_print(ABC) fprintf(stderr, "ERROR %s +%d:"ABC"\n", __func__, __LINE__) //FixMe; Use standard error output
#define PHY_SCREEN_WIDTH 1080
#define PHY_SCREEN_HIGHT 1800

namespace android
{
/* ---------- Data Definition Area Start ---------- */
GLclampf gl2_basic_render::gColorMatrix[6][4] =
{
    {0.2, 0.6, 0.4, 1.0},       //DeepGreen
    {1.0, 0.0, 0.0, 1.0},	//Red
    {0.0, 1.0, 0.0, 1.0},	//Green
    {1.0, 1.0, 0.0, 1.0},	//Yellow
    {0.0, 0.0, 1.0, 1.0},	//Blue
    {1.0, 1.0, 1.0, 1.0}        //White
};

/* simple triangle vertices.    FixMe; Change to Regular Triangle */
GLfloat gl2_basic_render::gSimpleTriangleVertices[6] =
{-0.5f, -0.288675f,    0.0f, 0.57735f,    0.5f, -0.288675f};

/*
* Cui.YY
* These constructor idea get from Android HWUI design
* Some case will result bad performance of shader as not good-desging
* shader compiler.
*
*   FixMe; Notice while add render command control
*
*   Below source will be add/remvoe in the runtume, so in order to
*   correct frame drawing, add the related logic in three part.
*   1: polygonShaderSetup
*   2: polygonBuildnLink
*   3: poligonDraw
*
*/

//Shader for vertex
const char * gl2_basic_render::gVS_Header_Attribute_vertexPosition =
    "attribute vec4 a_vertexPosition;\n";
const char * gl2_basic_render::gVS_Header_Uniform_rotationMatrix =
    "uniform mat4 u_rotationMatrix;\n";
const char * gl2_basic_render::gVS_Header_Uniform_scaleMatrix =
    "uniform mat4 u_scaleMatrix;\n";
const char * gl2_basic_render::gVS_Header_Uniform_translationMatrix =
    "uniform mat4 u_translationMatrix;\n";
const char * gl2_basic_render::gVS_Header_Attribute_passColor =
    "attribute vec4 a_passColor;\n";
const char *gl2_basic_render::gVS_Header_Attribute_texCoord =
    "attribute vec2 a_texCoord;\n";
const char * gl2_basic_render::gVS_Header_Varying_colorToFrag =
    "varying vec4 v_colorToFrag;\n";
const char * gl2_basic_render::gVS_Header_Varying_texCoordToFrag =
    "varying vec2 v_tcToFrag;\n";

const char * gl2_basic_render::gVS_Main_Start_Function =
    "void main() {\n";

const char * gl2_basic_render::gVS_Function_Direct_Pass_Position =
    "   gl_Position = a_vertexPosition;\n";
const char * gl2_basic_render::gVS_Function_Pass_RO_Multi_Position =
    "   gl_Position = gl_Position * u_rotationMatrix;\n";
const char * gl2_basic_render::gVS_Function_Pass_SC_Multi_Position =
    "   gl_Position = gl_Position * u_scaleMatrix;\n";
const char * gl2_basic_render::gVS_Function_Pass_TR_Multi_Position =
    "   gl_Position = gl_Position * u_translationMatrix;\n";
const char * gl2_basic_render::gVS_Function_Pass_Color_To_Frag =
    "   v_colorToFrag = a_passColor;\n";
const char * gl2_basic_render::gVS_Function_Pass_texCoord_To_Frag =
    "   v_tcToFrag = a_texCoord;\n";

const char * gl2_basic_render::gVS_Main_End_Function =
    "}\n";

//Shader for fragment
const char * gl2_basic_render::gFS_Header_Precision_Mediump_Float =
    "precision mediump float;\n";
const char * gl2_basic_render::gFS_Header_Varying_colorToFrag =
    "varying vec4 v_colorToFrag;\n";
const char * gl2_basic_render::gFS_Header_Varying_texCoordToFrag =
    "varying vec2 v_tcToFrag;\n";
const char * gl2_basic_render::gFS_Header_Sampler2D =
    "uniform sampler2D u_samplerTexture;\n";

const char * gl2_basic_render::gFS_Main_Start_Function =
    "void main() {\n";

const char * gl2_basic_render::gFS_Function_Pass_Constant_Color =
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n";
const char * gl2_basic_render::gFS_Function_Direct_Pass_Color =
    "  gl_FragColor = v_colorToFrag;\n";
const char * gl2_basic_render::gFS_Function_Direct_Sampler_texCoord =
    "  gl_FragColor = texture2D( u_samplerTexture, v_tcToFrag );\n";

const char * gl2_basic_render::gFS_Main_End_Function =
    "}\n";
/* ==========Data Definition Area End ==========*/

gl2_basic_render::gl2_basic_render(unsigned int index, unsigned int step)
    :mAttrVSPosition(0),mIndex(index), mStep(step), mCounter(1), mOGLProgram(0),  mOldTimeStamp(0)
{
    /* model view tranformation */
    hasRotation = true; //Done
    hasScale = false;  //Done
    hasTranslation = false;

    /* advanced vertex operation */
    hasLighting = false;
    hasTexture2D = true;
    hasMipMap = true;
    hasFBO = false;
    hasCubeWithVBO = false;
    hasColorDirectPassCombimeVBO = false;
    hasVAO = false;

    /* fragment shader */
    hasPreciMidium = true;
    hasColorConstantPass = false;
    hasColorDirectPass = false;

    /* polygon setup */
    hasSimTriangle = false;
    hasCube = true;
    hasBlender = false;

    /* transformation specific */
    mRotationAngle = 0;
    mScaleMagnitude = 0;
    mScaleIncreasing = true;
    mTranslationMagnitude = 0;

    /* polygon specific */
    mCubeVertices = 0;
    mCubeColor = 0;
    mCubeIndices = 0;
    mCubeNumOfIndex = 0;
    mBlenderVertices = 0;

    printf("-Config Print-\n \
   \t hasColorConstantPass \t%d\n \
   \t hasColorDirectPass \t%d\n \
   \t hasColorDirectPassCombimeVBO %d\n \
   \t hasPreciMidium \t%d\n  \
   \t hasRotation \t%d\n  \
   \t hasScale \t%d\n    \
   \t hasTranslation %d\n  \
   \t hasLighting \t%d\n   \
   \t hasTexture2D \t%d\n  \
   \t hasMipMap \t%d\n \
   \t hasFBO \t%d\n  \
   \t hasVAO \t%d\n \
   \t hasSimTriangle %d\n \
   \t hasCube\t%d\n \
   \t hasCubeWithVBO\t%d\n \
   \t hasBlender\t%d\n",
           hasColorConstantPass,
           hasColorDirectPass,
           hasColorDirectPassCombimeVBO,
           hasPreciMidium,
           hasRotation,
           hasScale,
           hasTranslation,
           hasLighting,
           hasTexture2D,
           hasMipMap,
           hasFBO,
           hasVAO,
           hasSimTriangle,
           hasCube,
           hasCubeWithVBO,
           hasBlender);
}

GLuint gl2_basic_render::loadShader(GLenum shaderType, const char* pSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader)
        {
            glShaderSource(shader, 1, &pSource, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled)
                {
                    GLint infoLen = 0;
                    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                    if (infoLen)
                        {
                            char* buf = (char*) malloc(infoLen);
                            if (buf)
                                {
                                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                                    fprintf(stderr, "Could not compile shader %d:\n%s\n",
                                            shaderType, buf);
                                    free(buf);
                                }
                            glDeleteShader(shader);
                            shader = 0;
                        }
                }
        }
    return shader;
}

void gl2_basic_render::loadTexture(int* width, int* height, void** pixelData)
{
    /*
    *M.png was the PNG32 format under the /data/ directory.
    *Assume that we just support 8888 format for now and must be an NonOfPower size.
    *SkBitmap::kARGB_8888_Config:
    */
    const char* fileName = "/data/M.png";
    struct stat dest;
    if(stat(fileName, &dest) < 0)
        {
            error_print("Get the texture file size failed!");
            exit(-1);
        }
    unsigned int fileSize =  dest.st_size;
    void * imagePixels = malloc(fileSize);
    if(imagePixels == NULL)
        {
            error_print("imagePixels OOM!");
            exit(-1);
        }
    FILE * file = NULL;
    file = fopen("/data/M.png", "r");
    if (file == NULL) printf("Open texture was faild!\n");
    unsigned int haveReaded = fread(imagePixels, 1, fileSize, file);
    if(fileSize == haveReaded) printf("Load %d bytes texture data was sucessful!", haveReaded);
    fclose(file);
    SkMemoryStream stream(imagePixels, fileSize);
    SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
    codec->setDitherImage(false);
    if(codec)
        {
            codec->decode(&stream, &mBitmap, SkBitmap::kARGB_8888_Config, SkImageDecoder::kDecodePixels_Mode);
            delete codec;
            codec = NULL;
        }
    else
        {
            printf("Create codec was faild!\n");
        }
    *width = mBitmap.width();
    *height = mBitmap.height();
    *pixelData= mBitmap.getPixels();
    free(imagePixels);
    int p2Width = 1 << (31 - __builtin_clz(*width));
    int p2Height = 1 << (31 - __builtin_clz(*height));
    printf("   Texture dimension: width %d, height %d\n", *width, *height);
    if(p2Width != *width || p2Height != *height)
        {
            error_print("Non power of 2 size for Texture data!");
            exit(-1);
        }
    /*
    -Assume that the texture was the power of 2. Throwing error if it's not-
        int p2Width = 1 << (31 - __builtin_clz(width));
        int p2Height = 1 << (31 - __builtin_clz(height));
        if(p2Width < width) p2Width = p2Width << 1;
        if(p2Height < height) p2Height = p2Height << 1;

        printf("Width %d, Height %d\n", width, height);
        printf("P2_Width %d, P2_Height %d\n", p2Width, p2Height);
        */
}

GLuint gl2_basic_render::createProgram(const char* pVertexSource, const char* pFragmentSource)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
        {
            return 0;
        }
    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
        {
            return 0;
        }
    GLuint program = glCreateProgram();
    if (program)
        {
            glAttachShader(program, vertexShader);
            glAttachShader(program, pixelShader);
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE)
                {
                    GLint bufLength = 0;
                    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                    if (bufLength)
                        {
                            char* buf = (char*) malloc(bufLength);
                            if (buf)
                                {
                                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                                    fprintf(stderr, "Could not link program:\n%s\n", buf);
                                    free(buf);
                                }
                        }
                    glDeleteProgram(program);
                    program = 0;
                }
        }
    return program;
}

void gl2_basic_render::polygonShaderSetup()
{
    //Setup Vertext shader header
    mVertexShader.append(gVS_Header_Attribute_vertexPosition);  //We always pass the vertex to the shader
    if(hasRotation) mVertexShader.append(gVS_Header_Uniform_rotationMatrix);
    if(hasScale) mVertexShader.append(gVS_Header_Uniform_scaleMatrix);
    if(hasTranslation) mVertexShader.append(gVS_Header_Uniform_translationMatrix);
    if(hasColorDirectPass) mVertexShader.append(gVS_Header_Attribute_passColor);
    if(hasColorDirectPass) mVertexShader.append(gVS_Header_Varying_colorToFrag);
    if(hasTexture2D) mVertexShader.append(gVS_Header_Attribute_texCoord);
    if(hasTexture2D) mVertexShader.append(gVS_Header_Varying_texCoordToFrag);

    //Setup Vertex shader main body
    mVertexShader.append(gVS_Main_Start_Function);

    mVertexShader.append(gVS_Function_Direct_Pass_Position);
    if(hasRotation) mVertexShader.append(gVS_Function_Pass_RO_Multi_Position);
    if(hasScale) mVertexShader.append(gVS_Function_Pass_SC_Multi_Position);
    if(hasTranslation) mVertexShader.append(gVS_Function_Pass_TR_Multi_Position);
    if(hasColorDirectPass) mVertexShader.append(gVS_Function_Pass_Color_To_Frag);
    if(hasTexture2D) mVertexShader.append(gVS_Function_Pass_texCoord_To_Frag);

    mVertexShader.append(gVS_Main_End_Function);
    printf("\nVertextShader=> \n");
    printf("%s\n", mVertexShader.string());

    //Setup Fragment shader header
    if(hasPreciMidium)mFramgmentShader.append(gFS_Header_Precision_Mediump_Float);
    if(hasColorDirectPass)mFramgmentShader.append(gFS_Header_Varying_colorToFrag);
    if(hasTexture2D) mFramgmentShader.append(gFS_Header_Varying_texCoordToFrag);
    if(hasTexture2D) mFramgmentShader.append(gFS_Header_Sampler2D);

    //Setup Fragment shader main body
    mFramgmentShader.append(gFS_Main_Start_Function);

    if(hasColorConstantPass)mFramgmentShader.append(gFS_Function_Pass_Constant_Color);
    if(hasColorDirectPass)mFramgmentShader.append(gFS_Function_Direct_Pass_Color);
    if(hasTexture2D) mFramgmentShader.append(gFS_Function_Direct_Sampler_texCoord);

    mFramgmentShader.append(gFS_Main_End_Function);
    printf("FragmentShader=> \n");
    printf("%s\n", mFramgmentShader.string());

    /* Do polygon vertex generation, Index mode by default
    * TODO: add cube index mode and non index mode switch
    */
    //generateCube(bool indexMode, float scale, float * * vertices, float * * normals, float * * texCoords, float * * colors, unsigned int * * indices)
    if(hasCube || hasCubeWithVBO)
        mCubeNumOfIndex = VertexGenerator::generateCube(true, 0.5, &mCubeVertices,
                          NULL, &mCubeTexCoord, &mCubeColor, &mCubeIndices);
}

bool gl2_basic_render::polygonBuildnLink(int w, int h, const char vertexShader[], const char fragmentShader[])
{
    mOGLProgram= createProgram(vertexShader, fragmentShader);
    if (!mOGLProgram)
        {
            return false;
        }
    //Retrieve shader paramer information
    mAttrVSPosition = glGetAttribLocation(mOGLProgram, "a_vertexPosition");
    if(hasRotation)
        mUniVSrotateMat = glGetUniformLocation(mOGLProgram, "u_rotationMatrix");
    if(hasScale)
        mUniVSscaleMat = glGetUniformLocation(mOGLProgram, "u_scaleMatrix");
    if(hasTranslation)
        mUniVStranslateMat = glGetUniformLocation(mOGLProgram, "u_translationMatrix");
    if(hasColorDirectPass)
        mAttrVSColorPass = glGetAttribLocation(mOGLProgram, "a_passColor");
    //Create various buffer object
    if(hasCubeWithVBO)
        {
            glGenBuffers(3, mVBOForVI); //Note: This is just used for Index mode rendering
            //[0] for Vertex
            glBindBuffer(GL_ARRAY_BUFFER, mVBOForVI[0]);
            glBufferData(GL_ARRAY_BUFFER, VertexGenerator::vertexSizeByte(true),
                         mCubeVertices, GL_STATIC_DRAW);

            //[1] for color
            glBindBuffer(GL_ARRAY_BUFFER, mVBOForVI[1]);
            glBufferData(GL_ARRAY_BUFFER, VertexGenerator::colorSizeByte(true),
                         mCubeColor, GL_STATIC_DRAW);

            //[2] for index
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOForVI[2]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 500/*VertexGenerator::indexSizeByte()*/,
                         mCubeIndices, GL_STATIC_DRAW);
        }
    if(hasTexture2D)
        {
            int width = 0, height =0;
            void* pixelData;
            glGenTextures(1, mTexture);
            glBindTexture(GL_TEXTURE_2D, mTexture[0]);
            loadTexture( &width, &height, &pixelData);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if(hasMipMap)
                {
                    glGenerateMipmap(GL_TEXTURE_2D);
                }
            mAttrVSTexCoordPass = glGetAttribLocation(mOGLProgram, "a_texCoord");  /*Texture coordination for VS */
            mUniFSSampler = glGetUniformLocation(mOGLProgram, "u_samplerTexture"); /*Sampler unit for FS */
            glActiveTexture(GL_TEXTURE0);                                   /* Enable Texutre sampler unit */
            glBindTexture(GL_TEXTURE_2D, mTexture[0]);
            glUniform1i(mUniFSSampler, 0);
        }

    /*
    *Cui. YY 20131008:W=window width, H=window height by default
    *As the NDC->WC formular, if w, h doesn't match as the same value in here,
    *It will be generate the stretch gemotry
    *
    *Ox = (x + w)/2
    *Oy = (y + h)/2
    *
    *Xw = (w/2)Xd + Ox
    *Yw = (h/2)Yd + Oy
    *Zw = ((f-n)/2)Zd + (n + f)/2
    *
    *In order to keep geometry on the centre, keep the view post dimension as the same.
    *Note that the origin in view post was that left-botton based on each specific EGL surface.
    */
    glViewport(0, (h-w)/2, w, w);
    //glViewport(0, 0, 120, 120);   // Just for fun
    /*
    *N=0.0, F=1.0 by default
    *If set this as (0, 0), as the formulal there is no Z value will be considered while rasterization.
    */
    glDepthRangef(0, 1);
    glClearColor(gColorMatrix[mIndex][0], gColorMatrix[mIndex][1],
                 gColorMatrix[mIndex][2], gColorMatrix[mIndex][3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //Culling should always be enabled to improve the perf of OpenGL, Culling back and CCW was defualt config
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glUseProgram(mOGLProgram);
    return true;
}

void gl2_basic_render::polygonDraw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    /* polygon vertex data */
    if(hasSimTriangle)glVertexAttribPointer(mAttrVSPosition, 2, GL_FLOAT, GL_FALSE, 0, gSimpleTriangleVertices);
    if(hasCubeWithVBO)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVBOForVI[0]);
            glVertexAttribPointer(mAttrVSPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(mAttrVSPosition);
        }
    if(hasCube)
        {
            glVertexAttribPointer(mAttrVSPosition, 3, GL_FLOAT, GL_FALSE, 0, mCubeVertices);
            glEnableVertexAttribArray(mAttrVSPosition);
        }
    /* do transformantion */
    if(hasRotation)
        {
            MatrixTransform::matrixIndentity(&mRotateMatrix);
            /*
            * Cui.YY 20130916
            * Why (++mRotationAngle) % 360 didn't pass the compiler as it's called undefined behaviour.
            * The compiler can do re-order feel free, so, cannot ensure the same result in different compiler
            * even with same compiler with different settings.
            * So, in C/C++ we can just modify one value in single line, can't modify same value in both
            * operands.  http://stackoverflow.com/questions/9951993/why-foo-foo-may-be-undefined
            */
            ++mRotationAngle;
            mRotationAngle = mRotationAngle % 360;//The step of angle increasing
            MatrixTransform::matrixRotate(&mRotateMatrix, (GLfloat)mRotationAngle, 1.0, 1.0, 0.0);
            MatrixTransform::matrixDump(&mRotateMatrix, "mRotateMatrix");
            glUniformMatrix4fv(mUniVSrotateMat, 1, GL_FALSE, (GLfloat * )mRotateMatrix.m);
        }
    if(hasScale)
        {
            /* do repeast operation for the change of scale magnitude */
            if (mScaleIncreasing)
                {

                    mScaleMagnitude += 0.01;
                    if (mScaleMagnitude > (GLfloat)1.5)
                        {
                            mScaleIncreasing = false;
                        }
                }
            else
                {
                    mScaleMagnitude -= 0.01;
                    if (mScaleMagnitude < (GLfloat)0.0)
                        {
                            mScaleIncreasing = 1;
                        }
                }
            MatrixTransform::matrixIndentity(&mScaleMatrix);
            MatrixTransform::matrixScale(&mScaleMatrix, mScaleMagnitude, mScaleMagnitude, mScaleMagnitude);
            MatrixTransform::matrixDump(&mScaleMatrix, "mScaleMatrix");
            glUniformMatrix4fv(mUniVSscaleMat, 1, GL_FALSE, (GLfloat * )mScaleMatrix.m);
        }
    if(hasTranslation)
        {
            mTranslationMagnitude += 0.1;
            if (mTranslationMagnitude > 1.0f) mTranslationMagnitude = 0;
            MatrixTransform::matrixIndentity(&mTranslateMatrix);
            /* FixMe; matrixTranslate was wrong ?  */
            MatrixTransform::matrixTranslate(&mTranslateMatrix, 0.0f, 0.0f, 0.0f);
            MatrixTransform::matrixDump(&mTranslateMatrix, "mTranslateMatrix");
            glUniformMatrix4fv(mUniVStranslateMat, 1, GL_FALSE, (GLfloat * )mTranslateMatrix.m);
        }
    /* Color and Light */

    /*
    *hasColorDirectPass can use as standalone, but if want to use VBO version, should alwasy mark has-
    ColorDirectPassCombineVBO as true. Not allow only hasColorDirectPassCombineVBO condition as true.
    */
    if(hasColorDirectPassCombimeVBO && !hasColorDirectPass)
        {
            error_print("Wrong configucation, Not allow!");
            exit(-1);
        }
    if(hasColorDirectPassCombimeVBO && hasColorDirectPass)  //Here, we depend on DirectPass shader ability
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVBOForVI[1]);
            glVertexAttribPointer(mAttrVSColorPass, 4, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(mAttrVSColorPass);

        }
    if (!hasColorDirectPassCombimeVBO && hasColorDirectPass)
        {
            glVertexAttribPointer(mAttrVSColorPass, 4, GL_FLOAT, GL_FALSE, 0, mCubeColor);
            glEnableVertexAttribArray(mAttrVSColorPass);
        }
    if(hasTexture2D)
        {
            glVertexAttribPointer(mAttrVSTexCoordPass, 2, GL_FLOAT, GL_FALSE, 0, mCubeTexCoord); /* Enable Texture coordination */
            glEnableVertexAttribArray(mAttrVSTexCoordPass);
        }
    /* Let's cook */
    if(hasSimTriangle)glDrawArrays(GL_TRIANGLES, 0, 3);
    if(hasCubeWithVBO)
        glDrawElements(GL_TRIANGLES, mCubeNumOfIndex, GL_UNSIGNED_INT, 0);
    if(hasCube)
        glDrawElements(GL_TRIANGLES, mCubeNumOfIndex, GL_UNSIGNED_INT, mCubeIndices);
    /*Swap something*/
    eglSwapBuffers(mEGLDisplay, mEGLSurface);
}
/*if(hasCube)glDrawArrays(GL_TRIANGLES, 0, 36);  //No index mode*/

/*
*   Note: This is Static member function
*/
void gl2_basic_render::frameControl(int fd, int events, void* data)
{
    gl2_basic_render* thisObject = (gl2_basic_render*)data;
    ssize_t num_event;
    DisplayEventReceiver::Event buffer[1];
    num_event = thisObject->mDisplayEventReceiver.getEvents(buffer, 1);

    if(thisObject->mCounter == thisObject->mStep)
        {
            if(num_event > 0)
                {
                    if(buffer[0].header.type == DisplayEventReceiver::DISPLAY_EVENT_VSYNC)
                        {
                            // Real rendering block
                            thisObject->polygonDraw();

                            LOG("TID:%d swap buffers--", gettid());
                        }
                }
            thisObject->mCounter = 1;
        }
    else
        {
            LOG("TID:%d skip buffers--", gettid());
            thisObject->mCounter++;
        }

    if (thisObject->mOldTimeStamp)
        {
            float t = float(buffer[0].header.timestamp - thisObject->mOldTimeStamp) / s2ns(1);
            LOG("TID:%d %f ms (%f Hz)--", gettid(), t*1000, 1.0/t);
        }
    thisObject->mOldTimeStamp = buffer[0].header.timestamp;
    LOG("TID:%d Event vsync:count=%d\n",gettid(), buffer[0].vsync.count);
}

/*
*   Note: This is Static member function
*/
void* gl2_basic_render::mainRender(void* thisthis)
{
    gl2_basic_render * thisObject = (gl2_basic_render*)thisthis;
    printf("TID:%d Create render\n", gettid());
    EGLBoolean returnValue;
    EGLConfig myConfig = {0};
    EGLint numConfigs;
    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLint s_configAttribs[] =
    {
        EGL_SURFACE_TYPE,        EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLint majorVersion;
    EGLint minorVersion;
    EGLContext context;
    EGLint w, h;
    thisObject->mEGLDisplay= eglGetDisplay(EGL_DEFAULT_DISPLAY);
    returnValue = eglInitialize(thisObject->mEGLDisplay, &majorVersion, &minorVersion);
    eglChooseConfig(thisObject->mEGLDisplay, s_configAttribs, &myConfig, 1, &numConfigs);
    thisObject-> mEGLSurface= eglCreateWindowSurface(thisObject->mEGLDisplay, myConfig, thisObject->windowSurface, NULL);
    context = eglCreateContext(thisObject->mEGLDisplay, myConfig, EGL_NO_CONTEXT, context_attribs);
    returnValue = eglMakeCurrent(thisObject->mEGLDisplay, thisObject->mEGLSurface, thisObject->mEGLSurface, context);
    eglQuerySurface(thisObject->mEGLDisplay, thisObject->mEGLSurface, EGL_WIDTH, &w);
    eglQuerySurface(thisObject->mEGLDisplay, thisObject->mEGLSurface, EGL_HEIGHT, &h);
    printf("TID:%d Window dimensions: %d x %d\n", gettid(), w, h);

    /*
    *   1: Generating polygon shader program
    *   2: Build shader and link it
    *   3: Pass control over to Looper to draw each frame
    */
    thisObject->polygonShaderSetup();
    if(!thisObject->polygonBuildnLink(w, h,
                                      thisObject->mVertexShader.string(), thisObject->mFramgmentShader.string()))
        {
            fprintf(stderr, "Could not set up graphics.\n");
            return (void *)0;
        }
    thisObject->mLoop = new Looper(false);
    thisObject->mLoop->addFd(thisObject->mDisplayEventReceiver.getFd(), 0, ALOOPER_EVENT_INPUT,
                             (ALooper_callbackFunc)gl2_basic_render::frameControl, thisObject);
    thisObject->mDisplayEventReceiver.setVsyncRate(1);//Enable vsync forever
    unsigned int tid = gettid();
    do
        {
            int ret = thisObject->mLoop->pollOnce(-1);
            switch(ret)
                {
                    case ALOOPER_POLL_WAKE:
                        printf("TID:%d Poll wake\n", tid);
                        break;
                    case ALOOPER_POLL_CALLBACK:
                        /* printf("TID:%d Poll callback\n", tid); */
                        break;
                    case ALOOPER_POLL_TIMEOUT:
                        printf("TID:%d Poll timeout\n", tid);
                        break;
                    case ALOOPER_POLL_ERROR:
                        printf("TID:%d Poll error\n", tid);
                        break;
                    default:
                        printf("TID:%d What???\n", tid);
                        break;
                }
        }
    while(true);
    return 0;
}

void gl2_basic_render::startRender(EGLNativeWindowType window)
{
    pthread_t thread_status;
    windowSurface = window;
    pthread_create(&thread_status, NULL, gl2_basic_render::mainRender, (void*)(this));
}

}
