#include <memory>
#include <utility>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>

#include "eclipse/scene/mat_expr.h"
#include "eclipse/scene/mat_expr_scanner.h"
#include "eclipse/scene/mat_expr_parser.hxx"
#include "eclipse/scene/known_ior.h"
#include "eclipse/util/logger.h"
#include "eclipse/math/vec3.h"

namespace eclipse { namespace material {

std::unique_ptr<NExpression> parse_expr(const std::string& expr)
{
    class ParsedExpression : public ParserCallback
    {
    public:
        void set(NExpression* expr) override
        {
            expression = std::unique_ptr<NExpression>(expr);
        }
        std::unique_ptr<NExpression> get()
        {
            return std::move(expression);
        }
        std::unique_ptr<NExpression> expression;
    };

    ParsedExpression out;
    std::istringstream in(expr);
    MatExprScanner scanner(&in);
    MatExprParser parser(&scanner, &out);

    try
    {
        if (parser.parse() != 0)
            throw std::runtime_error("Unknown material expression parsing error");
    }
    catch (MatExprSyntaxError& e)
    {
        int col = e.location.begin.column;
        int len = 1 + e.location.end.column - col;
        std::string msg =
            std::string(e.what()) + "\n" +
            "in col " + std::to_string(col) + ":\n\n" +
            "    " + expr + "\n" +
            "    " + std::string(col-1, ' ') + std::string(len, '^');
        throw MatExprSyntaxError(e.location, msg);
    }

    return std::move(out.get());
}

std::string validate_texture_name(const std::string& name)
{
    if (name.empty())
        return "Texure name cannot be empty";
    return "";
}

std::string validate_known_ior_name(const std::string& name)
{
    if (name.empty())
        return "Material name cannot be empty";
    return "";
}

namespace bxdf {

    std::string to_string(Type bxdf)
    {
        switch (bxdf)
        {
            case EMISSIVE:         return "emissive";
            case DIFFUSE:          return "diffuse";
            case CONDUCTOR:        return "conductor";
            case ROUGH_CONDUCTOR:  return "roughConductor";
            case DIELECTRIC:       return "dielectric";
            case ROUGH_DIELECTRIC: return "roughDielectric";
            default:               return "invalid";
        }
    }

    namespace param {

        std::string to_string(bxdf::param::Type param)
        {
            switch (param)
            {
                case REFLECTANCE:   return "reflectance";
                case SPECULARITY:   return "specularity";
                case TRANSMITTANCE: return "transmittance";
                case RADIANCE:      return "radiance";
                case INT_IOR:       return "intIOR";
                case EXT_IOR:       return "extIOR";
                case SCALE:         return "scale";
                case ROUGHNESS:     return "roughness";
                default:            return "invalid";
            }
        }

        Value Value::num(float v)
        {
            return { NUM, Vec3(v, 0, 0), "" };
        }

        Value Value::vec3(float x, float y, float z)
        {
            return { VECTOR, Vec3(x, y, z), "" };
        }

        Value Value::texture(std::string name)
        {
            return { TEXTURE, Vec3(0, 0, 0), std::move(name) };
        }

        Value Value::known_ior(std::string name)
        {
            return { KNOWN_IOR, Vec3(0, 0, 0), std::move(name) };
        }

