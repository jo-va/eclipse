#include <memory>
#include <utility>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>

#include "eclipse/scene/mat_expr.h"
#include "eclipse/scene/mat_expr_scanner.h"
#include "eclipse/scene/mat_expr_parser.hxx"
#include "eclipse/scene/material_except.h"
#include "eclipse/scene/known_ior.h"
#include "eclipse/util/logger.h"
#include "eclipse/math/vec3.h"

namespace eclipse { namespace material {

ExprNodePtr parse_expr(const std::string& expr)
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
            throw ParseError("Unknown material expression parsing error");
        }
    }
    catch (MatExprSyntaxError& e)
    {
        int col = e.location.begin.column;
        int len = 1 + e.location.end.column - col;
        std::string error = std::string(e.what()) + "\n" +
                            "in col " + std::to_string(col) + ":\n\n" +
                            "    " + expr + "\n" +
                            "    " + std::string(col-1, ' ') + std::string(len, '^');
        throw ParseError(error);
    }

    return std::move(out.get());
}

void validate_texture_name(const std::string& name)
{
    if (name.empty())
        throw ValidationError("Texture name cannot be empty");
}

void validate_known_ior_name(const std::string& name)
{
    if (name.empty())
        throw ValidationError("Material name cannot be empty");
}

std::string node_to_string(NodeType node)
{
    switch (node)
    {
        case BXDF_DIFFUSE:          return "diffuse";
        case BXDF_CONDUCTOR:        return "conductor";
        case BXDF_ROUGH_CONDUCTOR:  return "roughConductor";
        case BXDF_DIELECTRIC:       return "dielectric";
        case BXDF_ROUGH_DIELECTRIC: return "roughDielectric";
        case BXDF_EMISSIVE:         return "emissive";
        case OP_MIX:                return "mix";
        case OP_MIXMAP:             return "mixMap";
        case OP_BUMPMAP:            return "bumpMap";
        case OP_NORMALMAP:          return "normalMap";
        case OP_DISPERSE:           return "disperse";
        default:                    return "invalid";
    }
}

std::string param_to_string(ParamType param)
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
        case WEIGHT:        return "weight";
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

void ParamValue::validate() const
{
    if (type == TEXTURE)
        validate_texture_name(name);
    else if (type == KNOWN_IOR)
        validate_known_ior_name(name);
}

void NBxdfParam::validate() const
{
    switch (type)
    {
    case PARAM_NONE:
        throw ValidationError("Invalid BXDF param type");
    case REFLECTANCE:
        if (value.type == VECTOR && (value.vec[0] >= 1.0f || value.vec[1] >= 1.0f || value.vec[2] >= 1.0f))
            throw ValidationError("Energy conservation violation for parameter " + param_to_string(type) +
                                  "; ensure that all vector components are < 1.0");
        break;
    case SPECULARITY:
    case TRANSMITTANCE:
        if (value.type == VECTOR && (value.vec[0] >= 1.0f || value.vec[1] >= 1.0f || value.vec[2] >= 1.0f))
            throw ValidationError("Energy conservation violation for parameter " + param_to_string(type) +
                                  "; ensure that all vector components are < 1.0");
        break;
    case ROUGHNESS:
        if (value.type == NUM && (value.vec[0] < 0.0f || value.vec[0] > 1.0f))
            throw ValidationError("Values for parameter " + param_to_string(type) + " must be in the [0, 1] range");
        break;
    case INT_IOR:
    case EXT_IOR:
        if (value.type == KNOWN_IOR)
            get_known_ior(value.name);
        break;
    default:
        break;
    }
    value.validate();
}

NBxdf::NBxdf(NodeType type, NBxdfParamList params)
        : type(type), parameters(std::move(params))
{
}

std::map<NodeType, std::vector<ParamType>> allowed_bxdf_params =
{
    { NODE_NONE,             {                } },
    { BXDF_DIFFUSE,          { REFLECTANCE    } },
    { BXDF_EMISSIVE,         { RADIANCE,
                               SCALER         } },
    { BXDF_CONDUCTOR,        { SPECULARITY,
                               INT_IOR,
                               EXT_IOR        } },
    { BXDF_ROUGH_CONDUCTOR,  { SPECULARITY,
                               INT_IOR,
                               EXT_IOR,
                               ROUGHNESS      } },
    { BXDF_DIELECTRIC,       { SPECULARITY,
                               TRANSMITTANCE,
                               INT_IOR,
                               EXT_IOR        } },
    { BXDF_ROUGH_DIELECTRIC, { SPECULARITY,
                               TRANSMITTANCE,
                               INT_IOR,
                               EXT_IOR,
                               ROUGHNESS      } }
};

