#include <GG/VectorTexture.h>

#include <GG/GLClientAndServerBuffer.h>
#include <GG/Config.h>
#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg/nanosvg.h"

#include "nanovg/nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/case_conv.hpp>

using namespace GG;

namespace {
    NVGcontext* VG() {
        static NVGcontext* s_vg = nullptr;
        if (!s_vg) {
            s_vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);   // leaks, as NVGcontext only defined in nanovg.c
            std::cout << "Initialized NVGcontext!" << std::endl;
        }
        return s_vg;
    }


    //// Begin code adapted from https://github.com/VCVRack/Rack/blob/v0.6/src/widgets/SVGWidget.cpp

    struct Vec {
        float x = 0.f;
        float y = 0.f;

        Vec() {}

        Vec(float x, float y) :
            x(x), y(y)
        {}

        Vec minus(Vec b)
        { return Vec(x - b.x, y - b.y); }
    };

    static NVGcolor getNVGColor(uint32_t color) {
        return nvgRGBA(
            (color >> 0) & 0xff,
            (color >> 8) & 0xff,
            (color >> 16) & 0xff,
            (color >> 24) & 0xff);
    }

    static NVGpaint getPaint(NVGcontext *vg, NSVGpaint *p) {
        assert(p->type == NSVG_PAINT_LINEAR_GRADIENT || p->type == NSVG_PAINT_RADIAL_GRADIENT);
        NSVGgradient *g = p->gradient;
        assert(g->nstops >= 1);
        NVGcolor icol = getNVGColor(g->stops[0].color);
        NVGcolor ocol = getNVGColor(g->stops[g->nstops - 1].color);

        float inverse[6];
        nvgTransformInverse(inverse, g->xform);
        std::cout << "            inverse: " << inverse[0] << " " << inverse[1]
                  << " " << inverse[2] << " " << inverse[3]
                  << " " << inverse[4] << " " << inverse[5] << std::endl;
        Vec s, e;
        std::cout << "            sx: " << s.x << " sy: " << s.y << " ex: " << e.x << " ey: " << e.y << std::endl;
        // Is it always the case that the gradient should be transformed from (0, 0) to (0, 1)?
        nvgTransformPoint(&s.x, &s.y, inverse, 0, 0);
        nvgTransformPoint(&e.x, &e.y, inverse, 0, 1);
        std::cout << "            sx: " << s.x << " sy: " << s.y << " ex: " << e.x << " ey: " << e.y << std::endl;

        NVGpaint paint;
        if (p->type == NSVG_PAINT_LINEAR_GRADIENT)
            paint = nvgLinearGradient(vg, s.x, s.y, e.x, e.y, icol, ocol);
        else
            paint = nvgRadialGradient(vg, s.x, s.y, 0.0, 160, icol, ocol);
        return paint;
    }

    /** Returns the parameterized value of the line p2--p3 where it intersects with p0--p1 */
    static float getLineCrossing(Vec p0, Vec p1, Vec p2, Vec p3) {
        Vec b = p2.minus(p0);
        Vec d = p1.minus(p0);
        Vec e = p3.minus(p2);
        float m = d.x * e.y - d.y * e.x;
        // Check if lines are parallel, or if either pair of points are equal
        if (fabsf(m) < 1e-6)
            return NAN;
        return -(d.x * b.y - d.y * b.x) / m;
    }

    static void drawSVG(NVGcontext *vg, NSVGimage *svg) {
        //std::cout << "drawing image sized: " << svg->width << " x " << svg->height << " px" << std::endl;
        int shapeIndex = 0;
        // Iterate shape linked list
        for (NSVGshape *shape = svg->shapes; shape; shape = shape->next, shapeIndex++) {
            //std::cout << "    new shape: " << shapeIndex << "  id: " << shape->id
            //          << "  fillrule: " << shape->fillRule
            //          << "  from (" << shape->bounds[0] << ", " << shape->bounds[1]
            //          << ") to (" << shape->bounds[2] << ", " << shape->bounds[3] << ")" << std::endl;
            // Visibility
            if (!(shape->flags & NSVG_FLAGS_VISIBLE))
                continue;

            nvgSave(vg);

            // Opacity
            if (shape->opacity < 1.0)
                nvgGlobalAlpha(vg, shape->opacity);

            // Build path
            nvgBeginPath(vg);

            // Iterate path linked list
            for (NSVGpath *path = shape->paths; path; path = path->next) {
                //DEBUG_ONLY(printf("        new path: %d points, %s, from (%f, %f) to (%f, %f)\n", path->npts, path->closed ? "closed" : "open", path->bounds[0], path->bounds[1], path->bounds[2], path->bounds[3]);)

                nvgMoveTo(vg, path->pts[0], path->pts[1]);
                for (int i = 1; i < path->npts; i += 3) {
                    float *p = &path->pts[2*i];
                    nvgBezierTo(vg, p[0], p[1], p[2], p[3], p[4], p[5]);
                    // nvgLineTo(vg, p[4], p[5]);
                    //DEBUG_ONLY(printf("            bezier (%f, %f) to (%f, %f)\n", p[-2], p[-1], p[4], p[5]);)
                }

                // Close path
                if (path->closed)
                    nvgClosePath(vg);

                // Compute whether this is a hole or a solid.
                // Assume that no paths are crossing (usually true for normal SVG graphics).
                // Also assume that the topology is the same if we use straight lines rather than Beziers (not always the case but usually true).
                // Using the even-odd fill rule, if we draw a line from a point on the path to a point outside the boundary (e.g. top left) and count the number of times it crosses another path, the parity of this count determines whether the path is a hole (odd) or solid (even).
                int crossings = 0;
                Vec p0 = Vec(path->pts[0], path->pts[1]);
                Vec p1 = Vec(path->bounds[0] - 1.0, path->bounds[1] - 1.0);
                // Iterate all other paths
                for (NSVGpath *path2 = shape->paths; path2; path2 = path2->next) {
                    if (path2 == path)
                        continue;

                    // Iterate all lines on the path
                    if (path2->npts < 4)
                        continue;
                    for (int i = 1; i < path2->npts + 3; i += 3) {
                        float *p = &path2->pts[2*i];
                        // The previous point
                        Vec p2 = Vec(p[-2], p[-1]);
                        // The current point
                        Vec p3 = (i < path2->npts) ? Vec(p[4], p[5]) : Vec(path2->pts[0], path2->pts[1]);
                        float crossing = getLineCrossing(p0, p1, p2, p3);
                        float crossing2 = getLineCrossing(p2, p3, p0, p1);
                        if (0.0 <= crossing && crossing < 1.0 && 0.0 <= crossing2) {
                            crossings++;
                        }
                    }
                }

                if (crossings % 2 == 0)
                    nvgPathWinding(vg, NVG_SOLID);
                else
                    nvgPathWinding(vg, NVG_HOLE);
            }

            // Fill shape
            if (shape->fill.type) {
                switch (shape->fill.type) {
                    case NSVG_PAINT_COLOR: {
                        NVGcolor color = getNVGColor(shape->fill.color);
                        nvgFillColor(vg, color);
                        //DEBUG_ONLY(printf("        fill color (%g, %g, %g, %g)\n", color.r, color.g, color.b, color.a);)
                    } break;
                    case NSVG_PAINT_LINEAR_GRADIENT:
                    case NSVG_PAINT_RADIAL_GRADIENT: {
                        NSVGgradient *g = shape->fill.gradient;
                        (void)g;
                        //DEBUG_ONLY(printf("        gradient: type: %s xform: %f %f %f %f %f %f spread: %d fx: %f fy: %f nstops: %d\n", (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT ? "linear" : "radial"), g->xform[0], g->xform[1], g->xform[2], g->xform[3], g->xform[4], g->xform[5], g->spread, g->fx, g->fy, g->nstops);)
                        for (int i = 0; i < g->nstops; i++) {
                            //DEBUG_ONLY(printf("            stop: #%08x\t%f\n", g->stops[i].color, g->stops[i].offset);)
                        }
                        nvgFillPaint(vg, getPaint(vg, &shape->fill));
                    } break;
                }
                nvgFill(vg);
            }

            // Stroke shape
            if (shape->stroke.type) {
                nvgStrokeWidth(vg, shape->strokeWidth);
                // strokeDashOffset, strokeDashArray, strokeDashCount not yet supported
                nvgLineCap(vg, (NVGlineCap) shape->strokeLineCap);
                nvgLineJoin(vg, (int) shape->strokeLineJoin);

                switch (shape->stroke.type) {
                    case NSVG_PAINT_COLOR: {
                        NVGcolor color = getNVGColor(shape->stroke.color);
                        nvgStrokeColor(vg, color);
                        //DEBUG_ONLY(printf("        stroke color (%g, %g, %g, %g)\n", color.r, color.g, color.b, color.a);)
                    } break;
                    case NSVG_PAINT_LINEAR_GRADIENT: {
                        // NSVGgradient *g = shape->stroke.gradient;
                        // printf("        lin grad: %f\t%f\n", g->fx, g->fy);
                    } break;
                }
                nvgStroke(vg);
            }

            nvgRestore(vg);
        }
    }

    //// End code from https://github.com/VCVRack/Rack/blob/v0.6/src/widgets/SVGWidget.cpp
}