        std::string Value::validate() const
        {
            if (type == TEXTURE)
                return validate_texture_name(name);
            else if (type == KNOWN_IOR)
                return validate_known_ior_name(name);
            return "";
        }

} } // namespace bxdf::param

std::string NBxdfParam::validate() const
{
    switch (type)
    {
    case bxdf::param::INVALID:
        return "Invalid BXDF type";
    case bxdf::param::REFLECTANCE:
        if (value.type == bxdf::param::VECTOR &&
                (value.value[0] >= 1.0f || value.value[1] >= 1.0f || value.value[2] >= 1.0f))
            return "Energy conservation violation for parameter " + bxdf::param::to_string(type) +
                   "; ensure that all vector components are < 1.0";
        break;
    case bxdf::param::SPECULARITY:
    case bxdf::param::TRANSMITTANCE:
        if (value.type == bxdf::param::VECTOR &&
                (value.value[0] >= 1.0f || value.value[1] >= 1.0f || value.value[2] >= 1.0f))
            return "Energy conservation violation for parameter " + bxdf::param::to_string(type) +
                   "; ensure that all vector components are < 1.0";
        break;
    case bxdf::param::ROUGHNESS:
        if (value.type == bxdf::param::NUM && (value.value[0] < 0.0f || value.value[0] > 1.0f))
            return "Values for parameter " + bxdf::param::to_string(type) + " must be in the [0, 1] range";
        break;
    case bxdf::param::INT_IOR:
    case bxdf::param::EXT_IOR:
        if (value.type == bxdf::param::KNOWN_IOR)
        {
            std::string error;
            get_known_ior(value.name, error);
            if (!error.empty())
                return error;
        }
        break;
    case bxdf::param::RADIANCE:
        break;
    case bxdf::param::SCALE:
        break;
    }
    return value.validate();
}

NBxdf::NBxdf(bxdf::Type type, NBxdfParamList params)
        : type(type), parameters(std::move(params))
{
}

std::map<bxdf::Type, std::vector<bxdf::param::Type>> allowed_bxdf_params =
{
    { bxdf::INVALID,          {                             } },
    { bxdf::DIFFUSE,          { bxdf::param::REFLECTANCE    } },
    { bxdf::EMISSIVE,         { bxdf::param::RADIANCE,
                                bxdf::param::SCALE          } },
    { bxdf::CONDUCTOR,        { bxdf::param::SPECULARITY,
                                bxdf::param::INT_IOR,
                                bxdf::param::EXT_IOR        } },
    { bxdf::ROUGH_CONDUCTOR,  { bxdf::param::SPECULARITY,
                                bxdf::param::INT_IOR,
                                bxdf::param::EXT_IOR,
                                bxdf::param::ROUGHNESS      } },
    { bxdf::DIELECTRIC,       { bxdf::param::SPECULARITY,
                                bxdf::param::TRANSMITTANCE,
                                bxdf::param::INT_IOR,
                                bxdf::param::EXT_IOR        } },
    { bxdf::ROUGH_DIELECTRIC, { bxdf::param::SPECULARITY,
                                bxdf::param::TRANSMITTANCE,
                                bxdf::param::INT_IOR,
                                bxdf::param::EXT_IOR,
                                bxdf::param::ROUGHNESS      } }
};

std::string NBxdf::validate() const
{
    if (type == bxdf::INVALID)
        return "Invalid BXDF type";

    for (auto& param : parameters)
    {
        auto map_it = allowed_bxdf_params.find(type);
        std::vector<bxdf::param::Type>& allowed_params = map_it->second;
        if (std::find(allowed_params.begin(), allowed_params.end(), param.type) == allowed_params.end())
            return "Bxdf type " + bxdf::to_string(type) + " does not support parameter " + bxdf::param::to_string(param.type);

        std::string error = param.validate();
        if (!error.empty())
            return error;
    }
    return "";
}

NMatRef::NMatRef(const std::string& mat)
    : name(mat)
{
}

std::string NMatRef::validate() const
{
    if (name.empty())
        return "MatRef: material name cannot be empty";
    return "";
}

NMix::NMix(NExpression* left, NExpression* right, float w)
    : expressions{ std::unique_ptr<NExpression>(left), std::unique_ptr<NExpression>(right) }
    , weight(w)
{
}

std::string NMix::validate() const
{
    if (!expressions[0])
        return "NormalMap: missing left expression argument";
    else if (!expressions[1])
        return "NormalMap: missing right expression argument";

    std::string error;
    error = expressions[0]->validate();
    if (!error.empty())
        return "NormalMap: " + error;
    error = expressions[1]->validate();
    if (!error.empty())
        return "NormalMap: " + error;

    if (weight < 0.0f || weight > 1.0f)
        return "NormalMap: mix weight must be in the [0, 1] range";
    return "";
}

NMixMap::NMixMap(NExpression* left, NExpression* right, bxdf::param::Value tex)
    : expressions{ std::unique_ptr<NExpression>(left), std::unique_ptr<NExpression>(right) }
    , texture(tex.name)
{
}

std::string NMixMap::validate() const
{
    if (!expressions[0])
        return "MixMap: missing left expression argument";
    else if (!expressions[1])
        return "MixMap: missing right expression argument";

    std::string error;
    error = expressions[0]->validate();
    if (!error.empty())
        return "MixMap: " + error;
    error = expressions[1]->validate();
    if (!error.empty())
        return "MixMap: " + error;

    error = validate_texture_name(texture);
    if (!error.empty())
        return "MixMap: " + error;
    return error;
}

NBumpMap::NBumpMap(NExpression* expr, bxdf::param::Value tex)
    : expression(expr), texture(tex.name)
{
}

std::string NBumpMap::validate() const
{
    if (!expression)
        return "BumpMap: missing expression argument";

    std::string error = expression->validate();
    if (!error.empty())
        return "BumpMap: " + error;

    error = validate_texture_name(texture);
    if (!error.empty())
        return "BumpMap: " + error;
    return error;
}

NNormalMap::NNormalMap(NExpression* expr, bxdf::param::Value tex)
    : expression(expr), texture(tex.name)
{
}

std::string NNormalMap::validate() const
{
    if (!expression)
        return "NormalMap: missing expression argument";

    std::string error = expression->validate();
    if (!error.empty())
        return "NormalMap: " + error;

    error = validate_texture_name(texture);
    if (!error.empty())
        return "NormalMap: " + error;
    return error;
}

NDisperse::NDisperse(NExpression* expr, bxdf::param::Value iior, bxdf::param::Value eior)
    : expression(expr), int_ior(iior.value), ext_ior(eior.value)
{
}

std::string NDisperse::validate() const
{
    if (!expression)
        return "Disperse: missing expression argument";

    std::string error = expression->validate();
    if (!error.empty())
        return "Dispers: " + error;

    if (max_component(int_ior) == 0.0f && max_component(ext_ior) == 0.0f)
    {
        return "Disperse: at least one of the intIOR and extIOR parameters must contain a non-zero value";
    }
    return error;
}

void test_expr_parser()
{
    const char* expressions[] = {
    	"diffuse()",
		"emissive()",
		"emissive(scale: 10)",
		"diffuse(reflectance: {0.9, 0.9, 0.9})",
		"diffuse(reflectance: \"texture.jpEg\")",
		"conductor(specularity: \"texture.jpg\")",
		"emissive(radiance: {1,1,1}, scale: 10)",
		"bumpMap(conductor(specularity: \"texture.jpg\"), \"foo.jpg\")",
		"normalMap(conductor(specularity: \"texture.jpg\"), \"foo.jpg\")",
		"roughConductor(specularity: {.3,.3,.3}, intIOR: \"gold\", roughness: 1)",
		"dielectric(specularity: \"texture.jpg\", intIOR: \"gold\", extIOR: \"air\")",
		"mix(diffuse(reflectance:{0.2, 0.2, 0.2}), conductor(specularity: \"texture.jpg\"), 0.2)",
		"dielectric(specularity: \"texture.jpEg\", transmittance: {.9,.9,.9}, intIOR: 1.33, extIOR: \"air\")",
		"roughDielectric(specularity: \"texture.jpEg\", transmittance: {1,1,1}, intIOR: 1.33, extIOR: \"air\", roughness: 0.2)",
    };

    const unsigned num = sizeof(expressions) / sizeof(expressions[0]);

    LOG_INFO("Tesing ", num, " expressions");

    for (unsigned i = 0; i < num; ++i)
    {
        LOG_INFO("#", i, ": ", expressions[i]);
        std::unique_ptr<NExpression> expr = parse_expr(expressions[i]);
    }
}

} } // namespace eclipse::material
