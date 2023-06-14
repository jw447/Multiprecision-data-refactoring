#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <cmath>
#include <bitset>
#include "utils.hpp"
#include "Reconstructor/Reconstructor.hpp"
// #include "evaluate.hpp"

using namespace std;

vector<double> cc(5, 1);

template <class T>
void print_statistics(const T * data_ori, const T * data_dec, size_t data_size){
    double max_val = data_ori[0];
    double min_val = data_ori[0];
    double max_abs = fabs(data_ori[0]);
    for(int i=0; i<data_size; i++){
        if(data_ori[i] > max_val) max_val = data_ori[i];
        if(data_ori[i] < min_val) min_val = data_ori[i];
        if(fabs(data_ori[i]) > max_abs) max_abs = fabs(data_ori[i]);
    }
    double max_err = 0;
    int pos = 0;
    double mse = 0;
    for(int i=0; i<data_size; i++){
        double err = data_ori[i] - data_dec[i];
        mse += err * err;
        if(fabs(err) > max_err){
            pos = i;
            max_err = fabs(err);
        }
    }
    mse /= data_size;
    double psnr = 20 * log10((max_val - min_val) / sqrt(mse));
    cout << "MaxErr," << max_err << "," << "MSE," << mse << ",PSNR," << psnr << endl;
}

template <class T, class Reconstructor>
void evaluate(const vector<T>& data, const vector<double>& tolerance, Reconstructor reconstructor){
    struct timespec start, end;
    int err = 0;
    // auto a1 = compute_average(data.data(), dims[0], dims[1], dims[2], 3);
    // auto a12 = compute_average(data.data(), dims[0], dims[1], dims[2], 5);
    for(int i=0; i<tolerance.size(); i++){
        //cout << "Start reconstruction" << endl;
        err = clock_gettime(CLOCK_REALTIME, &start);
        auto reconstructed_data = reconstructor.progressive_reconstruct(tolerance[i]);
        err = clock_gettime(CLOCK_REALTIME, &end);
        //cout << "Reconstruct time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
        auto dims = reconstructor.get_dimensions();
        //MGARD::print_statistics(data.data(), reconstructed_data, data.size());
        print_statistics<T>(data.data(), reconstructed_data, data.size());
        // COMP_UTILS::evaluate_gradients(data.data(), reconstructed_data, dims[0], dims[1], dims[2]);
        // COMP_UTILS::evaluate_average(data.data(), reconstructed_data, dims[0], dims[1], dims[2], 0);
    }
}

template <class T, class Decomposer, class Interleaver, class Encoder, class Compressor, class ErrorEstimator, class SizeInterpreter, class Retriever>
void test(string filename, const vector<double>& tolerance, Decomposer decomposer, Interleaver interleaver, Encoder encoder, Compressor compressor, ErrorEstimator estimator, SizeInterpreter interpreter, Retriever retriever){
    auto reconstructor = MDR::ComposedReconstructor<T, Decomposer, Interleaver, Encoder, Compressor, SizeInterpreter, ErrorEstimator, Retriever>(decomposer, interleaver, encoder, compressor, interpreter, retriever);
    //cout << "loading metadata" << endl;
    reconstructor.load_metadata();

    size_t num_elements = 0;
    auto data = MGARD::readfile<T>(filename.c_str(), num_elements);
    evaluate(data, tolerance, reconstructor);
}

int main(int argc, char ** argv){

    int argv_id = 1;
    string filename = string(argv[argv_id ++]);
    int error_mode = atoi(argv[argv_id++]);
    int num_tolerance = atoi(argv[argv_id ++]);
    vector<double> tolerance(num_tolerance, 0);
    for(int i=0; i<num_tolerance; i++){
        tolerance[i] = atof(argv[argv_id ++]);    
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
    // ---------------------------------------------------

    double s = atof(argv[argv_id ++]);

    string metadata_file = string(token_) + "/metadata.bin";
    int num_levels = 0;
    int num_dims = 0;
    {
        // metadata interpreter, otherwise information needs to be provided
        size_t num_bytes = 0;
        auto metadata = MGARD::readfile<uint8_t>(metadata_file.c_str(), num_bytes);
        assert(num_bytes > num_dims * sizeof(uint32_t) + 2);
        num_dims = metadata[0];
        num_levels = metadata[num_dims * sizeof(uint32_t) + 1];
        //cout << "number of dimension = " << num_dims << ", number of levels = " << num_levels << endl;
    }

    cc[0] = 1.0 + 21.0*sqrt(3)/8;
    cc[1] = 1.0 + 21.0*sqrt(3)/8;
    cc[2] = 1.0 + 21.0*sqrt(3)/8;
    cc[3] = 1.0 + 21.0*sqrt(3)/8;
    cc[4] = 1.0 + 21.0*sqrt(3)/8;
    for(int i=0; i<num_levels; i++){
        cc[i] = atof(argv[argv_id ++]);
    }
    
    vector<string> files;
    for(int i=0; i<num_levels; i++){
        string filename = string(token_) + "/level_" + to_string(i) + ".bin";
        files.push_back(filename);
    }

    using T = float;
    using T_stream = uint32_t;
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
    auto retriever = MDR::ConcatLevelFileRetriever(metadata_file, files);
    switch(error_mode){
        case 1:{
            auto estimator = MDR::SNormErrorEstimator<T>(num_dims, num_levels - 1, s);
            // auto interpreter = MDR::SignExcludeGreedyBasedSizeInterpreter<MDR::SNormErrorEstimator<T>>(estimator);
            auto interpreter = MDR::NegaBinaryGreedyBasedSizeInterpreter<MDR::SNormErrorEstimator<T>>(estimator);
            // auto interpreter = MDR::RoundRobinSizeInterpreter<MDR::SNormErrorEstimator<T>>(estimator);
            // auto interpreter = MDR::InorderSizeInterpreter<MDR::SNormErrorEstimator<T>>(estimator);
            // auto estimator = MDR::L2ErrorEstimator_HB<T>(num_dims, num_levels - 1);
            // auto interpreter = MDR::SignExcludeGreedyBasedSizeInterpreter<MDR::L2ErrorEstimator_HB<T>>(estimator);
            test<T>(filename, tolerance, decomposer, interleaver, encoder, compressor, estimator, interpreter, retriever);            
            break;
        }
        default:{ // max error
            auto estimator = MDR::MaxErrorEstimatorOB<T>(num_dims);
            auto interpreter = MDR::SignExcludeGreedyBasedSizeInterpreter<MDR::MaxErrorEstimatorOB<T>>(estimator);
            // auto interpreter = MDR::RoundRobinSizeInterpreter<MDR::MaxErrorEstimatorOB<T>>(estimator);
            // auto interpreter = MDR::InorderSizeInterpreter<MDR::MaxErrorEstimatorOB<T>>(estimator);
            // auto estimator = MDR::MaxErrorEstimatorHB<T>();
            // auto interpreter = MDR::SignExcludeGreedyBasedSizeInterpreter<MDR::MaxErrorEstimatorHB<T>>(estimator);
            test<T>(filename, tolerance, decomposer, interleaver, encoder, compressor, estimator, interpreter, retriever);
        }
    }    
    return 0;
}
