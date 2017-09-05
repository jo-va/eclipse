#pragma once

#include <string>
#include <vector>
#include <memory>

#include "eclipse/math/vec3.h"

namespace eclipse { namespace material {

class ExprNode
{
public:
    virtual ~ExprNode() { }
    virtual std::string validate() const { return ""; }
};

std::unique_ptr<ExprNode> parse_expr(const std::string& expr, std::string& error);

typedef std::shared_ptr<ExprNode> ExprNodePtr;

enum BxdfType
{
    INVALID_BXDF = 0,
    DIFFUSE,
    CONDUCTOR,
    ROUGH_CONDUCTOR,
    DIELECTRIC,
    ROUGH_DIELECTRIC,
    EMISSIVE
};

std::string bxdf_to_string(BxdfType bxdf);

enum ParamType
{
    INVALID_PARAM = 0,
    REFLECTANCE,
    SPECULARITY,
    TRANSMITTANCE,
    RADIANCE,
    INT_IOR,
    EXT_IOR,
    SCALER,
    ROUGHNESS
};

std::string param_to_string(ParamType param);

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

    std::string validate() const;

    static ParamValue num(float v);
    static ParamValue vec3(float x, float y, float z);
    static ParamValue texture(std::string name);
    static ParamValue known_ior(std::string name);
};

struct NBxdfParam
{
    ParamType type;
    ParamValue value;

    std::string validate() const;
};

typedef std::vector<NBxdfParam> NBxdfParamList;

class OpNode : public ExprNode
{
};

class NBxdf : public ExprNode
{
public:
    BxdfType type;
    NBxdfParamList parameters;

    explicit NBxdf(BxdfType type, NBxdfParamList params = { });
    std::string validate() const override;
};

class NMatRef : public ExprNode
{
public:
    std::string name;

    explicit NMatRef(const std::string& mat);
    std::string validate() const override;
};

class NMix : public OpNode
{
public:
    std::unique_ptr<ExprNode> expressions[2];
    float weight;

    explicit NMix(ExprNode* left, ExprNode* right, float w);
    std::string validate() const override;
};

class NMixMap : public OpNode
{
public:
    std::unique_ptr<ExprNode> expressions[2];
    std::string texture;

    explicit NMixMap(ExprNode* left, ExprNode* right, ParamValue tex);
    std::string validate() const override;
};

class NBumpMap : public OpNode
{
public:
    std::unique_ptr<ExprNode> expression;
    std::string texture;

    explicit NBumpMap(ExprNode* expr, ParamValue tex);
    std::string validate() const override;
};

class NNormalMap : public OpNode
{
public:
    std::unique_ptr<ExprNode> expression;
    std::string texture;

    explicit NNormalMap(ExprNode* expr, ParamValue tex);
    std::string validate() const override;
};

class NDisperse : public OpNode
{
public:
    std::unique_ptr<ExprNode> expression;
    Vec3 int_ior;
    Vec3 ext_ior;

    explicit NDisperse(ExprNode* expr, ParamValue iior, ParamValue eior);
    std::string validate() const override;
};

} } // namespace eclipse::material
