#include "rewrite_read_groups.h"

#include <api/BamReader.h>
#include <api/BamWriter.h>

#include <toolkit/bamtools_tool.h>
#include <utils/bamtools_options.h>

#include <fstream>
#include <iostream>
#include <string>
using namespace std;
using namespace BamTools;
const std::string DEFAULT_MAPPING_FILE = "mapping.csv";  // string representation of stdout

struct RewriteReadGroupsTool::RewriteReadGroupsToolSettings{
    bool has_input_file_name;
    std::string input_file;
    bool has_output_file_name;
    std::string output_file; 
    std::string mapping_file;
    bool has_mapping_file;
    RewriteReadGroupsToolSettings():has_input_file_name(false), has_output_file_name(false){}
};

class RewriteReadGroupsTool::RewriteReadGroupsToolPrivate{
    public:
        RewriteReadGroupsToolPrivate(RewriteReadGroupsTool::RewriteReadGroupsToolSettings* settings):m_settings(settings){}
        ~RewriteReadGroupsToolPrivate(){
            m_reader.Close();
            m_writer.Close();
        }

        bool Run();

    private:
        std::map<string, string> rg_translation_dictionary;
        void ParseMappingFile();
        BamReader m_reader;
        BamWriter m_writer;
        RewriteReadGroupsTool::RewriteReadGroupsToolSettings* m_settings;
        const string default_record = "Unrecognized";
};

void RewriteReadGroupsTool::RewriteReadGroupsToolPrivate::ParseMappingFile(){
    ifstream infile;
    infile.open(m_settings -> mapping_file);
    string cell; 
    string original_rg, new_rg; 
    string line;
    int count = 0 ;
    while (std::getline(infile, line)){
        int idx = line.find(',');
        original_rg = line.substr(0,idx);
        new_rg = line.substr(idx+1);
        rg_translation_dictionary.insert(std::make_pair(original_rg, new_rg));
        count ++ ;
    }

    cout << "Read " << count << " lines from the mapping csv" << endl;
    infile.close();

    m_reader.Open(m_settings -> input_file);
    m_writer.Open(m_settings -> output_file, m_reader.GetHeader(), m_reader.GetReferenceData());
    BamAlignment alignment;
    count = 0; 
    int unknown = 0; 
    while( m_reader.GetNextAlignment( alignment )){
        if ( count % 1000 == 0){
            cout << "Read " << count << " records " << flush << "\r"; 
        }
        count ++; 
        string original_rg;
        string new_rg; 
        string fb, fc; 
        if (alignment.HasTag("RG")){
            alignment.GetTag("RG", original_rg);
        } else {
            alignment.GetTag("fb", fb);
            alignment.GetTag("fc", fc);
            original_rg = fb + "-" + fc;
        }

        if (rg_translation_dictionary.find(original_rg) == rg_translation_dictionary.end()){
            new_rg = default_record; 
            unknown ++; 
        } else {
            new_rg = rg_translation_dictionary.at(original_rg);
        }
        alignment.EditTag("RG","Z",new_rg);
        m_writer.SaveAlignment(alignment);
    }
    cout << endl; 
    cout << "RG renaming done, wrote " << count << " records" << endl;
    cout << unknown << " records had unrecognized read group" << endl; 

}

bool RewriteReadGroupsTool::RewriteReadGroupsToolPrivate::Run()
{
    if (!(m_settings -> has_input_file_name)){
        m_settings -> input_file = Options::StandardIn();
    }
    if (!(m_settings -> has_output_file_name)){
        m_settings -> output_file = Options::StandardOut();
    }
    if (!(m_settings -> has_mapping_file)){
        m_settings -> mapping_file = DEFAULT_MAPPING_FILE;
    }

    ParseMappingFile();
    // this does a single pass, chunking up the input file into smaller sorted temp files,
    // then write out using BamMultiReader to handle merging
    return false;
}

RewriteReadGroupsTool::RewriteReadGroupsTool()
    : AbstractTool()
    , m_settings(new RewriteReadGroupsToolSettings)
    , m_impl(0)
{
    // set program details
    const std::string name = "bamtools rewrite_read_groups";
    const std::string description =
        "replaces read groups with other names";
    const std::string args =
        "-in <filename> -out <filename> -csv <barcode -> sample mapping in csv file";

    Options::SetProgramInfo(name, description, args);

    // set up options
    OptionGroup* IO_Opts = Options::CreateOptionGroup("Input & Output");
    Options::AddValueOption("-in", "string", "the input BAM file", "",
                            m_settings->has_input_file_name, m_settings->input_file, IO_Opts,
                            Options::StandardIn());
    Options::AddValueOption("-out", "string",  "the output BAM file", "", 
                            m_settings->has_output_file_name,
                            m_settings->output_file, IO_Opts,
                            Options::StandardOut());
    Options::AddValueOption("-csv", "string", "mapping between old and new read groups", "",
                             m_settings -> has_mapping_file, m_settings->mapping_file, IO_Opts,
                            DEFAULT_MAPPING_FILE);
    }

RewriteReadGroupsTool::~RewriteReadGroupsTool()
{

    delete m_settings;
    m_settings = 0;

    delete m_impl;
    m_impl = 0;
}

int RewriteReadGroupsTool::Help()
{
    Options::DisplayHelp();
    return 0;
}

int RewriteReadGroupsTool::Run(int argc, char* argv[])
{

    // parse command line arguments
    Options::Parse(argc, argv, 1);

    // initialize SplitTool with settings
    m_impl = new RewriteReadGroupsToolPrivate(m_settings);

    // run SplitTool, return success/fail
    if (m_impl->Run()) {
        return 0;
    } else {
        return 1;
    }
}
