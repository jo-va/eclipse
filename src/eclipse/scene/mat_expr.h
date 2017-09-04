#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "eclipse/math/vec3.h"

namespace eclipse { namespace material {

namespace param {

constexpr char Reflectance[]   = "reflectance";
constexpr char Specularity[]   = "specularity";
constexpr char Transmittance[] = "transmittance";
constexpr char Radiance[]      = "radiance";
constexpr char IntIOR[]        = "intIOR";
constexpr char ExtIOR[]        = "extIOR";
constexpr char Scale[]         = "scale";
constexpr char Roughness[]     = "roughness";

} // namespace Param

class NExpression
{
public:
    virtual ~NExpression() { }
};

std::unique_ptr<NExpression> parse_material_expr(const std::string& expr);

enum BxdfType
{
    DIFFUSE = 0,
    CONDUCTOR,
    ROUGH_CONDUCTOR,
    DIELECTRIC,
    ROUGH_DIELECTRIC,
    EMISSIVE
};

std::string bxdf_to_string(BxdfType bxdf);

enum BxdfParamType
{
    REFLECTANCE = 0,
    SPECULARITY,
    TRANSMITTANCE,
    RADIANCE,
    INT_IOR,
    EXT_IOR,
    SCALE,
    ROUGHNESS
};

struct NValue
{
    enum ValueType { NUM = 1, VECTOR, TEXTURE, KNOWN_IOR };

    ValueType type;
    Vec3 value;
    std::string name;

    static NValue num(float v)                    { return { NUM,       Vec3(v, 0, 0), ""              }; }
    static NValue vec3(float x, float y, float z) { return { VECTOR,    Vec3(x, y, z), ""              }; }
    static NValue texture(std::string name)       { return { TEXTURE,   Vec3(0, 0, 0), std::move(name) }; }
    static NValue known_ior(std::string name)     { return { KNOWN_IOR, Vec3(0, 0, 0), std::move(name) }; }
};

struct NBxdfParam
{
    BxdfParamType type;
    NValue value;
};

class NOperation : public NExpression
{
};

typedef std::vector<NBxdfParam> NBxdfParamList;

class NBxdf : public NExpression
{
public:
    BxdfType type;
    NBxdfParamList parameters;

    explicit NBxdf(BxdfType type, NBxdfParamList params = { })
        : type(type), parameters(std::move(params)) { }
};

class NMatRef : public NExpression
{
public:
    std::string name;

    explicit NMatRef(const std::string& mat) : name(mat) { }
};

class NMix : public NOperation
{
public:
    std::unique_ptr<NExpression> expressions[2];
    float weight;

    explicit NMix(NExpression* left, NExpression* right, float w)
        : expressions{ std::unique_ptr<NExpression>(left), std::unique_ptr<NExpression>(right) }, weight(w) { }
};

class NMixMap : public NOperation
{
public:
    std::unique_ptr<NExpression> expressions[2];
    std::string texture;

    explicit NMixMap(NExpression* left, NExpression* right, NValue tex)
        : expressions{ std::unique_ptr<NExpression>(left), std::unique_ptr<NExpression>(right) }, texture(tex.name) { }
};

class NBumpMap : public NOperation
{
public:
    std::unique_ptr<NExpression> expression;
    std::string texture;

    explicit NBumpMap(NExpression* expr, NValue tex)
        : expression(expr), texture(tex.name) { }
};

class NNormalMap : public NOperation
{
public:
    std::unique_ptr<NExpression> expression;
    std::string texture;

    explicit NNormalMap(NExpression* expr, NValue tex)
        : expression(expr), texture(tex.name) { }
};

class NDisperse : public NOperation
{
public:
    std::unique_ptr<NExpression> expression;
    Vec3 int_ior;
    Vec3 ext_ior;

    explicit NDisperse(NExpression* expr, NValue iior, NValue eior)
        : expression(expr), int_ior(iior.value), ext_ior(eior.value) { }
};

} } // namespace eclipse::material
