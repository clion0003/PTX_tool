#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <exception>
#include <unordered_map>
//#include <mstch/mstch.hpp>
#include <boost/filesystem.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <fcntl.h>
#include "caffe.pb.h"
using namespace caffe;
using namespace std;

using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::Message;

#include "PTX_tool.h"

bool ReadProtoFromTextFile(const char* filename, Message* proto) {
    int fd = open(filename, O_RDONLY);
    FileInputStream* input = new FileInputStream(fd);
    bool success = google::protobuf::TextFormat::Parse(input, proto);
    delete input;
    close(fd);
    return success;
}

string endcode = R"xxx(
Fatbin elf code:
================
arch = sm_61
code version = [1,7]
producer = cuda
host = linux
compile_size = 64bit
)xxx";

/*Replace all float type conv layer entry in input_file with PTX_code files in PTX_code_path
  and print in output_file*/
void ptx_tool::ptx_replace(string output_file, string input_file, string prototxt_file, string PTX_code_path)
{
    fstream ifile, ofile, ptx_file;
    string file_name, str, ptx_code;
    try {
        ifile.open(input_file);
        ofile.open(output_file);
        if (!ifile) throw input_file;
        if (!ofile) throw output_file;

        /*Parse the prototxt*/
        NetParameter CNN;
        if(!ReadProtoFromTextFile(prototxt_file.c_str(), &CNN))
        {
            throw prototxt_file;
        }
        /*Record conv layers*/
        int conv_index[CNN.layer_size()];
        int i = 0;
        int size = 0;
        for (i = 0; i < CNN.layer_size(); i++){
            if (!strcmp(CNN.layer(i).type().c_str(),"Convolution")){
                conv_index[size] = i;
                size++;
            }
        }
        /*Regular expression for entry*/
        string regex_entry;

        /*for all conv layers*/
        for (i = 0; i < size; i++){
            /*Generate corresponding regex*/
            regex_entry = ".*entry _Z7" + CNN.layer((conv_index[i])).name() + "If.*";
            regex re(regex_entry);
            /*print in output file while matching the regex*/
            while (getline(ifile, str) and !regex_match(str, re)){
                ofile << str << endl;
            }
            ofile << str << endl;
            /*get to the beginning of the entry*/
            while (getline(ifile, str) and strcmp(str.c_str(),"{")){
                ofile << str << endl;
            }
            ofile << str << endl;
            /*read the PTX_code and write in the code*/
            file_name = PTX_code_path + "/" + CNN.layer((conv_index[i])).name() + ".ptx";
            ptx_file.open(file_name);						//open PTX file
            if (!ptx_file)
            {
                ptx_file.close();
                throw file_name;
            }
            while (getline(ptx_file, ptx_code))
            {
                ofile << ptx_code << endl;			//write the new PTX code
            }
            /*get to the end of the entry in the input file*/
            while (getline(ifile, str) and strcmp(str.c_str(),"}")){}
            ofile << str << endl;
            ptx_file.close();
        }
        while (getline(ifile, str)){
            ofile << str << endl;
        }
        cout << output_file << " generated" <<endl;
        ifile.close();
        ofile.close();
    }
    catch (char* a) {
        cout << "Can't open file: " << a << "!" << endl;
        ifile.close();
        ofile.close();
    }
    catch (string a) {
        cout << "Can't open file: " << a << "!" << endl;
        ifile.close();
        ofile.close();
    }
}

/*Generate ptx entries and insert the PTX code defined in prototxt_file at PTX_code_path
 * and append the input_file*/
void ptx_tool::ptx_generate(string output_file, string input_file, string prototxt_file, string PTX_code_path) {
    string command = "cp " + input_file + " " + output_file ;
    system(command.c_str());
    fstream ofile, ptx_file;
    string file_name, str, ptx_code;
    try {
        ofile.open(output_file, ios::app);
        if (!ofile) throw output_file;
        /*Parse the prototxt*/
        NetParameter CNN;
        if(!ReadProtoFromTextFile(prototxt_file.c_str(), &CNN))
        {
            throw prototxt_file;
        }
        /*Record conv layers*/
        int conv_index[CNN.layer_size()];
        int i = 0;
        int size = 0;
        for (i = 0; i < CNN.layer_size(); i++){
            if (!strcmp(CNN.layer(i).type().c_str(),"Convolution")){
                conv_index[size] = i;
                size++;
            }
        }
        /*for all conv layers*/
        for (i = 0; i < size; i++){
            string name = CNN.layer(conv_index[i]).name();
            /*float*/
            ofile << ".visible .entry _Z7" + name+ "IfEvPT_PKS0_S3_S3_(" << endl;
            ofile << ".param .u64 _Z7" + name + "IfEvPT_PKS0_S3_S3__param_0," << endl;
            ofile << ".param .u64 _Z7" + name + "IfEvPT_PKS0_S3_S3__param_1," << endl;
            ofile << ".param .u64 _Z7" + name + "IfEvPT_PKS0_S3_S3__param_2," << endl;
            ofile << ".param .u64 _Z7" + name + "IfEvPT_PKS0_S3_S3__param_3" << endl;
            ofile << ")" << endl;
            ofile << "{" << endl;
            /*read the PTX_code and write in the code*/
            file_name = PTX_code_path + "/" + name + ".ptx";
            ptx_file.open(file_name);						//open PTX file
            if (!ptx_file)
            {
                ptx_file.close();
                throw file_name;
            }
            while (getline(ptx_file, ptx_code))
            {
                ofile << ptx_code << endl;			//write the new PTX code
            }
            ofile << "}" <<endl;
            ofile << endl;
            ptx_file.close();

            /*double*/
            ofile << ".visible .entry _Z7" + name+ "IdEvPT_PKS0_S3_S3_(" << endl;
            ofile << ".param .u64 _Z7" + name + "IdEvPT_PKS0_S3_S3__param_0," << endl;
            ofile << ".param .u64 _Z7" + name + "IdEvPT_PKS0_S3_S3__param_1," << endl;
            ofile << ".param .u64 _Z7" + name + "IdEvPT_PKS0_S3_S3__param_2," << endl;
            ofile << ".param .u64 _Z7" + name + "IdEvPT_PKS0_S3_S3__param_3" << endl;
            ofile << ")" << endl;
            ofile << "{" << endl;
            ofile << "ret;" << endl;
            ofile << "}" << endl;
            ofile << endl;
        }
        ofile << endcode;
        cout << output_file << " generated" <<endl;
        ofile.close();
    }
    catch (char* a) {
        cout << "Can't open file: " << a << "!" << endl;
        ofile.close();
    }
    catch (string a) {
        cout << "Can't open file: " << a << "!" << endl;
        ofile.close();
    }
}
