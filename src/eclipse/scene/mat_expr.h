#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "eclipse/math/vec3.h"
#include "eclipse/scene/material_except.h"

namespace eclipse { namespace material {

enum NodeType
{
    NODE_NONE,
    BXDF_DIFFUSE,
    BXDF_CONDUCTOR,
    BXDF_ROUGH_CONDUCTOR,
    BXDF_DIELECTRIC,
    BXDF_ROUGH_DIELECTRIC,
    BXDF_EMISSIVE,
    OP_MIX,
    OP_MIXMAP,
    OP_BUMPMAP,
    OP_NORMALMAP,
    OP_DISPERSE
};

enum ParamType
{
    PARAM_NONE,
    REFLECTANCE,
    SPECULARITY,
    TRANSMITTANCE,
    RADIANCE,
    INT_IOR,
    EXT_IOR,
    SCALER,
    ROUGHNESS,
    WEIGHT
};

class ExprNode
{
public:
    virtual ~ExprNode() { }
    virtual void validate() const { }
};

typedef std::shared_ptr<ExprNode> ExprNodePtr;

ExprNodePtr parse_expr(const std::string& expr);

std::string node_to_string(NodeType bxdf);
std::string param_to_string(ParamType param);

bool is_bxdf_type(NodeType type);

enum ParamValueType
{
    NUM = 0,
    VECTOR,
    TEXTURE,
    KNOWN_IOR
};

constexpr char Reflectance[]   = "reflectance";
constexpr char Specularity[]   = "specularity";
constexpr char Transmittance[] = "transmittance";
constexpr char Radiance[]      = "radiance";
constexpr char IntIOR[]        = "intIOR";
constexpr char ExtIOR[]        = "extIOR";
constexpr char Scale[]         = "scale";
constexpr char Roughness[]     = "roughness";

struct ParamValue
{
    ParamValueType type;
    Vec3 vec;
    std::string name;

    void validate() const;

    static ParamValue num(float v);
    static ParamValue vec3(float x, float y, float z);
    static ParamValue texture(std::string name);
    static ParamValue known_ior(std::string name);
};

struct NBxdfParam
{
    ParamType type;
    ParamValue value;

    void validate() const;
};

typedef std::vector<NBxdfParam> NBxdfParamList;

class OpNode : public ExprNode
{
};

class NBxdf : public ExprNode
{
public:
    NodeType type;
    NBxdfParamList parameters;

    explicit NBxdf(NodeType type, NBxdfParamList params = { });
    void validate() const override;
};

class NMatRef : public ExprNode
{
public:
    std::string name;

    explicit NMatRef(const std::string& mat);
    void validate() const override;
};

class NMix : public OpNode
{
public:
    ExprNodePtr expressions[2];
    float weight;

    explicit NMix(ExprNode* left, ExprNode* right, float w);
    void validate() const override;
};

class NMixMap : public OpNode
{
public:
    ExprNodePtr expressions[2];
    std::string texture;

    explicit NMixMap(ExprNode* left, ExprNode* right, ParamValue tex);
    void validate() const override;
};

class NBumpMap : public OpNode
{
public:
    ExprNodePtr expression;
    std::string texture;

    explicit NBumpMap(ExprNode* expr, ParamValue tex);
    void validate() const override;
};

class NNormalMap : public OpNode
{
public:
    ExprNodePtr expression;
    std::string texture;

    explicit NNormalMap(ExprNode* expr, ParamValue tex);
    void validate() const override;
};

class NDisperse : public OpNode
{
public:
    ExprNodePtr expression;
    Vec3 int_ior;
    Vec3 ext_ior;

    explicit NDisperse(ExprNode* expr, ParamValue iior, ParamValue eior);
    void validate() const override;
};

} } // namespace eclipse::material
