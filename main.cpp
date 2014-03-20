#include <phpcpp.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <sys/stat.h>
#include <sstream>

class Poppler : public Php::Base
{
private:
    poppler::document *_document;

    bool doesFileExist(const std::string& name)
    {
#ifdef OS_WINDOWS
        struct _stat buf;
        int result = _stat(name.c_str(), &buf);
#else
        struct stat buf;
        int result = stat(name.c_str(), &buf);
#endif

        return result == 0;
    }
public:
    Poppler() {}
    virtual ~Poppler() {}

    /**
     * Load a given PDF document
     */
    void load(Php::Parameters &parameters)
    {
        std::string file = parameters[0];

        /**
         * Throw an exception if the file doesn't exist
         */
        if (!this->doesFileExist(file)) {
            std::stringstream message;
            message << "File " << file << " does not exist or cannot be opened";
            throw Php::Exception(message.str());
        }

        this->_document = poppler::document::load_from_file(file);
    }

    Php::Value getText()
    {
        int pages = this->_document->pages();

        std::stringstream content;

        for(int i = 0; i < pages; i++) {
            poppler::byte_array text = this->_document->create_page(i)->text().to_utf8();
            for (unsigned int i = 0; i < text.size(); i++) {
                content << text[i];
            }
        }

        return content.str();
    }

    void saveImage()
    {
        poppler::page_renderer pageRenderer;
        pageRenderer.set_render_hint(poppler::page_renderer::antialiasing);
        pageRenderer.set_render_hint(poppler::page_renderer::text_antialiasing);

        std::auto_ptr<poppler::page> page(this->_document->create_page(1));

        poppler::image image = pageRenderer.render_page(page.get());

        if (!image.save("output.png", "png", 3)) {
            throw Php::Exception("Could not save file successfully");
        }
    }
};

extern "C" {
    PHPCPP_EXPORT void *get_module()
    {
        static Php::Extension extension("poppler", "0.5");

        Php::Class<Poppler> poppler("Poppler");
        poppler.method("load", &Poppler::load);
        poppler.method("getText", &Poppler::getText);
        poppler.method("saveImage", &Poppler::saveImage);

        extension.add(std::move(poppler));

        return extension;
    }
}