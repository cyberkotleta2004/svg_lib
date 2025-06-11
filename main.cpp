#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>
#include <fstream>

using namespace std::literals;
using namespace svg;

/*
Пример использования библиотеки. Он будет компилироваться и работать, когда вы реализуете
все классы библиотеки.
*/

namespace {

Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    return polyline;
}

// Выводит приветствие, круг и звезду
void DrawPicture() {
    std::ofstream out("draw.svg");
    if(!out) {
        return;
    }
    Document doc;
    doc.Add(Circle().SetCenter({20, 20}).SetRadius(10));
    doc.Add(Text()
                .SetFontFamily("Verdana"s)
                .SetPosition({35, 20})
                .SetOffset({0, 6})
                .SetFontSize(12)
                .SetFontWeight("bold"s)
                .SetData("Hello C++"s));
    doc.Add(CreateStar({20, 50}, 10, 5, 5));
    doc.Render(out);
}

}  // namespace


int main() {
    DrawPicture();
    // Document doc;
    // doc.Add(Circle().SetCenter({20, 20}).SetRadius(10));
    // doc.Add(Text().SetFontSize(12).SetFontFamily("Verdana").SetFontWeight("bold").SetPosition({100, 100}).SetData("sndfj"));
    // doc.Add(Polyline().AddPoint({100, 100}).AddPoint({200, 200}).AddPoint({300, 300}));
    // std::ofstream out("output.svg");
    // if(!out) {
    //     std::cerr << "error";
    //     return 1;
    // }
    // doc.Render(out);
}