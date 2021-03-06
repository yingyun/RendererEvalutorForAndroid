/*
*RenderEvaluator main function, it's an entrance of program.
*
*Copyright (C) 2013 - 2014 Cui. Yingyun
*This file is released under the GPL2
*/
#include "RenderEvaluator.h"

int main(int argc, char** argv)
{
    usageRender();
    INIReader configReader(CONFIG_INI_PATH);
    if(configReader.ParseError() < 0) LOG_ERROR("Parsing config.ini failed!\n");

    MainFunctionType functionInfo = parseMainFunction(configReader);
    if(functionInfo.LayerRender != 0)
        {
            handleLayerRenderTask(configReader, functionInfo.LayerRender);
        }
    else if(functionInfo.AndroidFLATLAND != false)
        {
            handleFlatlandTask(configReader);
        }
    else if(functionInfo.GoogleTest != false)
        {
            handleGTest(configReader);
        }
    else
        {
            LOG_ERROR("You should config one main function at least\n");
        }
    return true;
}

void usageRender()
{
    /*
    *No meaninful code below, just only for showing RenderEvaluator can use
    *STL and Boost two powerful C++ library.
    */
    string description;
    description += "Cui.YY RenderEvaluator Ver:0.2\n\
Note: Be sure you have config.ini and texture image files under /data/RendererEvaluator/\n\n";
    std::cout << description.data() << std::endl;
}

void handleLayerRenderTask(INIReader& configParser, const unsigned int numOfLayer)
{
    sp<Native_Android> nativeSurface = new Native_Android();
    if(!nativeSurface->startService()) LOG_ERROR("Establish connection with SF was failed!\n");
    size_t layers = numOfLayer;

    vector< sp<RenderMachine> > renders;
    CLAMP(layers);

    for(unsigned int index = 0; index < layers; ++index)
        {
            /* Parse specific layer info */
            string layerName("Layer");
            char numBuf[5];
            snprintf(numBuf, 5, "%d", index);
            layerName += string(numBuf);
            LayerRenderType layerInfo = parseLayer(configParser, layerName);

            /* Create render machine and assgin canvas to it */
            renders.push_back(new RenderMachine(layerInfo));
            nativeSurface->setLayerRender(layerInfo);
            renders.back()->startRender(nativeSurface->createNativeSurface().get());
            sleep(1);
        }
    nativeSurface->threadPool();
    nativeSurface->stopService();
}

void handleFlatlandTask(INIReader& configParser)
{
    /* Do something */
}

void handleGTest(INIReader& configParser)
{
    /* Do something */
}

LayerRenderType parseLayer(INIReader & configParser, const string& layerName)
{
    LayerRenderType layerInfo;
    layerInfo.LayerRenderEffect = configParser.GetString(layerName, "LayerRenderEffect", "default");
    layerInfo.LayerDrawTick = configParser.GetInteger(layerName, "LayerDrawTick", 0);
    layerInfo.LayerOrder = configParser.GetInteger(layerName, "LayerOrder", 0);
    layerInfo.LayerPositionX = configParser.GetInteger(layerName, "LayerPositionX", 0);
    layerInfo.LayerPositionY = configParser.GetInteger(layerName, "LayerPositionY", 0);
    layerInfo.LayerWidth = configParser.GetInteger(layerName, "LayerWidth", 0);
    layerInfo.LayerHeight = configParser.GetInteger(layerName, "LayerHeight", 0);
    layerInfo.LayerAlpha = configParser.GetReal(layerName, "LayerAlpha",  1.0f);
    layerInfo.LayerObjectModel = configParser.GetString(layerName, "LayerObjectModel", "default");
    layerInfo.LayerTexture = configParser.GetString(layerName, "LayerTexture", "default");
    string index = layerName.substr(5);
    layerInfo.LayerIndex = atoi(index.data());
    return layerInfo;
}

MainFunctionType parseMainFunction(INIReader & configParser)
{
    MainFunctionType functionInfo;
    functionInfo.LayerRender = configParser.GetInteger("MainFunction", "LayerRender", 0);
    functionInfo.AndroidFLATLAND = configParser.GetBoolean("MainFunction", "AndroidFLATLAND", false);
    functionInfo.GoogleTest = configParser.GetBoolean("MainFunction", "GoogleTest", false);
    return functionInfo;
}

