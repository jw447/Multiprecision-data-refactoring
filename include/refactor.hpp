#ifndef _REFACTOR_HPP
#define _REFACTOR_HPP

#include <vector>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <bitset>
#include <iomanip>
#include "utils.hpp"
#include "data_org.hpp"
#include "encode.hpp"
#include "decode.hpp"
#include "error_est.hpp"
#include "decompose.hpp"
#include "recompose.hpp"
#include "io_utils.hpp"
#include "lossless.hpp"

using namespace std;
using namespace LOSSLESS;
using namespace MGARD;

namespace REFACTOR{

// size of segment: default 4 MB
const int seg_size = 4;

#define LOSSLESS_THRESHOLD 2000
// metadata for refactored data
template <class T>
class Metadata{
public:
    Metadata(){}
    Metadata(int target_level){
        level_elements = vector<size_t>(target_level + 1);
        level_error_bounds = vector<T>(target_level + 1);
    }
    vector<size_t> dims;              // dimensions of original data
    vector<size_t> level_elements;    // number of elements in each multigrid level
    vector<T> level_error_bounds;     // max errors in each multigrid level
    vector<vector<size_t>> component_sizes; // size of each component in each level
    vector<vector<double>> max_e;         // max_e[i][j]: max error of level i using the first (j+1) bit-planes
    vector<vector<double>> mse;         // mse[i][j]: mse of level i using the first (j+1) bit-planes
    vector<vector<uint8_t>> bitplane_indictors;   // indicator for bitplane encoding
    vector<vector<uint8_t>> lossless_indicators;   // indicator for bitplane lossless compression
    vector<int> order;                  // order of bitplane placement
    int mode = 0;                       // mode for error estimation
    int data_reorganization = IN_ORDER;   // whether to enable reorder of data
    bool max_e_estimator = false;
    bool mse_estimator = false;
    int option = 0;
    int encoded_bitplanes = 32;
    T max_val = 0;                  // max value in original data
    T min_val = 0;                  // min value in original data
    size_t total_encoded_size = 0;  // total encoded size

