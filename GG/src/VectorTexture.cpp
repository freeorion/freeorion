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
           // std::cout << "Initialized NVGcontext!" << std::endl;
        }
        return s_vg;
    }
}

namespace rack {
    void svgDraw(NVGcontext *vg, NSVGimage *svg);
} // namespace rack


class VectorTextureImpl {
public:
    void Load(const boost::filesystem::path& path)
    {
        //std::cout << "VectorTextureImpl::Load(" << path.generic_string() << ")" << std::endl;

        namespace fs = boost::filesystem;

        if (!fs::exists(path)) {
            //std::cerr << "VectorTexture::Load passed non-existant path: " << path.generic_string() << std::endl;
            throw VectorTexture::BadFile("VectorTexture file \"" + path.generic_string() + "\" does not exist");
        }
        if (!fs::is_regular_file(path)) {
            //std::cerr << "VectorTexture::Load passed non-file path: " << path.generic_string() << std::endl;
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

                if (nsvg_image) {
                    //std::cout << "SVG Loaded: " << filename << "  with " << NumShapes() << " shapes" << std::endl;
                } else {
                    throw VectorTexture::BadFile("VectorTexture \"file\" \"" + filename + "\" gave a null image pointer");
                }

            } catch (const std::exception& e) {
                //std::cerr << "SVG Load failed!: " << e.what() << std::endl;
            }
        }
    }

    void Render(const Pt& ul, const Pt& lr)
    {
        if (!nsvg_image)
            return;

        // clear any lingering GL errors
        auto err = glGetError();

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

        rack::svgDraw(VG(), nsvg_image.get());

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

int VectorTexture::NumShapes() const
{ return m_impl->NumShapes(); }

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
