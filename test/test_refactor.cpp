#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <cmath>
#include <bitset>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.hpp"
#include "Refactor/Refactor.hpp"

using namespace std;

template <class T, class Refactor>
void evaluate(const vector<T>& data, const vector<uint32_t>& dims, int target_level, int num_bitplanes, Refactor refactor){
    struct timespec start, end;
    int err = 0;
    //cout << "Start eval" << endl;
    err = clock_gettime(CLOCK_REALTIME, &start);
    refactor.refactor(data.data(), dims, target_level, num_bitplanes);
    err = clock_gettime(CLOCK_REALTIME, &end);
    //cout << "Refactor time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
}

template <class T, class Decomposer, class Interleaver, class Encoder, class Compressor, class ErrorCollector, class Writer>
void test(string filename, const vector<uint32_t>& dims, int target_level, int num_bitplanes, Decomposer decomposer, Interleaver interleaver, Encoder encoder, Compressor compressor, ErrorCollector collector, Writer writer){
    auto refactor = MDR::ComposedRefactor<T, Decomposer, Interleaver, Encoder, Compressor, ErrorCollector, Writer>(decomposer, interleaver, encoder, compressor, collector, writer);
    size_t num_elements = 0;
    auto data = MGARD::readfile<T>(filename.c_str(), num_elements);
    //cout << "begin eval" << std::endl;
    evaluate(data, dims, target_level, num_bitplanes, refactor);
    //cout << "end of eval<float>" << endl;
}

int main(int argc, char ** argv){

    int argv_id = 1;
    string filename = string(argv[argv_id ++]);
    int target_level = atoi(argv[argv_id ++]);
    int num_bitplanes = atoi(argv[argv_id ++]);
    if(num_bitplanes % 2 == 1) {
        num_bitplanes += 1;
        std::cout << "Change to " << num_bitplanes + 1 << " bitplanes for simplicity of negabinary encoding" << std::endl;
    }
    int num_dims = atoi(argv[argv_id ++]);
    //std::cout << num_dims << std::endl;
    vector<uint32_t> dims(num_dims, 0);
    for(int i=0; i<num_dims; i++){
        dims[i] = atoi(argv[argv_id ++]);
	//std::cout << dims[i] << std::endl;
    }
    
    // ----------- extract foldername --------------------
    int length = filename.length();
    char* foldername = new char[length + 1];
    strcpy(foldername, filename.c_str());
    //std::cout << foldername << std::endl;

    // loop through the string to extract all other tokens
    char * token_ = strtok(foldername, "/");
    while( foldername != NULL ) {
      token_ = foldername;
      //std::cout << foldername << std::endl;
      foldername = strtok(NULL, "/");
    }
    //std::cout << token_ << std::endl;

    // foldername is token_ now
    strtok(token_, ".");
    //std::cout << string(token_) + "_" << std::endl;
    std::string token_str = "_" + string(token_);

    token_ = new char[token_str.length()];
    strcpy(token_, token_str.c_str());

    struct stat st = {0};
    if (stat(token_, &st) == -1) {
            mkdir(token_, 0700);
    }
    // ---------------------------------------------------

    //string metadata_file = "refactored_data/metadata.bin";
    string metadata_file = string(token_) + "/metadata.bin";
    vector<string> files;
    for(int i=0; i<=target_level; i++){
        //string filename = "refactored_data/level_" + to_string(i) + ".bin";
        string filename = string(token_) + "/level_" + to_string(i) + ".bin";
        files.push_back(filename);
	//std::cout << filename << std::endl;
    }

    using T = float;

    using T_stream = uint32_t;
    //if(num_bitplanes > 32){
    //    num_bitplanes = 32;
    //    std::cout << "Only less than 32 bitplanes are supported for single-precision floating point" << std::endl;
    //}

    auto decomposer = MDR::MGARDOrthoganalDecomposer<T>();
    // auto decomposer = MDR::MGARDHierarchicalDecomposer<T>();
    auto interleaver = MDR::DirectInterleaver<T>();
    // auto interleaver = MDR::SFCInterleaver<T>();
    // auto interleaver = MDR::BlockedInterleaver<T>();
    // auto encoder = MDR::GroupedBPEncoder<T, T_stream>();
    auto encoder = MDR::NegaBinaryBPEncoder<T, T_stream>();
    // auto encoder = MDR::PerBitBPEncoder<T, T_stream>();
    // auto compressor = MDR::DefaultLevelCompressor();
    auto compressor = MDR::AdaptiveLevelCompressor(32);
    // auto compressor = MDR::NullLevelCompressor();
    //auto collector = MDR::SquaredErrorCollector<T>();
    auto collector = MDR::MaxErrorCollector<T>();
    auto writer = MDR::ConcatLevelFileWriter(metadata_file, files);
    // auto writer = MDR::HPSSFileWriter(metadata_file, files, 2048, 512 * 1024 * 1024);

    //std::cout << "begin test" << std::endl;
    test<T>(filename, dims, target_level, num_bitplanes, decomposer, interleaver, encoder, compressor, collector, writer);
    //std::cout << "end test" << std::endl;
    return 0;
}
