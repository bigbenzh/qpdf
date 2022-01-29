#include <qpdf/QPDFJob.hh>
#include <qpdf/JSONHandler.hh>

#include <memory>
#include <stdexcept>
#include <sstream>

static JSON JOB_SCHEMA = JSON::parse(QPDFJob::json_job_schema_v1().c_str());

namespace
{
    class Handlers
    {
      public:
        Handlers(std::shared_ptr<QPDFJob::Config> c_main);
        void handle(JSON&);

      private:
//#       include <qpdf/auto_job_json_decl.hh>

        void usage(std::string const& message);
        void initHandlers();

        JSONHandler jh;
        std::shared_ptr<QPDFJob::Config> c_main;
        std::shared_ptr<QPDFJob::CopyAttConfig> c_copy_att;
        std::shared_ptr<QPDFJob::AttConfig> c_att;
        std::shared_ptr<QPDFJob::PagesConfig> c_pages;
        std::shared_ptr<QPDFJob::UOConfig> c_uo;
        std::shared_ptr<QPDFJob::EncConfig> c_enc;
    };
}

Handlers::Handlers(std::shared_ptr<QPDFJob::Config> c_main) :
    c_main(c_main)
{
    initHandlers();
}

void
Handlers::initHandlers()
{
//#       include <qpdf/auto_job_json_init.hh>
    jh.addDictHandlers(
        [](std::string const&){},
        [this](std::string const&){c_main->checkConfiguration();});

    auto input = std::make_shared<JSONHandler>();
    auto input_file = std::make_shared<JSONHandler>();
    auto input_file_name = std::make_shared<JSONHandler>();
    auto output = std::make_shared<JSONHandler>();
    auto output_file = std::make_shared<JSONHandler>();
    auto output_file_name = std::make_shared<JSONHandler>();
    auto output_options = std::make_shared<JSONHandler>();
    auto output_options_qdf = std::make_shared<JSONHandler>();

    input->addDictHandlers(
        [](std::string const&){},
        [](std::string const&){});
    input_file->addDictHandlers(
        [](std::string const&){},
        [](std::string const&){});
    output->addDictHandlers(
        [](std::string const&){},
        [](std::string const&){});
    output_file->addDictHandlers(
        [](std::string const&){},
        [](std::string const&){});
    output_options->addDictHandlers(
        [](std::string const&){},
        [](std::string const&){});

    jh.addDictKeyHandler("input", input);
    input->addDictKeyHandler("file", input_file);
    input_file->addDictKeyHandler("name", input_file_name);
    jh.addDictKeyHandler("output", output);
    output->addDictKeyHandler("file", output_file);
    output_file->addDictKeyHandler("name", output_file_name);
    output->addDictKeyHandler("options", output_options);
    output_options->addDictKeyHandler("qdf", output_options_qdf);

    input_file_name->addStringHandler(
        [this](std::string const&, std::string const& v) {
            c_main->inputFile(v.c_str());
        });
    output_file_name->addStringHandler(
        [this](std::string const&, std::string const& v) {
            c_main->outputFile(v.c_str());
        });
    output_options_qdf->addBoolHandler(
        [this](std::string const&, bool v) {
            // QXXXQ require v to be true
            c_main->qdf();
        });
}

void
Handlers::handle(JSON& j)
{
    jh.handle(".", j);
}

void
QPDFJob::initializeFromJson(std::string const& json)
{
    std::list<std::string> errors;
    JSON j = JSON::parse(json);
    if (! j.checkSchema(JOB_SCHEMA, JSON::f_optional, errors))
    {
        std::ostringstream msg;
        msg << this->m->message_prefix
            << ": job json has errors:";
        for (auto const& error: errors)
        {
            msg << std::endl << "  " << error;
        }
        throw std::runtime_error(msg.str());
    }

    Handlers(config()).handle(j);
}