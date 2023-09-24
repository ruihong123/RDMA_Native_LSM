// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Decodes the blocks generated by block_builder.cc.

#include "table/block.h"
#include "memory_node/memory_node_keeper.h"
#include "TimberSaw/env.h"

namespace TimberSaw {

inline uint32_t Block::NumRestarts() const {
  assert(size_ >= sizeof(uint32_t));
  return DecodeFixed32(data_ + size_ - sizeof(uint32_t));
}

Block::Block(const BlockContents& contents, BlockType type)
    : data_(contents.data.data()),
      size_(contents.data.size()),
      RDMA_Regiested(true),
      type_(type) {
#ifndef NDEBUG
  if (type_!= IndexBlock && type_!=IndexBlock_Small && type_!=FilterBlock)
    assert(size_< 8192);
#endif
  if (size_ < sizeof(uint32_t)) {
    size_ = 0;  // Error marker
  } else {
    size_t max_restarts_allowed = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
    if (NumRestarts() > max_restarts_allowed) {
      // The size is too small for NumRestarts()
      assert(false);
      size_ = 0;
    } else {
//      printf("Block size for %p is %zu", data_, size_);
      restart_offset_ = size_ - (1 + NumRestarts()) * sizeof(uint32_t);
    }
  }
  if (type == Block_On_Memory_Side || type == Block_On_Memory_Side_Compressed){
    rdma_mg_ = Memory_Node_Keeper::rdma_mg;
  }else{
    rdma_mg_ = Env::Default()->rdma_mg;
  }
//  if(type_ == IndexBlock){
//    printf("INdex Block created, address is %p\n", this->data_);

//  }
}

Block::~Block() {
  if (RDMA_Regiested) {
//    DEBUG("Block garbage collected!\n");
//    No need to deallocate the data block from RDMA memory allocator since it is thread local.
    if (type_ == DataBlock){
//      printf("Block RDMA registered memory deallocated successfull\n");
//      printf("Delete buffer for cache,   start addr %p, length %zu content is %s\n", data_, size_ , data_+1);

      delete[] data_;
      return;
    }

    if (type_ == IndexBlock && rdma_mg_->Deallocate_Local_RDMA_Slot((void*)data_, IndexChunk)){
//      printf("Index Block RDMA registered memory deallocated successfully, address is %p\n", this->data_);
      return;
    }
    if (type_ == IndexBlock_Small && rdma_mg_->Deallocate_Local_RDMA_Slot((void*)data_, IndexChunk_Small)){
      //      printf("Index Block RDMA registered memory deallocated successfully, address is %p\n", this->data_);
      return;
    }
    if (type_ == FilterBlock && rdma_mg_->Deallocate_Local_RDMA_Slot((void*)data_, FilterChunk)){
//      printf("Block RDMA registered memory deallocated successfull\n");
      return;
    }
    if (type_ == Block_On_Memory_Side){
      return;
    }
    if (type_ == Block_On_Memory_Side_Compressed){
      delete[] data_;
      return;
    }
    printf("Not found in the RDMA mem pool");
  }
}



Iterator* Block::NewIterator(const Comparator* comparator) {
//  if (size_ < sizeof(uint32_t)) {
//    return NewErrorIterator(Status::Corruption("bad block contents"));
//  }
  const uint32_t num_restarts = NumRestarts();
  assert(num_restarts <= 64*1024*1024);
  if (num_restarts == 0) {
    return NewEmptyIterator();
  } else {
    return new Iter(comparator, data_, restart_offset_, num_restarts);
  }
}

}  // namespace TimberSaw
