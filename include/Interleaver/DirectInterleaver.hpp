#ifndef _MDR_DIRECT_INTERLEAVER_HPP
#define _MDR_DIRECT_INTERLEAVER_HPP

#include "InterleaverInterface.hpp"

namespace MDR {
    // direct interleaver with in-order recording
    template<class T>
    class DirectInterleaver : public concepts::InterleaverInterface<T> {
    public:
        DirectInterleaver(){}
        void interleave(T const * data, const std::vector<uint32_t>& dims, const std::vector<uint32_t>& dims_fine, const std::vector<uint32_t>& dims_coasre, T * buffer) const {
            uint32_t dim0_offset = dims[1] * dims[2];
            uint32_t dim1_offset = dims[2];
            uint32_t count = 0;
	    std::cout << std::to_string(dims_fine[0]) << "," << std::to_string(dims_fine[1]) << "," << std::to_string(dims_fine[2]) << "," << std::to_string(dim0_offset) << "," << std::to_string(dim1_offset) << ","  << std::to_string(count) << std::endl;
            for(int i=0; i<dims_fine[0]; i++){
                for(int j=0; j<dims_fine[1]; j++){
                    for(int k=0; k<dims_fine[2]; k++){
                        if((i < dims_coasre[0]) && (j < dims_coasre[1]) && (k < dims_coasre[2]))
                            continue;
			buffer[count ++] = data[i*dim0_offset + j*dim1_offset + k];
                    }
                }
            }
	    std::cout << std::to_string(dims_fine[0]) << "," << std::to_string(dims_fine[1]) << "," << std::to_string(dims_fine[2]) << "," << std::to_string(dim0_offset) << "," << std::to_string(dim1_offset) << ","  << std::to_string(count) << std::endl;
	    std::cout << "here here" << std::endl;
        }
        void reposition(T const * buffer, const std::vector<uint32_t>& dims, const std::vector<uint32_t>& dims_fine, const std::vector<uint32_t>& dims_coasre, T * data) const {
            uint32_t dim0_offset = dims[1] * dims[2];
            uint32_t dim1_offset = dims[2];
            uint32_t count = 0;
            for(int i=0; i<dims_fine[0]; i++){
                for(int j=0; j<dims_fine[1]; j++){
                    for(int k=0; k<dims_fine[2]; k++){
                        if((i < dims_coasre[0]) && (j < dims_coasre[1]) && (k < dims_coasre[2]))
                            continue;
                        data[i*dim0_offset + j*dim1_offset + k] = buffer[count ++];
                    }
                }
            }
        }
        void print() const {
            std::cout << "Direct interleaver" << std::endl;
        }
    };
}
#endif
