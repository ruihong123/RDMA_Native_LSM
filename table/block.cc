// Copyright (c) 2011 The TimberSaw Authors. All rights reserved.
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
  if (size_ < sizeof(uint32_t)) {
    size_ = 0;  // Error marker
  } else {
    size_t max_restarts_allowed = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
    if (NumRestarts() > max_restarts_allowed) {
      // The size is too small for NumRestarts()
      size_ = 0;
    } else {
      restart_offset_ = size_ - (1 + NumRestarts()) * sizeof(uint32_t);
    }
  }
  if (type == Block_On_Memory_Side){
    rdma_mg_ = Memory_Node_Keeper::rdma_mg;
  }else{
    rdma_mg_ = Env::Default()->rdma_mg;
  }

}

Block::~Block() {
  if (RDMA_Regiested) {
//    DEBUG("Block garbage collected!\n");
    if (type_ == DataBlock && rdma_mg_->Deallocate_Local_RDMA_Slot((void*)data_, "DataBlock")){
//      printf("Block RDMA registered memory deallocated successfull\n");
      return;
    }

    if (type_ == IndexBlock && rdma_mg_->Deallocate_Local_RDMA_Slot((void*)data_, "DataIndexBlock")){
//      printf("Block RDMA registered memory deallocated successfull\n");
      return;
    }
    if (type_ == FilterBlock && rdma_mg_->Deallocate_Local_RDMA_Slot((void*)data_, "FilterBlock")){
//      printf("Block RDMA registered memory deallocated successfull\n");
      return;
    }
    if (type_ == Block_On_Memory_Side){
      return;
    }
    DEBUG("Not found in the RDMA mem pool");
  }
}



Iterator* Block::NewIterator(const Comparator* comparator) {
  if (size_ < sizeof(uint32_t)) {
    return NewErrorIterator(Status::Corruption("bad block contents"));
  }
  const uint32_t num_restarts = NumRestarts();
  if (num_restarts == 0) {
    return NewEmptyIterator();
  } else {
    return new Iter(comparator, data_, restart_offset_, num_restarts);
  }
}

}  // namespace TimberSaw