class VectorTextureImpl {
public:
    void Load(const boost::filesystem::path& path)
    {
        std::cout << "VectorTextureImpl::Load(" << path.generic_string() << ")" << std::endl;

        namespace fs = boost::filesystem;

        if (!fs::exists(path)) {
            std::cerr << "VectorTexture::Load passed non-existant path: " << path.generic_string() << std::endl;
            throw VectorTexture::BadFile("VectorTexture file \"" + path.generic_string() + "\" does not exist");
        }
        if (!fs::is_regular_file(path)) {
            std::cerr << "VectorTexture::Load passed non-file path: " << path.generic_string() << std::endl;
            throw VectorTexture::BadFile("VectorTexture \"file\" \"" + path.generic_string() + "\" is not a file");
        }

        // convert path into UTF-8 format filename string
#if defined (_WIN32)
        boost::filesystem::path::string_type path_native = path.native();
        std::string filename;
        utf8::utf16to8(path_native.begin(), path_native.end(), std::back_inserter(filename));
#else
        std::string filename = path.generic_string();
#endif

        if (!fs::exists(path))
            throw VectorTexture::BadFile("VectorTexture file \"" + filename + "\" does not exist");
        if (!fs::is_regular_file(path))
            throw VectorTexture::BadFile("VectorTexture \"file\" \"" + filename + "\" is not a file");

        std::string extension = boost::algorithm::to_lower_copy(path.extension().string());

        if (extension == ".svg") {
            try {
                nsvg_image.reset(nsvgParseFromFile(filename.c_str(), "px", 96.0f));

                if (nsvg_image)
                    std::cout << "SVG Loaded: " << filename << "  with " << NumShapes() << " shapes" << std::endl;
                else
                    throw std::exception("null image pointer...");

                //fs::ifstream ifs(path, std::ios_base::binary);
                //std::ostringstream buf;
                //buf << ifs.rdbuf();
                //nsvg_image.reset(nsvgParse(buf.rdbuf(), "px", 96.0f));


            } catch (const std::exception& e) {
                std::cerr << "SVG Load failed!: " << e.what() << std::endl;
            }
        }
    }

