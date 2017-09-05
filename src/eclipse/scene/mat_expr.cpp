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

std::unique_ptr<ExprNode> parse_expr(const std::string& expr, std::string& error)
{
    class ParsedExpression : public ParserCallback
    {
    public:
        void set(ExprNode* expr) override
        {
            expression = std::unique_ptr<ExprNode>(expr);
        }
        std::unique_ptr<ExprNode> get()
        {
            return std::move(expression);
        }
        std::unique_ptr<ExprNode> expression;
    };

    ParsedExpression out;
    std::istringstream in(expr);
    MatExprScanner scanner(&in);
    MatExprParser parser(&scanner, &out);

    try
    {
        if (parser.parse() != 0)
        {
            error = "Unknown material expression parsing error";
            return nullptr;
        }
    }
    catch (MatExprSyntaxError& e)
    {
        int col = e.location.begin.column;
        int len = 1 + e.location.end.column - col;
        error = std::string(e.what()) + "\n" +
                "in col " + std::to_string(col) + ":\n\n" +
                "    " + expr + "\n" +
                "    " + std::string(col-1, ' ') + std::string(len, '^');
        return nullptr;
    }

    error = "";
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

std::string bxdf_to_string(BxdfType bxdf)
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

std::string bxdf_param_to_string(ParamType param)
{
    switch (param)
    {
    case REFLECTANCE:   return "reflectance";
    case SPECULARITY:   return "specularity";
    case TRANSMITTANCE: return "transmittance";
    case RADIANCE:      return "radiance";
    case INT_IOR:       return "intIOR";
    case EXT_IOR:       return "extIOR";
    case SCALER:        return "scaler";
    case ROUGHNESS:     return "roughness";
    default:            return "invalid";
    }
}

ParamValue ParamValue::num(float v)
{
    return { NUM, Vec3(v, 0, 0), "" };
}

ParamValue ParamValue::vec3(float x, float y, float z)
{
    return { VECTOR, Vec3(x, y, z), "" };
}

ParamValue ParamValue::texture(std::string name)
{
    return { TEXTURE, Vec3(0, 0, 0), std::move(name) };
}

ParamValue ParamValue::known_ior(std::string name)
{
    return { KNOWN_IOR, Vec3(0, 0, 0), std::move(name) };
}

std::string ParamValue::validate() const
{
    if (type == TEXTURE)
        return validate_texture_name(name);
    else if (type == KNOWN_IOR)
        return validate_known_ior_name(name);
    return "";
}

std::string NBxdfParam::validate() const
{
    switch (type)
    {
    case INVALID_PARAM:
        return "Invalid BXDF type";
    case REFLECTANCE:
        if (value.type == VECTOR && (value.vec[0] >= 1.0f || value.vec[1] >= 1.0f || value.vec[2] >= 1.0f))
            return "Energy conservation violation for parameter " + bxdf_param_to_string(type) +
                   "; ensure that all vector components are < 1.0";
        break;
    case SPECULARITY:
    case TRANSMITTANCE:
        if (value.type == VECTOR && (value.vec[0] >= 1.0f || value.vec[1] >= 1.0f || value.vec[2] >= 1.0f))
            return "Energy conservation violation for parameter " + bxdf_param_to_string(type) +
                   "; ensure that all vector components are < 1.0";
        break;
    case ROUGHNESS:
        if (value.type == NUM && (value.vec[0] < 0.0f || value.vec[0] > 1.0f))
            return "Values for parameter " + bxdf_param_to_string(type) + " must be in the [0, 1] range";
        break;
    case INT_IOR:
    case EXT_IOR:
        if (value.type == KNOWN_IOR)
        {
            std::string error;
            get_known_ior(value.name, error);
            if (!error.empty())
                return error;
        }
        break;
    default:
        break;
    }
    return value.validate();
}

NBxdf::NBxdf(BxdfType type, NBxdfParamList params)
        : type(type), parameters(std::move(params))
{
}

std::map<BxdfType, std::vector<ParamType>> allowed_bxdf_params =
{
    { INVALID_BXDF,     {                } },
    { DIFFUSE,          { REFLECTANCE    } },
    { EMISSIVE,         { RADIANCE,
                          SCALER         } },
    { CONDUCTOR,        { SPECULARITY,
                          INT_IOR,
                          EXT_IOR        } },
    { ROUGH_CONDUCTOR,  { SPECULARITY,
                          INT_IOR,
                          EXT_IOR,
                          ROUGHNESS      } },
    { DIELECTRIC,       { SPECULARITY,
                          TRANSMITTANCE,
                          INT_IOR,
                          EXT_IOR        } },
    { ROUGH_DIELECTRIC, { SPECULARITY,
                          TRANSMITTANCE,
                          INT_IOR,
                          EXT_IOR,
                          ROUGHNESS      } }
};

std::string NBxdf::validate() const
{
    if (type == INVALID_BXDF)
        return "Invalid BXDF type";

    for (auto& param : parameters)
    {
        auto map_it = allowed_bxdf_params.find(type);
        std::vector<ParamType>& allowed_params = map_it->second;
        if (std::find(allowed_params.begin(), allowed_params.end(), param.type) == allowed_params.end())
            return "Bxdf type " + bxdf_to_string(type) + " does not support parameter " + bxdf_param_to_string(param.type);

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

NMix::NMix(ExprNode* left, ExprNode* right, float w)
    : expressions{ std::unique_ptr<ExprNode>(left), std::unique_ptr<ExprNode>(right) }
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

NMixMap::NMixMap(ExprNode* left, ExprNode* right, ParamValue tex)
    : expressions{ std::unique_ptr<ExprNode>(left), std::unique_ptr<ExprNode>(right) }
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

NBumpMap::NBumpMap(ExprNode* expr, ParamValue tex)
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

NNormalMap::NNormalMap(ExprNode* expr, ParamValue tex)
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

NDisperse::NDisperse(ExprNode* expr, ParamValue iior, ParamValue eior)
    : expression(expr), int_ior(iior.vec), ext_ior(eior.vec)
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

} } // namespace eclipse::material