void NBxdf::validate() const
{
    if (type == NODE_NONE)
        throw ValidationError("Invalid BXDF type");

    for (auto& param : parameters)
    {
        auto map_it = allowed_bxdf_params.find(type);
        std::vector<ParamType>& allowed_params = map_it->second;
        if (std::find(allowed_params.begin(), allowed_params.end(), param.type) == allowed_params.end())
            throw ValidationError("Bxdf type " + node_to_string(type) +
                    " does not support parameter " + param_to_string(param.type));

        param.validate();
    }
}

NMatRef::NMatRef(const std::string& mat)
    : name(mat)
{
}

void NMatRef::validate() const
{
    if (name.empty())
        throw ValidationError("MatRef: material name cannot be empty");
}

NMix::NMix(ExprNode* left, ExprNode* right, float w)
    : expressions{ std::shared_ptr<ExprNode>(left), std::shared_ptr<ExprNode>(right) }
    , weight(w)
{
}

void NMix::validate() const
{
    if (!expressions[0])
        throw ValidationError("Mix: missing left expression argument");
    else if (!expressions[1])
        throw ValidationError("Mix: missing right expression argument");

    try
    {
        expressions[0]->validate();
        expressions[1]->validate();
    }
    catch (ValidationError& e)
    {
        throw ValidationError("Mix: " + std::string(e.what()));
    }

    if (weight < 0.0f || weight > 1.0f)
        throw ValidationError("Mix: mix weight must be in the [0, 1] range");
}

NMixMap::NMixMap(ExprNode* left, ExprNode* right, ParamValue tex)
    : expressions{ std::shared_ptr<ExprNode>(left), std::shared_ptr<ExprNode>(right) }
    , texture(tex.name)
{
}

void NMixMap::validate() const
{
    if (!expressions[0])
        throw ValidationError("MixMap: missing left expression argument");
    else if (!expressions[1])
        throw ValidationError("MixMap: missing right expression argument");

    try
    {
        expressions[0]->validate();
        expressions[1]->validate();
        validate_texture_name(texture);
    }
    catch (ValidationError& e)
    {
        throw ValidationError("MixMap: " + std::string(e.what()));
    }
}

NBumpMap::NBumpMap(ExprNode* expr, ParamValue tex)
    : expression(expr), texture(tex.name)
{
}

void NBumpMap::validate() const
{
    if (!expression)
        throw ValidationError("BumpMap: missing expression argument");

    try
    {
        expression->validate();
        validate_texture_name(texture);
    }
    catch (ValidationError& e)
    {
        throw ValidationError("BumpMap: " + std::string(e.what()));
    }
}

NNormalMap::NNormalMap(ExprNode* expr, ParamValue tex)
    : expression(expr), texture(tex.name)
{
}

void NNormalMap::validate() const
{
    if (!expression)
        throw ValidationError("NormalMap: missing expression argument");

    try
    {
        expression->validate();
        validate_texture_name(texture);
    }
    catch (ValidationError& e)
    {
        throw ValidationError("NormalMap: " + std::string(e.what()));
    }
}

NDisperse::NDisperse(ExprNode* expr, ParamValue iior, ParamValue eior)
    : expression(expr), int_ior(iior.vec), ext_ior(eior.vec)
{
}

void NDisperse::validate() const
{
    if (!expression)
        throw ValidationError("Disperse: missing expression argument");

    try
    {
        expression->validate();
    }
    catch (ValidationError& e)
    {
        throw ValidationError("Disperse: " + std::string(e.what()));
    }

    if (max_component(int_ior) == 0.0f && max_component(ext_ior) == 0.0f)
    {
        throw ValidationError("Disperse: at least one of the intIOR and extIOR parameters must contain a non-zero value");
    }
}

} } // namespace eclipse::material