    void Render(const Pt& ul, const Pt& lr)
    {
        if (!nsvg_image)
            return;

        float x0 = Value(ul.x);
        float y0 = Value(ul.y);
        float draw_w = Value((lr - ul).x);
        float draw_h = Value((lr - ul).y);

        float img_w = nsvg_image->width;
        if (img_w == 0.0f)
            img_w = 1.0f;
        float img_h = nsvg_image->height;
        if (img_h == 0.0f)
            img_h = 1.0f;

        float draw_scale_x = draw_w / img_w;
        float draw_scale_y = draw_h / img_h;

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        //std::cout << "Rendering " << NumShapes() << " shapes!" << std::endl;

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glPushMatrix();

        nvgBeginFrame(VG(), viewport[2] - viewport[0], viewport[3] - viewport[1], 1.0f);

        Rect r = ActiveScissorClippingRegion();
        if (r != Rect()) {
            float clip_x0 = Value(r.ul.x);
            float clip_y0 = Value(r.ul.y);
            float clip_w = Value(r.Width());
            float clip_h = Value(r.Height());
            nvgScissor(VG(), clip_x0, clip_y0, clip_w, clip_h);
        }

        nvgTranslate(VG(), x0, y0);
        nvgScale(VG(), draw_scale_x, draw_scale_y);

        drawSVG(VG(), nsvg_image.get());

        nvgEndFrame(VG());

        glPopMatrix();
        glPopClientAttrib();
        glPopAttrib();

        glEnable(GL_TEXTURE_2D);
    }

