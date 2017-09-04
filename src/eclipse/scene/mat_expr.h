#pragma once

#include <string>
#include <vector>
#include <memory>

#include "eclipse/math/vec3.h"

namespace eclipse { namespace material {

class NExpression
{
public:
    virtual ~NExpression() { }
    virtual std::string validate() const { return ""; }
};

std::unique_ptr<NExpression> parse_expr(const std::string& expr);

namespace bxdf {

    enum Type
    {
        INVALID = 0,
        DIFFUSE,
        CONDUCTOR,
        ROUGH_CONDUCTOR,
        DIELECTRIC,
        ROUGH_DIELECTRIC,
        EMISSIVE
    };

    std::string to_string(bxdf::Type bxdf);

    namespace param {

        enum Type
        {
            INVALID = 0,
            REFLECTANCE,
            SPECULARITY,
            TRANSMITTANCE,
            RADIANCE,
            INT_IOR,
            EXT_IOR,
            SCALE,
            ROUGHNESS
        };

        std::string to_string(bxdf::param::Type param);

        enum ValueType
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

        struct Value
        {
            ValueType type;
            Vec3 value;
            std::string name;

            std::string validate() const;

            static Value num(float v);
            static Value vec3(float x, float y, float z);
            static Value texture(std::string name);
            static Value known_ior(std::string name);
        };

    } // namespace param

} // namespace bxdf

struct NBxdfParam
{
    bxdf::param::Type type;
    bxdf::param::Value value;

    std::string validate() const;
};

typedef std::vector<NBxdfParam> NBxdfParamList;

class NOperation : public NExpression
{
};

class NBxdf : public NExpression
{
public:
    bxdf::Type type;
    NBxdfParamList parameters;

    explicit NBxdf(bxdf::Type type, NBxdfParamList params = { });
    std::string validate() const override;
};

class NMatRef : public NExpression
{
public:
    std::string name;

    explicit NMatRef(const std::string& mat);
    std::string validate() const override;
};

class NMix : public NOperation
{
public:
    std::unique_ptr<NExpression> expressions[2];
    float weight;

    explicit NMix(NExpression* left, NExpression* right, float w);
    std::string validate() const override;
};

class NMixMap : public NOperation
{
public:
    std::unique_ptr<NExpression> expressions[2];
    std::string texture;

    explicit NMixMap(NExpression* left, NExpression* right, bxdf::param::Value tex);
    std::string validate() const override;
};

class NBumpMap : public NOperation
{
public:
    std::unique_ptr<NExpression> expression;
    std::string texture;

    explicit NBumpMap(NExpression* expr, bxdf::param::Value tex);
    std::string validate() const override;
};

class NNormalMap : public NOperation
{
public:
    std::unique_ptr<NExpression> expression;
    std::string texture;

    explicit NNormalMap(NExpression* expr, bxdf::param::Value tex);
    std::string validate() const override;
};

class NDisperse : public NOperation
{
public:
    std::unique_ptr<NExpression> expression;
    Vec3 int_ior;
    Vec3 ext_ior;

    explicit NDisperse(NExpression* expr, bxdf::param::Value iior, bxdf::param::Value eior);
    std::string validate() const override;
};

} } // namespace eclipse::material
