#include <memory>
#include <utility>
#include <sstream>
#include <string>

#include "eclipse/scene/mat_expr.h"
#include "eclipse/scene/mat_expr_scanner.h"
#include "eclipse/scene/mat_expr_parser.hxx"
#include "eclipse/util/logger.h"

namespace eclipse { namespace material {

std::unique_ptr<NExpression> parse_material_expr(const std::string& expr)
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

void test_expr()
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
        std::unique_ptr<NExpression> expr = parse_material_expr(expressions[i]);
    }
}

} } // namespace eclipse::material
