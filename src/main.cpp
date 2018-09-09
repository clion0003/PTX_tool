#include <iostream>
#include <string>
#include <PTX_tool.h>

using namespace std;

int main(int argc, char** argv) {
    ptx_tool p1;
    /*
    if (argc != 5){
        cout << "Error: argc = " << argc << endl;
        cout << "Argument : output_file  input_file  prototxt  PTX_code_path" << endl;
        cout << "Example: ./cmake-build-debug/bin/PTX_tool  resource/_cuobjdump_new  "
                "resource/_cuobjdump_complete_output_conv_convcu resource/vgg16.prototxt  "
                "resource/PTX_code" << endl;
        return -1;
    }*/
    //p1.ptx_replace(argv[1], argv[2], argv[3], argv[4]);
    if (argc != 5) {
        cout << "Error: argc = " << argc << endl;
        cout << "Argument : output_file  input_file  prototxt  PTX_code_path" << endl;
        cout << "Example: ./cmake-build-debug/bin/PTX_tool  resource/logcaffe  "
                "resource/logcaffe_withoutend  resource/vgg16.prototxt  "
                "resource/PTX_code" << endl;
        return -1;
    }

    p1.ptx_generate(argv[1],argv[2],argv[3],argv[4]);
    return 0;
}