    int NumShapes() const
    {
        if (!nsvg_image)
            return 0;
        if (!nsvg_image->shapes)
            return 0;
        int count = 0;
        auto* shape = nsvg_image->shapes;
        while (shape) {
            ++count;
            shape = shape->next;
        }
        return count;
    }

    bool ImageLoaded() const
    { return nsvg_image.get(); }

    Pt Size()
    {
        if (!nsvg_image)
            return Pt();
        return {GG::X(nsvg_image->width), GG::Y(nsvg_image->height)};
    }

    std::shared_ptr<NSVGimage> nsvg_image;
};

///////////////////////////////////////
// class GG::VectorTexture
///////////////////////////////////////
VectorTexture::VectorTexture() :
    m_impl(new VectorTextureImpl())
{}

const boost::filesystem::path& VectorTexture::Path() const
{ return m_path; }

bool VectorTexture::TextureLoaded() const
{ return m_impl->ImageLoaded(); }

Pt VectorTexture::Size() const
{ return m_impl->Size(); }

void VectorTexture::Render(const Pt& ul, const Pt& lr) const
{ m_impl->Render(ul, lr); }

void VectorTexture::Load(const boost::filesystem::path& path)
{
    m_impl->Load(path);
    m_path = path;
}

///////////////////////////////////////
// class GG::VectorTextureManager
///////////////////////////////////////

VectorTextureManager::VectorTextureManager()
{}

const std::map<std::string, std::shared_ptr<VectorTexture>>& VectorTextureManager::Textures() const
{ return m_textures; }

std::shared_ptr<VectorTexture> VectorTextureManager::GetTexture(const boost::filesystem::path& path)
{
    std::map<std::string, std::shared_ptr<VectorTexture>>::iterator it = m_textures.find(path.generic_string());
    if (it == m_textures.end()) { // if no such texture was found, attempt to load it now, using name as the filename
        //std::cout << "VectorTextureManager::GetTexture storing new texture under name: " << path.generic_string();
        return (m_textures[path.generic_string()] = LoadTexture(path));
    } else { // otherwise, just return the texture we found
        return it->second;
    }
}

void VectorTextureManager::FreeTexture(const boost::filesystem::path& path)
{ FreeTexture(path.generic_string()); }

void VectorTextureManager::FreeTexture(const std::string& name)
{
    std::map<std::string, std::shared_ptr<VectorTexture>>::iterator it = m_textures.find(name);
    if (it != m_textures.end())
        m_textures.erase(it);
}

std::shared_ptr<VectorTexture> VectorTextureManager::LoadTexture(const boost::filesystem::path& path)
{
    auto temp = std::make_shared<VectorTexture>();
    temp->Load(path);
    return (m_textures[path.generic_string()] = temp);
}

VectorTextureManager& GG::GetVectorTextureManager()
{
    static VectorTextureManager manager;
    return manager;
}