    void init_encoded_sizes(){
        component_sizes.clear();
        for(int i=0; i<level_elements.size(); i++){
            component_sizes.push_back(vector<size_t>());
        }
    }
    void init_bitplane_indicators(){
        bitplane_indictors.clear();
        for(int i=0; i<level_elements.size(); i++){
            bitplane_indictors.push_back(vector<uint8_t>());
        }        
    }
    void init_lossless_indicators(){
        lossless_indicators.clear();
        for(int i=0; i<level_elements.size(); i++){
            lossless_indicators.push_back(vector<uint8_t>());
        }
    }
    size_t size() const{
        size_t metadata_size = 
                sizeof(int) + sizeof(int) + sizeof(size_t)  // option + encoded_bitplanes + number of levels
                + level_elements.size() * sizeof(size_t)    // level_elements
                + level_error_bounds.size() * sizeof(T)     // level_eb
                + sizeof(size_t) + dims.size() * sizeof(size_t) // dimensions
                + sizeof(uint8_t) + sizeof(uint8_t) // estimator flags
                + sizeof(size_t) + order.size() * sizeof(int) // order
                + sizeof(int) + sizeof(int)                 // mode + reorganization
                + sizeof(T) + sizeof(T) + sizeof(size_t)    // max_val, min_val, total size
                ;
        for(int i=0; i<component_sizes.size(); i++){
            metadata_size += sizeof(size_t) + component_sizes[i].size() * sizeof(size_t);
        }
        for(int i=0; i<bitplane_indictors.size(); i++){
            metadata_size += sizeof(size_t) + bitplane_indictors[i].size() * sizeof(uint8_t);
        }
        for(int i=0; i<lossless_indicators.size(); i++){
            metadata_size += sizeof(size_t) + lossless_indicators[i].size() * sizeof(uint8_t);
        }
        if(max_e_estimator) {
            for(int i=0; i<max_e.size(); i++){
                metadata_size += sizeof(size_t) + max_e[i].size() * sizeof(double);
            }
        }
        if(mse_estimator) {
            for(int i=0; i<mse.size(); i++){
                metadata_size += sizeof(size_t) + mse[i].size() * sizeof(double);
            }
        }
        return metadata_size;
    }
    template <class T1>
    size_t serialize_level_vectors(const vector<vector<T1>>& level_vecs, uint8_t * buffer_pos) const{
        uint8_t const * const start = buffer_pos;
        for(int i=0; i<level_vecs.size(); i++){
            *reinterpret_cast<size_t*>(buffer_pos) = level_vecs[i].size();
            buffer_pos += sizeof(size_t);
            memcpy(buffer_pos, level_vecs[i].data(), level_vecs[i].size() * sizeof(T1));
            buffer_pos += level_vecs[i].size() * sizeof(T1);
        }
        return buffer_pos - start;
    }
    uint8_t * serialize() const{
        uint8_t * buffer = (uint8_t *) malloc(size());
        uint8_t * buffer_pos = buffer;
        *reinterpret_cast<int*>(buffer_pos) = option;
        buffer_pos += sizeof(int);
        *reinterpret_cast<int*>(buffer_pos) = encoded_bitplanes;
        buffer_pos += sizeof(int);
        size_t num_levels = level_elements.size();
        *reinterpret_cast<size_t*>(buffer_pos) = num_levels;
        buffer_pos += sizeof(size_t);
        memcpy(buffer_pos, level_elements.data(), num_levels * sizeof(size_t));
        buffer_pos += num_levels * sizeof(size_t);
        memcpy(buffer_pos, level_error_bounds.data(), num_levels * sizeof(T));
        buffer_pos += num_levels * sizeof(T);
        *reinterpret_cast<size_t*>(buffer_pos) = dims.size();
        buffer_pos += sizeof(size_t);
        memcpy(buffer_pos, dims.data(), dims.size() * sizeof(size_t));
        buffer_pos += dims.size() * sizeof(size_t);
        *reinterpret_cast<size_t*>(buffer_pos) = order.size();
        buffer_pos += sizeof(size_t);
        memcpy(buffer_pos, order.data(), order.size() * sizeof(int));
        buffer_pos += order.size() * sizeof(int);
        *reinterpret_cast<int*>(buffer_pos) = mode;
        buffer_pos += sizeof(int);
        *reinterpret_cast<int*>(buffer_pos) = data_reorganization;
        buffer_pos += sizeof(int);
        *reinterpret_cast<T*>(buffer_pos) = max_val;
        buffer_pos += sizeof(T);
        *reinterpret_cast<T*>(buffer_pos) = min_val;
        buffer_pos += sizeof(T);
        *reinterpret_cast<size_t*>(buffer_pos) = total_encoded_size;
        buffer_pos += sizeof(size_t);
        *buffer_pos = mse_estimator;
        buffer_pos += sizeof(uint8_t);
        *buffer_pos = max_e_estimator;
        buffer_pos += sizeof(uint8_t);
        buffer_pos += serialize_level_vectors(component_sizes, buffer_pos);
        buffer_pos += serialize_level_vectors(bitplane_indictors, buffer_pos);
        buffer_pos += serialize_level_vectors(lossless_indicators, buffer_pos);
        if(max_e_estimator){
            buffer_pos += serialize_level_vectors(max_e, buffer_pos);
        }
        if(mse_estimator){
            buffer_pos += serialize_level_vectors(mse, buffer_pos);
        }
        return buffer;
    }
    // auto increment buffer_pos
    template <class T1>
    vector<vector<T1>> deserialize_level_vectors(const uint8_t *& buffer_pos, size_t num_levels){
        vector<vector<T1>> level_vecs;
        for(int i=0; i<num_levels; i++){
            size_t num = *reinterpret_cast<const size_t*>(buffer_pos);
            buffer_pos += sizeof(size_t);
            vector<T1> level_vec = vector<T1>(reinterpret_cast<const T1 *>(buffer_pos), reinterpret_cast<const T1 *>(buffer_pos) + num);
            level_vecs.push_back(level_vec);
            buffer_pos += num * sizeof(T1);
        }
        return level_vecs;
    }
    void deserialize(const uint8_t * serialized_data){
        const uint8_t * buffer_pos = serialized_data;
        option = *reinterpret_cast<const int*>(buffer_pos);
        buffer_pos += sizeof(int);
        encoded_bitplanes = *reinterpret_cast<const int*>(buffer_pos);
        buffer_pos += sizeof(int);
        size_t num_levels = *reinterpret_cast<const size_t*>(buffer_pos);
        buffer_pos += sizeof(size_t);
        level_elements = vector<size_t>(reinterpret_cast<const size_t *>(buffer_pos), reinterpret_cast<const size_t *>(buffer_pos) + num_levels);
        buffer_pos += num_levels * sizeof(size_t);
        level_error_bounds = vector<T>(reinterpret_cast<const T *>(buffer_pos), reinterpret_cast<const T *>(buffer_pos) + num_levels);
        buffer_pos += num_levels * sizeof(T);
        size_t num_dimensions = *reinterpret_cast<const size_t*>(buffer_pos);
        buffer_pos += sizeof(size_t);
        dims = vector<size_t>(reinterpret_cast<const size_t *>(buffer_pos), reinterpret_cast<const size_t *>(buffer_pos) + num_dimensions);
        buffer_pos += num_dimensions * sizeof(size_t);
        size_t order_size = *reinterpret_cast<const size_t*>(buffer_pos);
        buffer_pos += sizeof(size_t);
        order = vector<int>(reinterpret_cast<const int *>(buffer_pos), reinterpret_cast<const int *>(buffer_pos) + order_size);
        buffer_pos += order_size * sizeof(int);
        mode = *reinterpret_cast<const int*>(buffer_pos);
        buffer_pos += sizeof(int);
        data_reorganization = *reinterpret_cast<const int*>(buffer_pos);
        buffer_pos += sizeof(int);
        max_val = *reinterpret_cast<const T*>(buffer_pos);
        buffer_pos += sizeof(T);
        min_val = *reinterpret_cast<const T*>(buffer_pos);
        buffer_pos += sizeof(T);
        total_encoded_size = *reinterpret_cast<const size_t*>(buffer_pos);
        buffer_pos += sizeof(size_t);
        mse_estimator = *buffer_pos;
        buffer_pos += sizeof(uint8_t);
        max_e_estimator = *buffer_pos;
        buffer_pos += sizeof(uint8_t);
        // deserialize_level_vectors has auto increment for buffer_pos
        component_sizes = deserialize_level_vectors<size_t>(buffer_pos, num_levels);
        bitplane_indictors = deserialize_level_vectors<uint8_t>(buffer_pos, num_levels);
        lossless_indicators = deserialize_level_vectors<uint8_t>(buffer_pos, num_levels);
        if(max_e_estimator){
            // deserialize_level_vectors has auto increment for buffer_pos
            max_e = deserialize_level_vectors<double>(buffer_pos, num_levels);
        }
        if(mse_estimator){
            // deserialize_level_vectors has auto increment for buffer_pos
            mse = deserialize_level_vectors<double>(buffer_pos, num_levels);
        }
    }
    void to_file(const string& filename) const{
        auto serialized_data = serialize();
        writefile(filename.c_str(), serialized_data, size());
        free(serialized_data);
    }
    void from_file(const string& filename){
        size_t num_bytes = 0;
        uint8_t * buffer = readfile_pointer<uint8_t>(filename.c_str(), num_bytes);
        deserialize(buffer);
        free(buffer);
    }
    void set_mode(int mode_){
        mode = mode_;
        if(mode == MAX_ERROR){
            // enable both estimators for retrieval
            max_e_estimator = true;
            mse_estimator = true;
        }
        else if(mode == SQUARED_ERROR){
            max_e_estimator = false;
            mse_estimator = true; 
        }
        else{
            cerr << "set_mode: mode not supported! Exit." << endl;
            exit(0);
        }
    }
    const vector<vector<double>>& get_level_errors() const{
        if(mode == MAX_ERROR){
            return max_e;
        }
        else if(mode == SQUARED_ERROR){
            return mse;        
        }
        else{
            cerr << "get_level_errors: mode not supported! Exit." << endl;
            exit(0);
        }
    }
};

// interleave coeffcients in the finer level
/*
@params data: decomposed data
@params dims: original dims (to compute offset)
@params dims_fine: fine level dimensions
@params dims_coarse: coarse level dimensions
@params buffer: data buffer for level coefficients
@params max_err: max error in current level
*/
template <class T>
void interleave_level_coefficients(const T * data, const vector<size_t>& dims, const vector<size_t>& dims_fine, const vector<size_t>& dims_coasre, T * buffer){
    switch(dims.size()){
        case 3:
            // interleave_level_coefficients_3d(data, dims, dims_fine, dims_coasre, buffer);
            interleave_level_coefficients_3d_space_filling_curve(data, dims, dims_fine, dims_coasre, buffer);
            break;
        default:
            cerr << "Other dimensions are not supported" << endl;
    }
}

// put coeffcients in the finer level to the correct position
/*
@params buffer: data buffer for level coefficients
@params dims: original dims (to compute offset)
@params dims_fine: fine level dimensions
@params dims_coarse: coarse level dimensions
@params data: decomposed data
*/
template <class T>
void reposition_level_coefficients(const T * buffer, const vector<size_t>& dims, const vector<size_t>& dims_fine, const vector<size_t>& dims_coasre, T * data){
    switch(dims.size()){
        case 3:
            // reposition_level_coefficients_3d(buffer, dims, dims_fine, dims_coasre, data);
            reposition_level_coefficients_3d_space_filling_curve(buffer, dims, dims_fine, dims_coasre, data);
            break;
        default:
            cerr << "Other dimensions are not supported" << endl;
    }
}

// refactor level-centric decomposed data in hierarchical fashion
/*
@params data: decomposed data
@params target_level: decomposed level
@params dims: data dimensions
@params metadata: metadata object
*/
template <class T>
vector<vector<uint8_t*>> level_centric_data_refactor(const T * data, int target_level, const vector<size_t>& dims, Metadata<T>& metadata){
    const uint8_t index_size = log2(sizeof(T) * UINT8_BITS);
    int max_level = log2(*min_element(dims.begin(), dims.end()));
    if(target_level > max_level) target_level = max_level;
    auto level_dims = init_levels(dims, target_level);
    vector<size_t>& level_elements = metadata.level_elements;
    vector<T>& level_error_bounds = metadata.level_error_bounds;
    // compute level elements
    level_elements[0] = 1;
    for(int j=0; j<dims.size(); j++){
        level_elements[0] *= level_dims[0][j];
    }
    size_t pre_num_elements = level_elements[0];
    for(int i=1; i<=target_level; i++){
        size_t num_elements = 1;
        for(int j=0; j<dims.size(); j++){
            num_elements *= level_dims[i][j];
        }
        level_elements[i] = num_elements - pre_num_elements;
        pre_num_elements = num_elements;
    }
    // init bitplane indicators
    metadata.init_bitplane_indicators();
    metadata.init_lossless_indicators();
    // init metadata sizes recorder
    metadata.init_encoded_sizes();
    // record all level components
    vector<vector<uint8_t*>> level_components;
    vector<size_t> dims_dummy(dims.size(), 0);
    for(int i=0; i<=target_level; i++){
        // cout << "encoding level " << target_level - i << endl;
        const vector<size_t>& prev_dims = (i == 0) ? dims_dummy : level_dims[i - 1];
        uint8_t * buffer = (uint8_t *) malloc(level_elements[i] * sizeof(T));
        // extract components for each level
        interleave_level_coefficients(data, dims, level_dims[i], prev_dims, reinterpret_cast<T*>(buffer));
        // string outfile("decomposed_level_");
        // writefile((outfile + to_string(target_level - i) + ".dat").c_str(), reinterpret_cast<T*>(buffer), level_elements[i]);
        level_error_bounds[i] = record_level_max_value(reinterpret_cast<T*>(buffer), level_elements[i]);
        if(level_elements[i] * sizeof(T) < seg_size){
            if(metadata.max_e_estimator){
                auto level_max_e = vector<double>(1, 0);
                metadata.max_e.push_back(level_max_e);
            }
            if(metadata.mse_estimator){
                auto level_mse = vector<double>(1, 0);
                metadata.mse.push_back(level_mse);
            }
            vector<uint8_t*> tiny_level;
            tiny_level.push_back(buffer);
            level_components.push_back(tiny_level);
            metadata.component_sizes[i].push_back(level_elements[i] * sizeof(T));
        }
        else{
            vector<uint8_t>& level_lossless_indicators = metadata.lossless_indicators[i];
            level_lossless_indicators = vector<uint8_t>(metadata.encoded_bitplanes, false);
            // identify exponent of max element
            int level_exp = 0;
            frexp(level_error_bounds[i], &level_exp);
            cout << "level " << i << " max err = " << level_error_bounds[i] << ", exp = " << level_exp << endl;
            if(metadata.max_e_estimator){
                struct timespec start, end;
                int err = clock_gettime(CLOCK_REALTIME, &start);
                auto level_max_e = record_level_max_e(reinterpret_cast<T*>(buffer), level_elements[i], metadata.encoded_bitplanes, level_error_bounds[i]);
                metadata.max_e.push_back(level_max_e);
                err = clock_gettime(CLOCK_REALTIME, &end);
                cout << "A_inf recording time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
            }
            if(metadata.mse_estimator){
                struct timespec start, end;
                int err = clock_gettime(CLOCK_REALTIME, &start);
                auto level_mse = record_level_mse(reinterpret_cast<T*>(buffer), level_elements[i], metadata.encoded_bitplanes, level_exp);
                metadata.mse.push_back(level_mse);
                err = clock_gettime(CLOCK_REALTIME, &end);
                cout << "A_2 recording time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
            }
            struct timespec start, end;
            int err = clock_gettime(CLOCK_REALTIME, &start);
            // intra-level progressive encoding
            vector<uint8_t> starting_bitplanes;
            vector<uint32_t> intra_level_sizes;
            auto intra_level_components = encode<T, uint64_t>(reinterpret_cast<T*>(buffer), level_elements[i], level_exp, metadata.encoded_bitplanes, starting_bitplanes, intra_level_sizes);
            // release extracted component
            free(buffer);
            // put intra_level_components to level components with optional lossless compression
            level_components.push_back(vector<uint8_t *>());
            vector<size_t>& component_sizes = metadata.component_sizes[i];
            // record starting bitplanes
            {
                uint8_t * compressed_starting_bitplanes = NULL;
                size_t lossless_length = zstd_lossless_compress(ZSTD_COMPRESSOR, 3, starting_bitplanes.data(), starting_bitplanes.size(), &compressed_starting_bitplanes);
                level_components[i].push_back(compressed_starting_bitplanes);
                component_sizes.push_back(lossless_length);                
            }
            for(int k=0; k<intra_level_components.size(); k++){
                if(intra_level_sizes[k] > LOSSLESS_THRESHOLD){
                    uint8_t * lossless_compressed = NULL;
                    size_t lossless_length = zstd_lossless_compress(ZSTD_COMPRESSOR, 3, reinterpret_cast<uint8_t*>(intra_level_components[k]), intra_level_sizes[k], &lossless_compressed);
                    free(intra_level_components[k]);
                    level_components[i].push_back(lossless_compressed);
                    component_sizes.push_back(lossless_length);
                    level_lossless_indicators[k] = true;
                }
                else{
                    level_components[i].push_back(reinterpret_cast<uint8_t*>(intra_level_components[k]));
                    component_sizes.push_back(intra_level_sizes[k]);
                }
            }
            err = clock_gettime(CLOCK_REALTIME, &end);
            cout << "Bitplane encoding time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
        }
    }
    auto t = metadata.component_sizes;
    for(int i=0; i<t.size(); i++){
        for(int j=0; j<t[i].size(); j++){
            cout << t[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
    return level_components;
}

// reposition level-centric decomposed data for recomposition
/*
@params data: decomposed data
@params metadata: metadata object
@params target_level: decomposed level
@params target_recompose_level: recomposed level
@params dims: data dimensions, modified after calling the function
*/
template <class T>
T * level_centric_data_reposition(const vector<vector<const uint8_t*>>& level_components, const Metadata<T>& metadata, int target_level, int target_recompose_level, const vector<int>& intra_recompose_level, vector<size_t>& dims){
    const uint8_t index_size = log2(sizeof(T) * UINT8_BITS);
    auto level_dims = init_levels(dims, target_level);
    const vector<size_t>& level_elements = metadata.level_elements;
    const vector<T>& level_error_bounds = metadata.level_error_bounds;
    size_t num_elements = 1;
    for(int j=0; j<dims.size(); j++){
        dims[j] = level_dims[target_recompose_level][j];
        num_elements *= dims[j];
    }
    T * data = (T *) malloc(num_elements * sizeof(T));
    vector<size_t> dims_dummy(dims.size(), 0);
    // reposition_level_coefficients(reinterpret_cast<T*>(level_components[0][0]), dims, level_dims[0], dims_dummy, data);
    for(int i=0; i<=target_recompose_level; i++){
        const vector<size_t>& prev_dims = (i == 0) ? dims_dummy : level_dims[i - 1];
        if(level_elements[i] * sizeof(T) < seg_size){
            reposition_level_coefficients(reinterpret_cast<const T *>(level_components[i][0]), dims, level_dims[i], prev_dims, data);
        }
        else{
            cout << "decoding level " << level_elements.size() - 1 - i << ", size of components = " << level_components[i].size() << endl;
            // identify exponent of max element
            int level_exp = 0;
            frexp(level_error_bounds[i], &level_exp);
            T * buffer = NULL;
            // -1 to exclude starting bitplane indices
            int retrieved_bitplanes = intra_recompose_level[i] ? intra_recompose_level[i] - 1 : metadata.encoded_bitplanes;
            if(retrieved_bitplanes > metadata.encoded_bitplanes) retrieved_bitplanes = metadata.encoded_bitplanes;            
            cout << "retrieved_bitplanes = " << retrieved_bitplanes << endl;
            // intra-level progressive decoding
            struct timespec start, end;
            int err = clock_gettime(CLOCK_REALTIME, &start);
            const vector<size_t>& level_sizes = metadata.component_sizes[i];
            // using 64 bit for encoding
            vector<const uint64_t *> intra_level_components(retrieved_bitplanes);
            vector<uint8_t *> lossless_decompressed_components;
            // retrieval starting bitplanes
            vector<uint8_t> starting_bitplanes;
            {
                uint8_t * lossless_decompressed = NULL;
                size_t compressed_length = zstd_lossless_decompress(ZSTD_COMPRESSOR, level_components[i][0], level_sizes[0], &lossless_decompressed);
                uint32_t starting_bitplanes_size = (level_elements[i] - 1) / 64 + 1;
                starting_bitplanes = vector<uint8_t>(reinterpret_cast<const uint8_t *>(lossless_decompressed), reinterpret_cast<const uint8_t *>(lossless_decompressed) + starting_bitplanes_size);
                free(lossless_decompressed);
            }
            // lossless decompression
            for(int k=0; k<retrieved_bitplanes; k++){
                if(metadata.lossless_indicators[i][k]){
                    uint8_t * lossless_decompressed = NULL;
                    size_t compressed_length = zstd_lossless_decompress(ZSTD_COMPRESSOR, level_components[i][k + 1], level_sizes[k + 1], &lossless_decompressed);
                    intra_level_components[k] = reinterpret_cast<const uint64_t *>(lossless_decompressed);
                    lossless_decompressed_components.push_back(lossless_decompressed);
                }
                else{
                    intra_level_components[k] = reinterpret_cast<const uint64_t *>(level_components[i][k + 1]);
                }
            }
            buffer = decode<T, uint64_t>(intra_level_components, starting_bitplanes, level_elements[i], level_exp, retrieved_bitplanes);
            for(int k=0; k<lossless_decompressed_components.size(); k++){
                free(lossless_decompressed_components[k]);
            }
            err = clock_gettime(CLOCK_REALTIME, &end);
            cout << "Bitplane decoding time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
            err = clock_gettime(CLOCK_REALTIME, &start);
            reposition_level_coefficients(buffer, dims, level_dims[i], prev_dims, data);
            err = clock_gettime(CLOCK_REALTIME, &end);
            cout << "Level reposition time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
            // string outfile("reconstructed_level_");
            // writefile((outfile + to_string(level_elements.size() - 1 - i) + ".dat").c_str(), reinterpret_cast<const T *>(buffer), level_elements[i]);
            // release reconstructed component
            free(buffer);
        }
    }
    return data;
}

template <class T>
Metadata<T> multigrid_data_refactor(vector<T>& data, const vector<size_t>& dims, int target_level, int option, int reorganization, uint8_t ** refactored_data){
    // create metadata
    Metadata<T> metadata(target_level);
    // set dimensions
    metadata.dims = dims;
    // set encoding option
    metadata.option = option;
    // set data reorganization
    metadata.data_reorganization = reorganization;
    // set error mode, use MAX_ERROR as metric for data reorganization for generality
    metadata.set_mode(MAX_ERROR);
    // collect metadata in original data
    T max_val = data[0];
    T min_val = data[0];
    for(int i=1; i<data.size(); i++){
        if(data[i] > max_val) max_val = data[i];
        if(data[i] < min_val) min_val = data[i];
    }
    metadata.max_val = max_val;
    metadata.min_val = min_val;
    // multigrid decompose
    struct timespec start, end;
    int err = 0;
    err = clock_gettime(CLOCK_REALTIME, &start);
    MGARD::Decomposer<T> decomposer;
    decomposer.decompose(data.data(), dims, target_level);
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Decomposition time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    err = clock_gettime(CLOCK_REALTIME, &start);
    // bit-plane encoding
    auto components = level_centric_data_refactor(data.data(), target_level, dims, metadata);
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Refactor time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    // data reorganization
    err = clock_gettime(CLOCK_REALTIME, &start);
    size_t total_size = 0;
    const vector<size_t>& level_elements = metadata.level_elements;
    const vector<T>& level_error_bounds = metadata.level_error_bounds;    
    const auto& level_errors = metadata.get_level_errors();
    if(*refactored_data) free(*refactored_data);
    if(metadata.data_reorganization == GREEDY_SHUFFLING){
        *refactored_data = refactored_data_reorganization_greedy_shuffling(dims.size(), metadata.mode, components, metadata.component_sizes, level_errors, metadata.order, total_size);
    }
    else if(metadata.data_reorganization == UNIFORM_ERROR){
        *refactored_data = refactored_data_reorganization_uniform_error(dims.size(), metadata.mode, components, metadata.component_sizes, metadata.max_e, metadata.order, total_size);
    }
    else if(metadata.data_reorganization == ROUND_ROBIN){
        *refactored_data = refactored_data_reorganization_round_robin(components, metadata.component_sizes, metadata.order, total_size);
    }
    else{
        *refactored_data = refactored_data_reorganization_in_order(components, metadata.component_sizes, metadata.order, total_size);
    }
    metadata.total_encoded_size = total_size;
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Reorganization time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    // write metadata
    for(int i=0; i<components.size(); i++){
        for(int j=0; j<components[i].size(); j++){
            if(components[i][j]) // skip null pointer for the sign in embedded encoding
                free(components[i][j]);
        }
    }
    return metadata;
}

template <class T>
T * multigrid_data_recompose(const string& filename, const Metadata<T>& metadata, double tolerance, vector<size_t>& recompose_dims, size_t& recompose_times){
    int target_level = metadata.level_elements.size() - 1;
    const vector<size_t>& dims = metadata.dims;
    const vector<vector<double>>& level_errors = metadata.get_level_errors();
    const vector<vector<size_t>>& level_sizes = metadata.component_sizes;
    const vector<int>& order = metadata.order;
    cout << "init level components\n";
    vector<int> num_intra_level_components(target_level + 1, 0);
    size_t retrieved_size = REFACTOR::interpret_reading_size(dims.size(), level_sizes, level_errors, order, metadata.mode, tolerance, num_intra_level_components);
    for(int i=0; i<=target_level; i++){
        if(num_intra_level_components[i] != 0){
            // increment number of level components because 0 and 1 are grouped as 1
            num_intra_level_components[i] ++;
        }
    }
    cout << "retrieved_size = " << retrieved_size << endl;
    // read refactored data
    uint8_t * refactored_data = (uint8_t *) malloc(retrieved_size);
    IO::posix_read(filename, refactored_data, retrieved_size);
    auto components = read_reorganized_data(refactored_data, level_sizes, order, retrieved_size);
    int recomposed_level = 0;
    for(int i=0; i<=target_level; i++){
        if(num_intra_level_components[target_level - i] != 0){
            recomposed_level = i;
            break;
        }
    }
    cout << "Recompose to level " << recomposed_level << endl;;
    recompose_times = target_level - recomposed_level;
    struct timespec start, end;
    int err = 0;
    err = clock_gettime(CLOCK_REALTIME, &start);
    T * data = level_centric_data_reposition<T>(components, metadata, target_level, recompose_times, num_intra_level_components, recompose_dims);
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Reposition time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    // refactored_data can only be freed after reposition because components used pointers
    free(refactored_data);
    err = clock_gettime(CLOCK_REALTIME, &start);
    MGARD::Recomposer<T> recomposer;
    recomposer.recompose(data, recompose_dims, recompose_times);
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Recomposition time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    size_t num_elements = 1;
    for(const auto& d:dims){
        num_elements *= d;
    }
    cout << "Compression ratio = " << num_elements * sizeof(T) * 1.0 / retrieved_size << endl;
    return data;
}

}

#endif