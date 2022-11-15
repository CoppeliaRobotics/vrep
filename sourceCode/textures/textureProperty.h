#pragma once

#include "textureObject.h"
#include "ser.h"
#include "7Vector.h"

class CSceneObject;

class CTextureProperty
{
public:
    CTextureProperty();
    CTextureProperty(int textureOrVisionSensorObjectID);
    virtual ~CTextureProperty();

    CTextureProperty* copyYourself();
    void serialize(CSer& ar);
    bool announceObjectWillBeErased(const CSceneObject* object);
    void performObjectLoadingMapping(const std::map<int,int>* map);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void addTextureDependencies(int objID,int objSubID);
    void scaleObject(float scalingFactor);
    void setInterpolateColors(bool ic);
    bool getInterpolateColors();
    void setApplyMode(int dt);
    int getApplyMode();
    void adjustForFrameChange(const C7Vector& mCorrection);

    int getTextureObjectID();
    CTextureObject* getTextureObject();
    void transformToFixedTextureCoordinates(const C7Vector& transf,const std::vector<float>& vertices,const std::vector<int>& triangles);
    std::vector<floatFloat>* getTextureCoordinates(int objectStateId,const C7Vector& transf,const std::vector<float>& vertices,const std::vector<int>& triangles);
    std::vector<floatFloat>* getFixedTextureCoordinates();
    C7Vector getTextureRelativeConfig();
    void setTextureRelativeConfig(const C7Vector& c);
    void getTextureScaling(float& x,float& y);
    void setTextureScaling(float x,float y);
    void setRepeatU(bool r);
    bool getRepeatU();
    void setRepeatV(bool r);
    bool getRepeatV();

    void setFixedCoordinates(const std::vector<floatFloat>* coords); // nullptr to remove them and have calculated coords
    bool getFixedCoordinates();

    void setTextureMapMode(int mode);
    int getTextureMapMode();

    int* getTexCoordBufferIdPointer();

    void setStartedTextureObject(CTextureObject* it);
    CTextureObject* getStartedTextureObject();


private:
    void _commonInit();
    CTextureObject* _startedTexture;

    // to copy and serialize:
    bool _interpolateColor;
    int _applyMode;
    bool _repeatU;
    bool _repeatV;
    int _textureOrVisionSensorObjectID;
    int _textureCoordinateMode;
    C7Vector _textureRelativeConfig;
    float _textureScalingX;
    float _textureScalingY;
    int _texCoordBufferId; // used for VBOs
    std::vector<floatFloat> _fixedTextureCoordinates;

    // do not copy nor serialize:
    int _objectStateId;
    std::vector<floatFloat> _calculatedTextureCoordinates;
};
