#include <iostream>
#include <string>
using namespace std;
#ifndef PTX_TOOL
#define PTX_TOOL
class ptx_tool{
public:
	/*Replace all float type conv layer entry in input_file with PTX_code files in PTX_code_path
	  and print in output_file*/
	void ptx_replace(string output_file, string input_file, string prototxt_file, string PTX_code_path);

    void ptx_generate(string output_file, string input_file, string prototxt_file, string PTX_code_path);
};

extern string endcode;

#endif // PTX_TOOL
